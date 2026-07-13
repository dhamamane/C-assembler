/* 
 * symbol_table
 * 
 * Processes the source file to identify and store all labels in the symbol table.
 * Each label's name, type ("code", "data" or "external"), and initial address (to be updated later -exept for external label)
 * are recorded. This function prepares the symbol table for use in the first pass
 * of the assembler.
 * 
 * Parameters:
 *  - fp: pointer to the source file to read
 *  - sym_table: array of Symbol structures to store label information
 * 
 */

void symbol_table(FILE *fp, Symbol *sym_table);
