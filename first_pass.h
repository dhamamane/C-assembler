#define FIRST_ADRESS 100



/*
 * first_pass
 *
 * Performs the first pass of the assembler on the source file.
 * This function reads the assembly source file line by line, encodes instructions and directives into temporary arrays 
 * (i_arr for instructions and d_arr for data), updates the symbol table, and records unresolved labels or matrices in the miss array for 
 * resolution during the second pass.
 *
 * Parameters:
 *  - fp: pointer to the source file
 *  - instructions: array of instruction names
 *  - registers: array of register names
 *  - sym_table: array of symbols (labels) with their types
 *  - labels_count: number of labels in sym_table
 *  - miss_arr: pointer to an array storing missing labels (for second pass)
 *  - miss_count: pointer to the count of missing labels
 *  - code_arr: pointer to the dynamically created array for final code
 *  - code_count: pointer to store the total number of code lines
 *  - ic: pointer to instruction counter
 *  - dc: pointer to data counter
 *
 * Returns:
 *  - 1 if the first pass completes successfully
 *  - 0 if a memory allocation error occurs
 */
int first_pass(FILE *fp, char *instructions[16], char *registers[8], Symbol *sym_table, int labels_count, Miss **miss_arr, int *miss_count, short **code_arr, int *code_count, int *ic, int *dc);


