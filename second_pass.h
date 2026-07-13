/*
 * second_pass
 *
 * Performs the second pass of the assembler. Resolves addresses of labels
 * recorded during the first pass, encodes relocation information, and 
 * generates output files:
 *  - object file (.ob) containing instructions and data
 *  - entry file (.ent) for entry labels (if any)
 *  - extern file (.ext) for external labels (if any)
 *
 * Parameters:
 *  - fp_am: pointer to the preprocessed assembly file
 *  - fp_ob: pointer to the object file to write
 *  - name: base name of the input file (used for .ent and .ext)
 *  - sym_table: array of symbols (labels) with their addresses
 *  - labels_count: number of labels in sym_table
 *  - miss_arr: array of missing labels recorded during first pass
 *  - miss_count: number of entries in miss_arr
 *  - code_arr: array of instructions and data
 *  - code_count: total number of lines in code_arr
 *  - ic: instruction counter from first pass
 *  - dc: data counter from first pass
 *
 * Returns:
 *  - 1 if successful
 *  - 0 if a file cannot be created or memory allocation fails
 */
int second_pass(FILE *fp_am, FILE *fp_ob, char *name, Symbol *sym_table, int labels_count, Miss *miss_arr, int miss_count, short *code_arr, int code_count, int ic, int dc);



/*
 * convert_and_write
 *
 * Converts a short code value into a string using a 2-bit to letter encoding 
 * (00=a, 01=b, 10=c, 11=d) and writes it to the given file.
 *
 * Parameters:
 *  - code: the short value to encode
 *  - size: number of letters to generate (each represents 2 bits)
 *  - fp: pointer to the file to write
 */
void convert_and_write(short code, int size, FILE *fp);




