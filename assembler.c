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
	int i, res;

	char *registers[8] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
	char *instructions[16] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "jsr", "red", "prn", "rts", "stop"};
	char *directives[5] = {".data", ".string", ".mat", ".entry", ".extern"};

	Symbol *sym_table;				/* symbol table to store labels */
	int labels_count;				/* number of labels stored */

	int num_macros;
	char (*macro_names)[MAX_MACRO_LEN];

	Miss *miss_arr;					/* array for missing labels to resolve in second pass */
	int miss_count;					/* number of missing labels */

	short *code_arr;				/* array to store encoded instructions and data */
	int code_count;					/* total number of code/data lines */

	int ic, dc;					/* instruction and data counter */



	/* Main loop: translates all assembly files into base of 4 letters */

	for(i=1; i<argc; i++){

		fp_as = open_file(argv[i], ".as", "r");												/* 1- check file extension */
        if (fp_as == NULL)
            continue;


        sym_table = allocate_symbol_table(fp_as, &labels_count);							/* 2- create dynamically sym_table array */
        if (sym_table == NULL && labels_count > 0) {
            fclose(fp_as);
            continue;
        }


		symbol_table(fp_as, sym_table);														/* 3- fill symbol table with labels names and labels type */


		macro_names = allocate_macro_names(fp_as, &num_macros);								/* 4- create dynamically macro_names array */
        if (macro_names == NULL && num_macros > 0) {
            free(sym_table);
            fclose(fp_as);
            continue;
        }


		res = check_macro(fp_as, registers, instructions, directives, macro_names, sym_table, labels_count);		/* 5a- preprocessor : check macro definition */
		if(!res){
			free(sym_table);
			free(macro_names);
			fclose(fp_as);
			continue;
		}


        fp_am = open_file(argv[i], ".am", "w+");														/* 5b- create after-macro file */
        if (!fp_am) {
            free(sym_table);
            free(macro_names);
            fclose(fp_as);
            continue;
        }


		res = expand_macro(fp_as, fp_am, num_macros, macro_names);							/* 5c- expand macros and write to .am file */
		free(macro_names);		/* free macro_names array */
		fclose(fp_as);			/* close source file */

		if(!res){							/* memory allocation failed inside expand_macro */
			printf("malloc failed\n");
			free(sym_table);
			fclose(fp_am);
			continue;
		}


		res = check(fp_am, instructions, directives, registers, sym_table, labels_count);				/* 6- check input file for syntax, if error skip file */
		if(!res){
			free(sym_table);
			fclose(fp_am);					
			continue;
		}
			
	
		res = first_pass(fp_am, instructions, registers, sym_table, labels_count, &miss_arr, &miss_count, &code_arr, &code_count, &ic, &dc);			/* 7- first pass: encode instruction and data */
		if(!res){								/* malloc or realloc failed */
			printf("malloc failed\n");
			free(sym_table);
			fclose(fp_am);
			continue;
		}


		fp_ob = open_file(argv[i], ".ob", "w");														/* 8- create object file */
		if(fp_ob == NULL){
			free(sym_table);
			free(code_arr);
			free(miss_arr);
			fclose(fp_am);
			continue;
		}
	

		res = second_pass(fp_am, fp_ob, argv[i], sym_table, labels_count, miss_arr, miss_count, code_arr, code_count, ic, dc);					/* 9- second pass: resolve missing labels, fill addresses, create entry/extern files */
		if(!res){											/* if object, entry or extern file doesn't open */
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




/**
 * Opens a file by joining a base name and an extension.
 * @param base_name File prefix (e.g., "file").
 * @param extension Extension to append (e.g., ".ob").
 * @param mode Opening mode (e.g., "r", "w", "w+").
 * @return Opened FILE pointer, or NULL on error.
 */
FILE *open_file(char *base_name, char *extension, char *mode){
	FILE *fp;
	char file_name[256];

    sprintf(file_name, "%s%s", base_name, extension);
    fp = fopen(file_name, mode);

    if (fp == NULL)
        printf("Could not open file\n");
    
    return fp;
}



/**
 * Scans the assembly file to count both local (ending with ':') and external labels,
 * then dynamically allocates the symbol table.
 * @param fp Pointer to the active assembly file.
 * @param labels_count_out Pointer to write back the total number of labels found.
 * @return A pointer to the allocated Symbol table, or NULL if none found or malloc failed.
 */
Symbol *allocate_symbol_table(FILE *fp, int *labels_count_out){
    int c;
    int count = 0;
    char word[MAX_NAME + 1];
    Symbol *table;

    while ((c = fgetc(fp)) != EOF){
        if (c == ':')								/* Count standard label definitions (e.g., "LOOP:") */
            count++;
    }

    rewind(fp);

    while (fscanf(fp, "%" STR(MAX_NAME) "s", word) == 1){
        if (strcmp(word, ".extern") == 0)						/* Count external labels (e.g., ".extern W") */
            count++;
    }

    rewind(fp);

    *labels_count_out = count;					    /* Pass the final count back to the main program */

    if (count <= 0)
        return NULL;

    table = malloc(count * sizeof(Symbol));						/* Allocate memory based on the total count */
    if (table == NULL) {
        printf("malloc failed for symbol table\n");
    }

    return table;
}



/**
 * Calculates the number of macros in the file and dynamically allocates 
 * a contiguous memory block to store their names.
 * @param fp_as Pointer to the source assembly file.
 * @param num_macros_out Pointer to store the resulting macro count.
 * @return A generic pointer (void*) to the allocated memory block, or NULL on failure.
 */
void *allocate_macro_names(FILE *fp_as, int *num_macros_out){
    char (*names)[MAX_MACRO_LEN];
    int num;

    num = macro_size(fp_as).num; 						/* Calculate the number of macros using the preprocessor function */
    *num_macros_out = num; 									/* Pass the count back to the main function */

    if (num <= 0)
        return NULL;

    names = malloc(num * sizeof(*names));					/* Allocate a single block: (number of macros) * (max length of each macro name) */
    if (names == NULL)
        printf("malloc failed for macro names\n");

    return names;
}