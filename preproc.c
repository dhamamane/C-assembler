#include"assembler.h"
#include"preproc.h"



/* 
 * Helper function: calculate the number of macros and the maximum number of lines in any macro.
 * Parameters:
 *  - fp: pointer to the file containing macros
 * Returns:
 *  - Size struct with number of lines and maximum lines
 */

Size macro_size(FILE *fp)
{
	char line[MAX_LEN+2];
	char first_word[MAX_NAME+1];
	int n_lines;					/* counter for lines in current macro */
	Size s;					/* struct contains num of macros and maximum lines number in macro */

	n_lines = 0;
	s.num = 0;
	s.max_lines = 0;

	while(fgets(line, sizeof(line), fp) != NULL){					/* read file line by line */

		if(sscanf(line, "%"STR(MAX_NAME)"s", first_word)>0 && strcmp(first_word, "mcro") == 0){		/* if line starts a macro definition */
			s.num++;		/* count macro */

			while(fgets(line, sizeof(line), fp) != NULL && !(sscanf(line, "%"STR(MAX_NAME)"s", first_word)>0 && strcmp(first_word, "mcroend") == 0) ){	/* count the number of lines in this macro until mcroend is reached */
				n_lines++;
			}
		}

		s.max_lines = (n_lines>s.max_lines)? n_lines : s.max_lines;		/* update maximum number of lines */
		n_lines = 0;							/* reset for next macro */

	}

	rewind(fp);

	return s;
}




/* 
 * check_macro: validate macro definitions before expansion.
 * Checks for duplicates, reserved words, and proper syntax.
 * Fills macro_names array with macro identifiers.
 * Parameters:
 *  - fp: pointer to source file
 *  - registers, instructions, directives: arrays of valid names
 *  - macro_names: array to store macro names
 *  - sym_table: symbol table for labels
 *  - labels_count: number of labels in symbol table
 * Returns:
 *  - Returns 1 if valid, 0 if any error.
 */

int check_macro(FILE *fp, char *registers[8], char *instructions[16], char *directives[5], char (*macro_names)[MAX_MACRO_LEN], Symbol *sym_table, int labels_count)
{
	char line[102];
	char first_word[MAX_NAME+1], macro_name[MAX_NAME+1], extra[MAX_NAME+1];
	int i, s, valid = 1;
	int mac_index = 0;						/* index for macro_names array */
	int line_num;						/* current line number in input file */


	/* check definitions of macros and copy macro name in macro_names array -if it hasn't already been defined */

	for(line_num=1; fgets(line, sizeof(line), fp) != NULL; line_num++){							/* read file line by line */
		s = sscanf(line, "%"STR(MAX_NAME)"s %"STR(MAX_NAME)"s %"STR(MAX_NAME)"s", first_word, macro_name, extra);

		/* if line starts with 'mcro' */
		if(s>0 && strcmp(first_word, "mcro") == 0){

			if(mac_index>0 && s>1){
				/* check that the macro has not already been defined */
				for(i=0; i<mac_index && strcmp(macro_names[i], macro_name)!=0; i++)			/* compare name of macro with defined macro names */
					;
				if(i<mac_index){		/* macro already defined */
					printf("error on line %d : the macro %s is defined twice\n", line_num, macro_name);
					valid = 0;
					continue;				/* do not copy macro_name in macro_names array */
				}
			}
						
			/* check no extra words after macro name */
			if(s == 3){
				printf("error on line %d : characters extra after macro name\n", line_num);
				valid = 0;
				continue;
			}

			/* check macro name is not a reserved word */
			if(!reserved_word(macro_name, "macro", line_num, instructions, directives, registers)){
				valid = 0;
				continue;
			}

			/* check macro name is not a label name */
			for(i=0; i<labels_count && strcmp(macro_name, sym_table[i].name) != 0; i++)			/* compare macro name with label names */
				;

			if(i<labels_count && strcmp(macro_name, sym_table[i].name) == 0){
				printf("error on line %d: the macro name %s is a label name\n", line_num, macro_name);
				valid = 0;
				continue;
			}
			
			strcpy(macro_names[mac_index], macro_name);			/* store macro_names */
			mac_index++;
		}

		/* check for mcroend with extra characters */
		else if(s == 2 && strcmp(first_word, "mcroend") == 0){
			printf("error on line %d : characters extra after mcroend\n", line_num);
			valid = 0;
		     }
			
	}				/* end of for loop */

	rewind(fp);
	return valid;
}




/* 
 * Expand_macro: replaces macro invocations with their contents in output file.
 * Reads source assembly file (fp_as) and writes expanded assembly to fp_am.
 * Parameters:
 *  - fp_as: source file pointer
 *  - fp_am: file pointer to .am file
 *  - num_macros: number of macros to expand
 *  - macro_names: names of macros
 * Returns:
 *  - 1 if successful, 0 if memory allocation fails.
 */

int expand_macro(FILE *fp_as, FILE *fp_am, int num_macros, char (*macro_names)[MAX_MACRO_LEN])
{

	int i, j, s;
	int mac_index = 0;		/* index of macro_names and mac_content arrays (advanced in parallel) */

	char line[102];						/* for \n and \0 */	
	char first_word[MAX_NAME+1], macro_name[MAX_NAME+1];
	char (**mac_content)[102];				/* dynamic array of macros content */

	
	int max_macro_lines = macro_size(fp_as).max_lines;
	rewind(fp_as);						/* after max_macro_lines definition */



	/* allocate array of macros */
	mac_content = malloc(num_macros * sizeof(*mac_content));
	if (!mac_content)
		return 0;

	/* allocate array of lines for each macro */
	for (i=0; i < num_macros+1; i++) {
		mac_content[i] = malloc((max_macro_lines+1) * sizeof(*mac_content[i]));
		if (!mac_content[i]) 
			return 0;
	}


	/* fill macro arrays and create .am file with fp_am */

	while(fgets(line, sizeof(line), fp_as) != NULL){

		line[100] = '\n';			/* ensure newline at end */

		s = sscanf(line, "%"STR(MAX_NAME)"s %"STR(MAX_NAME)"s", first_word, macro_name);

		/* copy blank or comment lines directly */
		if(s == 0 || first_word[0] == ';'){
			fputs(line, fp_am);
			continue;
		}

		/* if macro definition starts, store its content and skip in output file */

		if(strcmp(first_word, "mcro") == 0){

			/* fill macro_names[mac_index] */
			strcpy(macro_names[mac_index], macro_name);

			/* fill mac_content[mac_index] */
			fgets(line, sizeof(line), fp_as);		/* first line of macro content */
			line[100] = '\n';

			s = sscanf(line, "%"STR(MAX_NAME)"s", first_word);

			for(i=0; !(s>0 && strcmp(first_word, "mcroend") == 0); i++){			/* as long as we have not reached 'mcroend' */
				strncpy(mac_content[mac_index][i], line, 101);
				fgets(line, sizeof(line), fp_as);
				line[100] = '\n';

				s = sscanf(line, "%"STR(MAX_NAME)"s", first_word);
			}
			strcpy(mac_content[mac_index][i], "<END>");		/* mark end of mac_content[mac_index] */

			mac_index++;
			continue;
		}


		/* if current line is a macro invocation */

		if(mac_index>0){								/* at least one macro has already been defined */
			for (i=0; i<mac_index && strcmp(first_word, macro_names[i]) != 0; i++)		/* compare first word with macro names */
				;
		}

		/* if macro invocation, expand it */

		if(i<mac_index){
			for(j=0; strcmp(mac_content[i][j], "<END>") !=0; j++)
				fputs(mac_content[i][j], fp_am);			/* copy the content of mac_content[i] into fp_am, replacing the macro name line in .am file */
			continue;
		}

		/* otherwise, copy line as-is */
		fputs(line, fp_am);
		
	}					/* end of loop */

	

	/* free allocated memory */

	for (i=0; i < num_macros+1; i++) {
		free(mac_content[i]);   
	}

	free(mac_content);

	
	rewind(fp_am);

	return 1;
		

}




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

int reserved_word(char *name, char *type, int line_num, char *instructions[16], char *directives[5], char *registers[8])
{
	int i;

			
	for(i=0; i<16; i++)
		if(!strcmp(name, instructions[i])){
			printf("error on line %d : the %s name '%s' is a instruction name\n", line_num, type, name);
			return 0;
		}

	for(i=0; i<5; i++)
		if(!strcmp(name, directives[i]+1)){
			printf("error on line %d : the %s name '%s' is an directive name\n", line_num, type, name);
			return 0;
		}

	for(i=0; i<8; i++)
		if(!strcmp(name, registers[i])){
			printf("error on line %d : the %s name '%s' is a register name\n", line_num, type, name);
			return 0;
		}
	
	return 1;
}







