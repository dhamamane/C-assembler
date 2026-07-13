#include"assembler.h"
#include"preproc.h"
#include"error.h"
#include"table.h"
#include"first_pass.h"
#include"second_pass.h"



/*
 * main
 *
 * Entry point of the assembler program.
 * Processes each input assembly source file (.as) and generates:
 *  1. Preprocessed file (.am) after macro expansion
 *  2. Object file (.ob) with encoded instructions and data
 *  3. Entry file (.ent) listing entry labels (if any)
 *  4. Extern file (.ext) listing external label references (if any)
 *
 * Workflow per file:
 *  - Open source file
 *  - Count and allocate symbol table
 *  - Fill symbol table with label names and types
 *  - Detect and expand macros into a .am file
 *  - Run syntax and semantic validation
 *  - Perform first pass: encode instructions and data, track unresolved labels
 *  - Perform second pass: resolve labels, generate output files
 *
 * Error handling:
 *  - Skips file if allocation or file I/O fails
 *  - Frees allocated memory before continuing
 *
 * Parameters:
 *  - argc: number of command line arguments
 *  - argv: array of command line arguments (input file names without extension)
 *
 * Returns:
 *  - 0 always (error messages printed directly if issues occur)
 */

int main(int argc, char *argv[])
{

	FILE *fp_as, *fp_am, *fp_ob;			/* files pointers */
	int i, c;
	char file_name[30];				/* base filename without extension */
	char word[10];

	char *registers[8] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
	char *instructions[16] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr", "red", "prn", "rts", "stop"};
	char *directives[5] = {".data", ".string", ".mat", ".entry", ".extern"};

	Symbol *sym_table;				/* symbol table to store labels */
	int labels_count;				/* number of labels stored */

	int num_macros;
	char (*macro_names)[102];

	Miss *miss_arr;					/* array for missing labels to resolve in second pass */
	int miss_count;					/* number of missing labels */

	short *code_arr;				/* array to store encoded instructions and data */
	int code_count;					/* total number of code/data lines */

	int ic, dc;					/* instruction and data counter */



	/* Main loop: translates all assembly files into base of 4 letters */

	for(i=1; i<argc; i++){


		/* 1- check end of file name */

		sprintf(file_name, "%s.as", argv[i]);
		fp_as = fopen(file_name, "r");
		if(fp_as == NULL){
			printf("Could not open file\n");
			continue;
		}


		
		/* 2- sym_table array */

		/* count num of current file labels and extern labels */
		labels_count = 0;
		while((c = fgetc(fp_as)) != EOF){
			if(c == ':')				/* label definition */
				labels_count++;
		}

		rewind(fp_as);

		while(fscanf(fp_as, "%"STR(MAX_NAME)"s", word) == 1){
			if(strcmp(word, ".extern") == 0)		/* external label */
				labels_count++;
		}

		rewind(fp_as);


		/* create dynamically sym_table */
		sym_table = malloc(labels_count * sizeof(Symbol));
		if(sym_table == NULL){
			printf("malloc failed\n");
			fclose(fp_as);
			continue;
		}



		/* 3- fill symbol table with labels names and labels type */
		symbol_table(fp_as, sym_table);



		/* 4- macro_names array */

		/* calculate number of macros with preproc file function with preproc.c function */
		num_macros = macro_size(fp_as).num;

		/* create dynamically macro names array */
		macro_names = malloc((num_macros) * sizeof(*macro_names));
		if(!macro_names){
			printf("malloc failed\n");
			free(sym_table);
			fclose(fp_as);
			continue;
		}



		/* 5- preprocessor */

		/* 5a- check macro definition, skip file if error */
		if(!check_macro(fp_as, registers, instructions, directives, macro_names, sym_table, labels_count)){
			free(sym_table);
			free(macro_names);
			fclose(fp_as);
			continue;
		}


		/* 5b- create after macro file */
		sprintf(file_name, "%s.am", argv[i]);
		fp_am = fopen(file_name, "w+");
		if(fp_am == NULL){
			printf("Could not open file\n");
			free(sym_table);
			free(macro_names);
			fclose(fp_as);
			continue;
		}


		/* 5c- expand macros and write to .am file */
		if(!expand_macro(fp_as, fp_am, num_macros, macro_names)){			/* memory allocation failed inside expand_macro 		*/
			printf("malloc failed\n");
			free(sym_table);
			free(macro_names);
			fclose(fp_as);
			continue;
		}


		free(macro_names);		/* free macro_names array */
		fclose(fp_as);			/* close source file */

		

		/* 6- check input file for syntax, if error skip file */
		if(!check(fp_am, instructions, directives, registers, sym_table, labels_count)){
			free(sym_table);
			fclose(fp_am);					
			continue;
		}



		/* 7- create dynamically miss_arr array */
		miss_arr = malloc(10*sizeof(Miss));			/* realloc after in first_pass if needed */
		if(!miss_arr){
			printf("malloc failed\n");
			free(sym_table);
			fclose(fp_am);
			continue;
		}
			
	

		/* 8- first pass: encode instruction and data */
		if(!first_pass(fp_am, instructions, registers, sym_table, labels_count, &miss_arr, &miss_count, &code_arr, &code_count, &ic, &dc)){				/* realloc of miss_arr or malloc of code_arr failed */

			printf("malloc failed\n");
			free(sym_table);
			fclose(fp_am);
			continue;
		}



		/* 9- create object file */
		sprintf(file_name, "%s.ob", argv[i]);		/* add extension '.ob' to input file name */
		fp_ob = fopen(file_name, "w");
		if(fp_ob == NULL){
			printf("Could not open file\n");
			free(sym_table);
			free(code_arr);
			free(miss_arr);
			fclose(fp_am);
			continue;
		}



		/* 10- second pass: resolve missing labels, fill addresses, create entry/extern files */
		if(!second_pass(fp_am, fp_ob, argv[i], sym_table, labels_count, miss_arr, miss_count, code_arr, code_count, ic, dc)){			/* if object file, entry file or extern file doesn't open */
			printf("Could not open file\n");
		}



		/* free allocated memory */
		free(sym_table);
		free(code_arr);
		free(miss_arr);


		/* close files; entry and external files already closed in second_pass */
		fclose(fp_am);
		fclose(fp_ob);



	}		/* end of loop */



	


	return 0;
}




