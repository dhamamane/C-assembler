#include"assembler.h"
#include"preproc.h"
#include"error.h"


/* 
 * check
 * 
 * Validates the entire assembly source file for syntax and semantic errors.
 * Checks include:
 *  1. Line length and total number of lines
 *  2. Blank lines and comment lines
 *  3. Label definitions (format, duplicates, reserved words)
 *  4. Directives (.data, .string, .mat, .entry, .extern)
 *  5. Instruction lines and their parameters
 *  6. Spacing, commas, extra text, and invalid expressions
 *
 * Parameters:
 *  - fp: pointer to the source file
 *  - instructions: array of instruction names
 *  - directives: array of directive names
 *  - registers: array of register names
 *  - sym_table: array of labels and their types
 *  - labels_count: number of labels in sym_table
 *
 * Returns:
 *  - 1 if the file passes all checks
 *  - 0 if any errors are detected
 */

int check(FILE *fp, char *instructions[16], char *directives[5], char *registers[8], Symbol *sym_table, int labels_count)
{
	int i, j = 0, k, p, x, y, count, comma, valid = 1, line_num, line_counter, elements;
	long val;
	char line[MAX_LEN+2], copy[MAX_LEN+2];		/* for \n and \0 */
	char buffer[10];
	char *token, *temp, *start, *ptr, *ptr2, *endptr;

	
	


			
			
	/* main loop: read file line by line */

	for(line_num=1, line_counter=1; fgets(line, sizeof(line), fp) != NULL; line_num++, line_counter++){

		strncpy(copy, line, MAX_LEN+1);					/* copy the line for manipulation */


		/* check length of line */

		if( (line[strlen(line)-1]  !=  '\n' && !feof(fp))  || (feof(fp) && strlen(line)>MAX_LEN) ){			/* line too long if his last character is not '\n', and if last line line is too long if contains more than MAX_LEN characters */

			printf("error on line %d : maximum number of characters has been exceeded\n", line_num);
			valid = 0;
			continue;
		}


		/* check num of lines in input file */
		if(line_counter>MAX_LINE){
			printf("error : input file exceeded the maximum allowed number of lines (%d lines)\n", MAX_LINE);
			valid = 0;
			break;
		}
	
		token = strtok(line, " \t\n");			/* extract first word from line */


		/* 1- blank line */

		if(token == NULL){
			line_counter--;			/* blank lines are not counted */
			continue;
		}



		/* 2- comment line */

		if(token[0] == ';'){
			if(line[0] == ' ' || line[0] == '\t'){								/* check blank before ';' */
				printf("error on line %d : blank before ';' in comment line\n", line_num);
				valid = 0;
			}

			line_counter--;				/* comments not counted as code lines */
			continue;
		}



		/* 3- label definition line */

		if((ptr = strchr(token, ':'))  !=  NULL){		

			if((*(ptr+1)) != '\0'){				/* ':' is not at end of token */
				printf("error on line %d : missing space after label name followed by ':' in label definition line\n", line_num);
				valid = 0;
				continue;
			}
			
			*ptr = '\0';					/* remove the ':' so that the token contains only the label name */
			

			/* check reserved word */
			if(!reserved_word(token, "label", line_num, instructions, directives, registers)){			/* check reserved word; this function is written in preproc.c */
				valid = 0;
				continue;
			}


			/* check for duplicate labels */
			count = 0;
			for(i=0; i<labels_count; i++){	
				if(strcmp(token, sym_table[i].name) == 0)				/* compare token with label names */
					count++;
			}

			if(count>1){								/* label name appears more than one time in label names array */
				printf("error on line %d : label '%s' is defined twice in this file\n", line_num, token);
				valid = 0;
				continue;
				}


			/* check label length and characters */
			if(strlen(token)>MAX_NAME){				
				printf("error on line %d : label name exceeds %d characters\n", line_num, MAX_NAME);
				valid = 0;
				continue;
			}

			if(!isalpha(token[0])){							
				printf("error on line %d : label name must start with a letter\n", line_num);
				valid = 0;
				continue;
			}

			if(strlen(token)>1){				

				for(i=1; i<strlen(token) && (isdigit(token[i]) || isalpha(token[i])); i++)				
					;

				if(i<strlen(token) && !isdigit(token[i]) && !isalpha(token[i])){				/* no digit and no letter character into label name */
					printf("error on line %d : label name must contain only letters and digits\n", line_num);
					valid = 0;
					continue;
				}
			}		
	

			/* move to next token (instruction or directive after label) */
			token = strtok(NULL, " \t\n");						
			if(token == NULL){
				printf("error on line %d : no instruction/data after label name followed by ':'\n", line_num);
				valid = 0;
				continue;
			}

			if(token[0] == ','){
				printf("error on line %d : illegal comma after label name followed by ':'\n", line_num);
				valid = 0;
				continue;
			}

		}				/* end of if label definition line */



		/* 4- directive line */

		if(token[0] == '.'){

			for(i=0; i<5 && strncmp(token, directives[i], strlen(directives[i]))  !=  0 ; i++)		/* compare token with directives */
				;

			/* if token is a directive name: moves token to the next word */
			if(i<5){			
				if(token[strlen(directives[i])] != '\0'){						/* token is longer than directive name */
					printf("error on line %d : missing space after directive name\n", line_num);
					valid = 0;
					continue;
				}
				
				token = strtok(NULL, " \t\n");				/* parameter after directive */
				if(token == NULL){
					printf("error on line %d : missing parameter after '%s' directive\n", line_num, directives[i]);
					valid = 0;
					continue;
				}
			}
			

			/* handle specific directives */

			switch(i)
			{
				case(0):					/* data */
					start = copy + (token-line);				/* start points in copy at the same position that token points in line */
					if(!check_integer_list(start, line_num, directives[i]+1))
						valid = 0;

					continue;							/* end of line */


				case(1):		/* string */

					/* check double quotes */
					if(token[0] != '"' || token[strlen(token)-1] != '"'){
						printf("syntax error on line %d : missing opening or closing double quote at the beginnig or at the end of the string\n", line_num);
						valid = 0;
						continue;
					}

					/* check ASCII character */
					for(j=0; token[j] != '\0' && token[j]>=0 && token[j]<=127; j++)
						;
					if(token[j] != '\0'){
						printf("error on line %d : not ASCII character in a string\n", line_num);
						valid = 0;
					}
					continue;		/* end of line */


				case(2):		/* matrix */

					/* check the first parentheses of the matrix */
					ptr = strchr(token, '[');

					if(ptr == NULL){
						printf("syntax error on line %d : miss '[' in matrix definition\n", line_num);
						valid = 0;
						continue;
					}
						
					if(ptr != token){
						printf("syntax error on line %d : character between '.mat' and '['\n", line_num);
						valid = 0;
						continue;
					}

					ptr2 = strchr(ptr, ']');
					if(ptr2 == NULL){
						printf("syntax error on line %d : miss ']' in matrix definition\n", line_num);
						valid = 0;
						continue;
					}

					if(ptr2 == ptr+1){
						printf("error on line %d : missing matrix size in []\n", line_num);
						valid = 0;
						continue;
					}

					strncpy(buffer, ptr+1, ptr2-ptr-1);		/* store content of first parentheses in buffer */
					buffer[ptr2-ptr-1]='\0';

					val = strtol(buffer, &endptr, 10);
					if(*endptr != '\0'){
						printf("error on line %d : matrix size contains a non-digit character\n", line_num);
						valid = 0;
						continue;
					}
					x = val;		/* first matrix size */

					/* check the second parentheses of the matrix */
					ptr = strchr(ptr2, '[');
					if(ptr == NULL){
						printf("syntax error on line %d : miss '[' in matrix definition\n", line_num);
						valid = 0;
						continue;
					}
						
					if(ptr != (ptr2+1)){
						printf("syntax error on line %d : character between first and second matrix size\n", line_num);
						valid = 0;
						continue;
					}

					ptr2 = strchr(ptr, ']');
					if(ptr2 == NULL){
						printf("syntax error on line %d : miss ']' in matrix definition\n", line_num);
						valid = 0;
						continue;
					}

					if(ptr2 == ptr+1){
						printf("error on line %d : missing matrix size in []\n", line_num);
						valid = 0;
						continue;
					}

					strncpy(buffer, ptr+1, ptr2-ptr-1);		/* store content of second parentheses in buffer */
					buffer[ptr2-ptr-1]='\0';

					val = strtol(buffer, &endptr, 10);
					if(*endptr != '\0'){
						printf("error on line %d : matrix size contains a non-digit character\n", line_num);
						valid = 0;
						continue;
					}
					y = val;		/* second matrix size */


					if(*(ptr2+1) != '\0'){				/* char just after [x][y] */
						printf("syntax error on line %d : expected space between matrix size and matrix elements\n", line_num);
						valid = 0;
						continue;
					}
					
					token = strtok(NULL, " \t\n");			/* next token: amtrix arguments */
					if(token == NULL)						/* no arguments after matrix size; end of line */
						continue;


					/* if arguments after matrix size: check list of matrix values after matrix dimensions */
					start = copy + (token-line);					/* start points in copy at the same position token points in line */

					/* check the list of integer after matrix size */
					if(!check_integer_list(start, line_num, directives[i]+1)){
						valid = 0;
						continue;
					}


					/* check the number of matrix elements */
					ptr = strchr(copy, ']');		/* 'copy' is a copy of curent line */
					ptr2 = strchr(ptr+1, ']')+1;			/* moves to the position after the second ']' following '.mat' */
					temp = strtok(ptr2, " ,\t\n");		/* temp iterates over all matrix elements provided after the matrix dimensions */
					elements = 0;

					while(temp  !=  NULL){			/* count num of elements after matrix size */
						elements++;
						temp = strtok(NULL, " ,\t\n");
					}

					if(elements > (x*y)){
						printf("error on line %d : too many elements provided for the matrix dimensions\n", line_num);
						valid = 0;
					}

					continue;			/* end of line */


				case(3):				/* entry */

					/* check that label name after '.entry' is defined in current file */

					for(j=0; j<labels_count && strcmp(token, sym_table[j].name)  !=  0; j++)		/* compare token (entry label) with labels names defined in this file */
						;					
					if(j == labels_count){				/* token didn't found among label names */
						printf("error on line %d : label name after .entry is not found\n", line_num);
						valid = 0;
						continue;
					}

					/* check that label name after '.entry' is not an external label */

					if(strcmp(sym_table[j].type, "external") == 0){			/* external label */
						printf("error on line %d : label name after .entry is an external label\n", line_num);
						valid = 0;
						continue;
					}

					/* check extra text after entry label */

					token = strtok(NULL, " \t\n");	
					if(token  !=  NULL){
						printf("error on line %d : extra text after entry label\n", line_num);
						valid = 0;
					} 
										
					continue;		/* end of line */


				case(4):		/* extern */

					/* check that label name after '.extern' is not defined in current file */

					for(j=0; j<labels_count && (strcmp(token, sym_table[j].name) != 0 || strcmp(sym_table[j].type, "extern") == 0); j++)		/* compare token (external label) with not external label names */
						;

					if(strcmp(token, sym_table[j].name) == 0 && strcmp(sym_table[j].type, "external") != 0){		/* if external label defined in current file */
						printf("error on line %d : label declared as 'extern' is defined in the current file\n", line_num);
						valid = 0;
						continue;
					}

					/* check label length and characters */
					if(strlen(token)>MAX_NAME){										
						printf("error on line %d : label name exceeds %d characters\n", line_num, MAX_NAME);
						valid = 0;
						continue;
					}

					if(!isalpha(token[0])){									
						printf("error on line %d : label name must start with a letter\n", line_num);
						valid = 0;
						continue;
					}

					if(strlen(token)>1){						

						for(k=1; k<strlen(token) && (isdigit(token[k]) || isalpha(token[k])); k++)				
							;

						if(k<strlen(token) && !isdigit(token[k]) && !isalpha(token[k])){		/* no digit and no letter character into label name */
							printf("error on line %d : label name must contain only letters and digits\n", line_num);
							valid = 0;
							continue;
						}
					}	

					/* check extra text after external label */

					token = strtok(NULL, " \t\n");	
					if(token != NULL){
						printf("error on line %d : extra text after external label\n", line_num);
						valid = 0;
					} 

					continue;		/* end of line */


				default:							/* the word beginning by '.' is not a directive name */
					printf("error on line %d : invalid directive name\n", line_num);
					valid = 0;
					continue;

			}		/* end of switch */

		}		/* end of if directive line */




		/* 5- instruction line */

		for(i=0; i<16 && strncmp(token, instructions[i], strlen(instructions[i])) != 0; i++)			/* compare token with instructions */
			;
	
		if(i == 16){								/* not valid instruction name */
			printf("error on line %d : invalid expression\n", line_num);
			valid = 0;
			continue;
		}


		if(token[strlen(instructions[i])]  !=  '\0'){				/* char extra after instruction name */

			token += strlen(instructions[i]);				/* move token past instruction name */
			if((ptr = strchr(token, ',')) != NULL)
				*ptr = '\0';						/* token captures the next word until the comma */

			/* check if instruction name and parameter are concatenated without space */

			for(j=0; j<labels_count && strcmp(token, sym_table[j].name) != 0; j++)			/* compare token with labels names */
				;
			ptr = strchr(token, '[');

			if(token[0] == '#' || (token[0] == 'r' && token[1] != '\0' && token[1]>=48 && token[1]<=55) || ptr != NULL || j<labels_count){	/* if argument just after instruction name without blank (number, or 'r' with digit between 0 and 7, or matrix, or label) */
				printf("error on line %d : missing space between instruction name and argument\n", line_num);
			}

			else{
				printf("error on line %d : invalid instruction name\n", line_num);
			}

			valid = 0;
			continue;
		}

		token = strtok(NULL, " \t\n");						/* move to first parameter */
			
		/* handle zero-parameter instructions */

		if(i == 14 || i == 15){		/* rts, stop */
			if(token != NULL){
				printf("error on line %d : argument after '%s' instruction\n", line_num, instructions[i]);
				valid = 0;
			}
			continue;				/* end of line */
		}

		/* handle instructions expecting 1 or 2 parameters */

		if(token == NULL){
			printf("error on line %d : missing argument after '%s' instruction\n", line_num, instructions[i]);
			valid = 0;
			continue;
		}

		/* detect extra comma immediately after instruction name */
		if(token[0] == ','){
			printf("error on line %d : extra comma between instruction name and argument\n", line_num);
			valid = 0;
			continue;
		}
			
		/* split token if it contains a comma (first and second parameters) */
		comma = 0;
		temp = NULL;
		if((ptr = strchr(token, ',')) != NULL){
			*ptr = '\0';			/* split token and keep only the first param in token */
			comma++;
			ptr++;

			while(*ptr == ','){		/* skip many commas between first param and second param */
				comma++;		/* keep num of comma between params */
				ptr++;			/* shift the pointer forward */
			}

			while(*ptr == ' ' || *ptr == '\t' || *ptr == '\n')		/* skip whitespaces after comma */
				ptr++;

			temp = ptr;			 /* temp points to second param */		
		}

			
		/* check first param */
		p = check_instruction_param(token, line_num, registers, sym_table, labels_count);
		if(p == (-2)){			/* syntax error already reported */
			valid = 0;
			continue;
		}

		else if(p == (-1)){		/* invalid parameter */
			printf("error on line %d : invalid first param after '%s' instruction\n", line_num, instructions[i]);
			valid = 0;
			continue;
			}

			else if((i>=4 && i<=12 && p == 0) || (i == 4 && p == 3)){				/* parameter doesn't match the instruction */
				printf("error on line %d : first parameter doesn't match %s instruction\n", line_num, instructions[i]);
				valid = 0;
				continue;
				}


		/* check second param */

		/* define next token */
		if(temp != NULL && *temp != '\0')		/* if text after comma */
			token = temp;			/* temp is the second part of previous token */
		else
			token = strtok(NULL, " \t\n");

		/* handle one-parameter instructions (i=5..13) */

		if(i>=5 && i<=13){
			if(token != NULL){
				printf("error on line %d : too much arguments after '%s' instruction\n", line_num, instructions[i]);		/* too much params */
				valid = 0;
			}
			continue;			/* checking is finished */
		}

			
		/* handle two-parameter instructions (i=0..4) */

		if(token == NULL){		/* miss param */
			printf("error on line %d : missing second argument after '%s' instruction\n", line_num, instructions[i]);
			valid = 0;
			continue;
		}

		/* check comma */
		if(comma == 0){
			printf("error on line %d : missing comma between arguments in instruction line\n", line_num);
			valid = 0;
			continue;
		}
		if(comma>1){
			printf("error on line %d : extra comma between arguments in instruction line\n", line_num);
			valid = 0;
			continue;
		}

		/* split token if contains comma in the middle to save only the param before comma */
		ptr = NULL;
		if((ptr = strchr(token, ',')) != NULL){
			*ptr = '\0';				/* split token to keep only the second param */
		}

		if(ptr != NULL){					/* comma after second param */
			printf("error on line %d : extra comma after instruction arguments\n", line_num);
			valid = 0;
			continue;
		}

		p = check_instruction_param(token, line_num, registers, sym_table, labels_count);		/* check param according to instruction */
		if(p == (-2)){				/* syntax error already reported */
			valid = 0;
			continue;
		}

		else if(p == (-1)){				/* invalid parameter */
			printf("error on line %d : invalid second param after '%s' instruction\n", line_num, instructions[i]);
			valid = 0;
			continue;
			}

			else if(i != 1 && p == 0){				/* parameter doesn't match the instruction */
				printf("error on line %d : second parameter doesn't match %s instruction\n", line_num, instructions[i]);
				valid = 0;
				continue;
				}

				
		/* detect extra text after second parameter */	
		token = strtok(NULL, " \t\n");
		if(token != NULL){			/* text after second param */
			printf("error on line %d : extra text after instruction parameters\n", line_num);
			valid = 0;
			continue;
		}
		

	}			/* end of main for loop */

	rewind(fp);

	return valid;
}






/*
 * check_integer_list : checks syntax and range of a list of integers after a directive like data or matrix declaration
 *
 * iterates through the string to ensure proper comma placement, valid digits (with optional '+' or '-'), 
 * and no extra characters. then splits the string into integers and verifies each is within -512 to 511.
 *
 * parameters:
 * - start: pointer to the first character of the integer list
 * - line_num: current line number (for error messages)
 * - directive: directive name (for error messages)
 *
 * returns 1 if the list is valid, 0 otherwise.
 */

int check_integer_list(char *start, int line_num, char directive[10])
{
	int i, val;
	int space;		/* flag indicating a blank character was encountered between 2 words */
	int comma;		/* flag indicating a comma was encountered between 2 words */
	char *token;

	
	comma = 1;				/* initially, comma not allowed after directive name or matrix declaration */
	space = 0;


	/* 1- check commas and char type */

	for(i=0; start[i] != '\0'; i++){

		if(start[i] == ' ' || start[i] == '\t' || start[i] == '\n'){			/* blank character */
			space = 1;
			continue;			/* skip blanks */
		}

		if(start[i] == ','){				/* handle commas */
			if(comma){
				printf("syntax error on line %d : illegal comma into integer list after %s directive\n", line_num, directive);
				return 0;
			}

			else{				/* valid comma: update flags */
				comma = 1;
				continue;
			}
		}

		/* not a blank and not a comma: validate char */

		/* skip '+' or '-' */
		if(start[i] == '+' || start[i] == '-'){
			if(!isdigit(start[i+1])){			/* no number after '+' or '-' */
				printf("error on line %d : non-digit input in integer list after %s directive\n", line_num, directive);
				return 0;
			}
		}

		else if(!isdigit(start[i])){			/* char is not blank, not comma, not + and not - */
			printf("error on line %d : non-digit input in integer list after %s directive\n", line_num, directive);
			return 0;
		     }


		/* check missing comma */
		if(space == 1 && comma == 0){			/* two args with space between but not comma */
			printf("syntax error on line %d : missing comma between 2 integers after %s directive\n", line_num, directive);
			return 0;
		}


		/* valid digit encounted: update flags */
		comma = 0;
		space = 0;

	}				/* end of first loop */


	if(comma){						/* extra comma at end of integer list */
		printf("error on line %d : extra comma at end of integer list after %s directive\n", line_num, directive);
		return 0;
	}


	

	/* 2- check range of numbers of data part */

	token = strtok(start, " ,\t\n");			/* split line in token from start */

	while(token != NULL){			
		val = atoi(token);

		if(val<(-512) || val>511){				/* ensure number fits in 10 bits */
			printf("error on line %d : integer in list after %s directive out of range, cannot be represented in 10 bits\n", line_num, directive);
			return 0;
		}

		token = strtok(NULL, " ,\t\n");			/* move to the next integer */

	}		/* end of second loop */

	
	return 1;
}






/* 
 * check_instruction_param
 *
 * Checks the type of a parameter used after an assembly instruction.
 * The parameter can be:
 *   - an immediate number (#number) encoded in 8 bits   -> returns 0
 *   - a label defined in the file                       -> returns 1
 *   - a matrix referenced by [register][register]       -> returns 2
 *   - a valid register                                  -> returns 3
 *
 * Returns -2 if a syntax error is detected (prints an error message),
 * or -1 if the parameter is invalid.
 *
 * param        = string containing the parameter to check
 * line_num     = line number in the file for error messages
 * registers    = array of valid register names
 * sym_table    = array of defined symbols (labels and matrices)
 * labels_count = number of entries in sym_table
 */

int check_instruction_param(char *param, int line_num, char *registers[8], Symbol *sym_table, int labels_count)
{
	int i, value;
	char *ptr, *ptr2, *endptr, temp[MAX_LEN+1];


	/* check if parameter is an immediate number starting with '#' */
	if(param[0] == '#'){
		if(param[1] == '\0'){							/* no number after '#' */
			printf("error on line %d : no number after '#'\n", line_num);
			return -2;
		}

		value = strtol(param+1, &endptr, 10);					 /* convert string to integer and enptr point on the first non-digit char */
		if(*endptr != '\0'){
			printf("error on line %d : input after '#' is not a valid integer number\n", line_num);
			return -2;
		}

		/* check if number fits in 8 bits */
		if(value<(-128) || value>127){
			printf("error on line %d : instruction parameter number out of range, cannot be represented in 8 bits\n", line_num);
			return -2;
		}
		else
			return 0;		/* number code */
	}


	/* check if parameter matches a label in the symbol table */
	for(i=0; i<labels_count; i++){
		if(strcmp(param, sym_table[i].name) == 0)
			return 1;				/* label code */
	}


	/* check if parameter matches a register */
	for(i=0; i<8; i++){
		if(strcmp(param, registers[i]) == 0)
			return 3;			/* register code */
	}


	/* check if parameter is a matrix reference with format name[reg][reg] */

	/* check first parentheses */
	if((ptr=strchr(param, '[')) == NULL)
		return -1;

	strncpy(temp, param, ptr-param);			/* store matrix name in temp */
	temp[ptr-param] = '\0';
	
	for(i=0; i<labels_count && strcmp(temp, sym_table[i].name); i++)			/* compare temp with defined matrix names */
			;
	if(i == labels_count){			/* matrix name not defined in current file */
		printf("error on line %d : matrix name '%s' after instruction is not defined in current file\n", line_num, temp);
		return -2;
	}
	
	if((ptr2 = strchr(ptr+1, ']')) == NULL)			/* find closing ']' for first index */
		return -1;

	strncpy(temp, ptr+1, ptr2-ptr-1);		/* copy the content of first matrix parentheses in temp */
	temp[ptr2-ptr-1] = '\0';

	/* matrix index must be a register, in instruction line */
	for(i=0; i<8 && strcmp(temp, registers[i]); i++)
		;
	if(i == 8){			/* parentheses content is not a register */
		printf("error on line %d : in instruction line, matrix indexes must be valid registers\n", line_num);
		return -2;
	}
	
	/* check second parentheses */
	if(*(ptr2+1) != '[')
		return -1;

	ptr = ptr2+1;			/* ptr points to '[' */

	if((ptr2=strchr(ptr, ']')) == NULL)
		return -1;

	strncpy(temp, ptr+1, ptr2-ptr-1);			/* copy the content of second matrix parentheses in temp */
	temp[ptr2-ptr-1] = '\0';

	/* matrix index must be a register, in instruction line */
	for(i=0; i<8 && strcmp(temp, registers[i]); i++)
		;

	if(i == 8){				/* parentheses content is not a register */
		printf("error on line %d : in instruction line, matrix indexes must be valid registers\n", line_num);
		return -2;
	}

	
	return 2;		/* matrix code */

}








