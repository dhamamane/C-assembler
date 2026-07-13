#include "assembler.h"
#include "second_pass.h"



/*
 * second_pass
 *
 * Performs the second pass of the assembler. This stage resolves label addresses,
 * finalizes the machine code, and generates the required output files (.ob, .ent, .ext).
 *
 * Steps performed:
 *  1. Label Resolution:
 *     - For each unresolved symbol recorded in miss_arr during the first pass,
 *       look up its address in the symbol table (sym_table).
 *     - Write the address into the correct place in code_arr.
 *     - Add a relocation flag: 1 for external labels, 2 for relocatable labels.
 *
 *  2. Object File Creation (.ob):
 *     - Write header with instruction count (IC) and data count (DC).
 *     - Write each line of code_arr, paired with its memory address,
 *       encoded in base-4 letters (a, b, c, d).
 *
 *  3. Entry File Creation (.ent):
 *     - If the source file contains `.entry` directives,
 *       create a .ent file listing each entry label and its final address.
 *
 *  4. External File Creation (.ext):
 *     - If the source file contains `.extern` directives,
 *       create a .ext file listing each external label and all addresses where it is referenced.
 *
 * Parameters:
 *   - fp_am: pointer to preprocessed source file (.am)
 *   - fp_ob: pointer to object file (.ob)
 *   - name: base name of the source file (used to build .ent/.ext file names)
 *   - sym_table: array of labels with their addresses and types
 *   - labels_count: number of labels in the symbol table
 *   - miss_arr: array of unresolved symbols (labels to be resolved in this pass)
 *   - miss_count: number of unresolved symbols
 *   - code_arr: array of encoded instructions and data
 *   - code_count: number of entries in code_arr
 *   - ic: final instruction counter (number of instruction words)
 *   - dc: final data counter (number of data words)
 *
 * Returns:
 *   - 1 if successful
 *   - 0 if a file cannot be created or in case of a memory error
 */


int second_pass(FILE *fp_am, FILE *fp_ob, char *name, Symbol *sym_table, int labels_count, Miss *miss_arr, int miss_count, short *code_arr, int code_count, int ic, int dc){

	int i, j, tmp;
	int size;			/* number of letters needed to encode each 2 bits into letters a-d */
	int ent = 0, ext = 0;			/* flags: 1 if .entry or .extern labels exist */
	short address;
	char s[30], file_name[30];
	FILE *fp_ent, *fp_ext;
	



	/* 1- Complete code with label addresses and add relocation code at the end.
 	 *
	 * Steps:
	 * - go to each missing line in code_arr (the indices are saved in miss_arr).
	 * - for each, find the missing label name (also stored in miss_arr).
	 * - search for the label in the symbol table (in sym_table array).
	 * - encode the label's address (saved in the symbol table) into the corresponding entry in code_arr (if the label is external encode address as 0.).
	 * - encode the relocation tag.
	 */


	for(i=0; i<miss_count; i++){								/* iterate over all entries in miss_arr */

		/* search address of label name */
		for(j=0; j<labels_count && strcmp(miss_arr[i].name, sym_table[j].name) != 0; j++)		/* search for the missing label name in the symbol table */
			;

		/* sym_table[j].name == miss_arr[i].name */
		code_arr[miss_arr[i].line] = sym_table[j].address;		/* encode the address of the found label from the symbol table into the code array */

		code_arr[miss_arr[i].line] <<= 2;			/* shift the code left by 2 bits, to bits 2-9 */

		/* encode relocation tag */
		if(strcmp(sym_table[j].type, "external") == 0)			/* if external label */
			code_arr[miss_arr[i].line] |= 1;
		else						/* not external: label defined in current file */
			code_arr[miss_arr[i].line] |= 2;

	}



	/* 2- write object file */


	/* first line: convert and write number of code lines and number of data lines */

	/* ic */
	size = 1;
	tmp = ic;

	/* calculate num of letters needed to encode ic value */
	while(tmp>>=2)							/* shift 'tmp' right by 2 more bits each round; increment 'size' if result is not 0 */
		size++;

	fputs(" ", fp_ob);
	convert_and_write(ic, size, fp_ob);				/* write IC value in letters */
	fputs("\t", fp_ob);


	/* dc */
	size = 1;
	tmp = dc;

	/* calculate num of letters needed to encode dc */
	while(tmp>>=2)							/* shift 'tmp' right by 2 more bits each round; increment 'size' if result is not 0 */
		size++;

	convert_and_write(dc, size, fp_ob);				/* write DC value in letters */
	fputs("\n", fp_ob);



	/* convert and write addresses and code (from code_arr) in object file */
	
	for(i=0, address=100; i<code_count; i++, address++){		/* iterate over each row of code_arr */

		convert_and_write(address, 4, fp_ob);		/* address takes 8 bits */
		fputs("\t", fp_ob);

		convert_and_write(code_arr[i], 5, fp_ob);		/* line of code takes 10 bits */
		fputs("\n", fp_ob);

	}

	rewind(fp_ob);






	/* 3- create and fill entry and extern files */

	/* create entry file if needed */

	while(fscanf(fp_am, "%s", s) == 1 && strcmp(s, ".entry") != 0)			/* search for '.entry' in current file */
		;
	rewind(fp_am);

	if(strcmp(s, ".entry") == 0){
		ent = 1;
		sprintf(file_name, "%s.ent", name);		/* add '.ent' extension to input file name */
		fp_ent = fopen(file_name, "w");
		if(fp_ent == NULL){
			fclose(fp_ob);
			return 0;
		}
	}
	
	/* create external file if needed */

	while(fscanf(fp_am, "%s", s) == 1 && strcmp(s, ".extern") != 0)			/* search for '.extern' in current file */
		;
	rewind(fp_am);

	if(strcmp(s, ".extern") == 0){
		ext = 1;
		sprintf(file_name, "%s.ext", name);		/* add '.ext' extension to intput file name */
		fp_ext = fopen(file_name, "w");
		if(fp_ext == NULL){
			fclose(fp_ob);
			if(ent){
				fclose(fp_ent);
			}
			return 0;
		}
	}


	/* fill .ent file with entry labels and the address where they are defined in object file (stored in code_arr) */

	if(ent){

		while(fscanf(fp_am, "%s", s) == 1){				/* search for entry label */
		
			if(strcmp(s, ".entry") == 0){

				fscanf(fp_am, "%s", s);				/* s = label name */
				for(i=0; i<labels_count && strcmp(s, sym_table[i].name) != 0; i++)		/* search for label name in sym_arr */
					;
				address = sym_table[i].address;			/* store the address where this label is defined in 'address' */

				fprintf(fp_ent, "%s\t", s);				/* write label name */
				convert_and_write(address, 4, fp_ent);				/* write label address */
				fputs("\n", fp_ent);				/* advance to next line */

			}
	
		}

		rewind(fp_am);
		rewind(fp_ent);

		fclose(fp_ent);
	}


	/* fill .ext file with external labels and all the addresses where they appear in object file (stored in code_arr) */

	if(ext){

		while(fscanf(fp_am, "%s", s) == 1){			/* search for each external label */

			if(strcmp(s, ".extern") == 0){

				fscanf(fp_am, "%s", s);				/* s = label name */

				for(i=0; i<miss_count; i++){				/* search for all occurrences of label name in miss_arr */

					if(strcmp(s, miss_arr[i].name) == 0){

						address = miss_arr[i].line + 100;			/* addresses begining at 100 */				
						fprintf(fp_ext, "%s\t", s);
						convert_and_write(address, 4, fp_ext);
						fputs("\n", fp_ext);

					}
				}		/* end of for */

			}		/* end of if */

		}		/* end of while */

		rewind(fp_am);
		rewind(fp_ext);

		fclose(fp_ext);
	}


	return 1;
}





/*
 * convert_and_write
 *
 * Converts a binary value into base-4 letter encoding and writes it to a file.
 *
 * Encoding:
 *   - Each 2 bits are mapped to one character:
 *       00 → 'a', 01 → 'b', 10 → 'c', 11 → 'd'
 *
 * Process:
 *   - The code is shifted so that each group of 2 bits can be isolated.
 *   - A lookup table ("abcd") is used to map the 2-bit value to a character.
 *   - The resulting string of letters is written to the given file.
 *
 * Parameters:
 *   - code: the binary value to encode
 *   - size: number of letters (2-bit groups) to generate
 *   - fp: file pointer where the encoded string will be written
 */


void convert_and_write(short code, int size, FILE *fp){

	int i;
	int shift;					/* shift amount for each 2-bit group */
	int first_shift;					/* initial shift to move first 2 bits to rightmost */
	char converted_code[6];						/* encoded letters string */
	char *letter = "abcd";					/* mapping: 00=a, 01=b, 10=c, 11=d */


	/* bring each 2 bits in the righmost positions and use the bitmask of '11' to determinate each letter match to those 2 bits */

	first_shift = (2*size)-2;			/* initial shift for first 2 bits */

	for(i=0; i<size; i++){

		shift = first_shift - (2*i);
		converted_code[i] = letter[(code >> shift) & 3];	/* for each 2 bit in 'code', perform an AND with the bitmask '11' to check which letter they correspond to */

	}

	converted_code[size] = '\0';			/* end of string */
	fprintf(fp, "%s", converted_code);			/* write encoded string to file */

}







