#include"assembler.h"
#include "table.h"


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

void symbol_table(FILE *fp, Symbol *sym_table)
{
	int s;
	int idx = 0;			/* sym_table index */
	char line[MAX_LEN+1];
	char word[MAX_NAME+1], word2[MAX_NAME+1];			/* words extracted from line */




	while(fgets(line, sizeof(line), fp) != NULL){						/* read file line by line */

		s = sscanf(line, "%"STR(MAX_NAME)"s %"STR(MAX_NAME)"s", word, word2);			/* read safely two words from line */

		if(s<2)
			continue;			/* skip lines that are not label definitions or external label */



		/* if external label */

		if(strcmp(word, ".extern") == 0){

			strcpy(sym_table[idx].name, word2);			/* store label name */
			sym_table[idx].address = 0;				/* external labels have address 0 */
			sym_table[idx].type = "external";			/* mark type as external */

			idx++;					/* move to next sym_table entry */
			continue;
		}
			

		/* if define label line */

		if(word[strlen(word)-1] == ':'){

			word[strlen(word) - 1] = '\0';			/* remove trailing ':' from label */
			strcpy(sym_table[idx].name, word);			/* store label name */
			

			/* determine label type based on next word */
			if(word2[0] == '.')					/* data into label */
				sym_table[idx].type = "data";

			else							/* instruction into label */
				sym_table[idx].type = "code";

			/* addresses will be filled later in first_pass */

			idx++;					/* move to next sym_table entry */

		}
	

	}				/* end of while */

	
	rewind(fp);				/* reset file pointer for further reading */

}





