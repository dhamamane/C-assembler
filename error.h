
/* 
 * check
 * Validates an entire assembly source file for syntax and semantic errors.
 * Parameters:
 *  - fp: pointer to source file
 *  - instructions: array of instruction names
 *  - directives: array of directive names
 *  - registers: array of register names
 *  - sym_table: symbol table of labels
 *  - labels_count: number of labels in sym_table
 * Returns:
 *  - 1 if no errors, 0 otherwise
 */
int check(FILE *fp, char *instructions[16], char *directives[5], char *registers[8], Symbol *sym_table, int labels_count);



/* 
 * check_integer_list
 * Checks syntax and range of a list of integers after a directive like .data or .mat
 * Parameters:
 *  - start: pointer to first character of integer list
 *  - line_num: current line number
 *  - directive: directive name for error messages
 * Returns:
 *  - 1 if no errors, 0 otherwise
 */
int check_integer_list(char *start, int line_num, char directive[10]);



/* 
 * check_instruction_param
 * Checks syntax of an instruction parameter (number, label, matrix, or register)
 * Parameters:
 *  - param: the parameter string
 *  - line_num: current line number
 *  - registers: array of register names
 *  - sym_table: symbol table of labels
 *  - labels_count: number of labels in sym_table
 * Returns:
 *  - 0 for number, 1 for label, 2 for matrix, 3 for register
 *  - -1 if general syntax error. -2 if syntax error with a specific error message
 */
int check_instruction_param(char *param, int line_num, char *registers[8], Symbol *sym_table, int labels_count);





