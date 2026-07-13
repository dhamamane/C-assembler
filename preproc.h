
/* Size structure used to store number of lines and maximum lines for a macro */
typedef struct{
		int num;		/* number of lines in macro */
		int max_lines;			/* maximum lines in macro */
	}Size;



/* 
 * macro_size
 * Reads a file and returns the number of lines and maximum lines of a macro.
 * Parameters:
 *  - fp: pointer to the file containing macros
 * Returns:
 *  - Size struct with number of lines and maximum lines
 */
Size macro_size(FILE *fp);



/* 
 * check_macro
 * Checks and validates macros in the source file.
 * Parameters:
 *  - fp: pointer to source file
 *  - registers, instructions, directives: arrays of valid names
 *  - macro_names: array to store macro names
 *  - sym_table: symbol table for labels
 *  - labels_count: number of labels in symbol table
 * Returns:
 *  - Returns 1 if valid, 0 if any error.
 */
int check_macro(FILE *fp, char *registers[8], char *instructions[16], char *directives[5], char (*macro_names)[102], Symbol *sym_table, int labels_count);



/* 
 * expand_macro
 * Expands macros from source file into intermediate file.
 * Parameters:
 *  - fp_as: source file pointer
 *  - fp_am: file pointer to .am file
 *  - num_macros: number of macros to expand
 *  - macro_names: names of macros
 * Returns:
 *  - 1 if successful, 0 if memory allocation fails.
 */
int expand_macro(FILE *fp_as, FILE *fp_am, int num_macros, char (*macro_names)[102]);



/* 
 * reserved_word
 * Checks if a name is a reserved word (instruction, directive, or register).
 * Parameters:
 *  - name: the word to check
 *  - type: type of word (filled if reserved)
 *  - line_num: line number in source file
 *  - instructions, directives, registers: arrays of valid names
 * Returns:
 *  - 1 if reserved, 0 otherwise
 */
int reserved_word(char *name, char type[10], int line_num, char *instructions[16], char *directives[5], char *registers[8]);





