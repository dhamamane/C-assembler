#include"assembler.h"
#include "first_pass.h"



/* 
 * first_pass - Perform the first pass of the assembler.
 * 
 * This function reads the assembly source file line by line, encodes instructions and directives into temporary arrays 
 * (i_arr for instructions and d_arr for data), updates the symbol table, and records unresolved labels or matrices in the miss array for 
 * resolution during the second pass.
 * 
 * Supported directives:
 *   - .data    : Encodes a list of numbers into the data array.
 *   - .mat     : Encodes a matrix by storing its elements and 
 *                padding with zeros if necessary.
 *   - .string  : Encodes a string, including escape sequences, 
 *                followed by a terminating '\0'.
 * 
 * Instructions:
 *   - Each instruction is encoded into binary form according 
 *     to its opcode and operands.
 *   - Operands can be numbers, registers, labels, or matrices.
 *   - Unresolved labels/matrices are recorded in miss_arr 
 *     for later resolution.
 * 
 * Symbol table updates:
 *   - Labels defined in code receive addresses starting from 100.
 *   - Labels defined in data receive addresses starting from 
 *     (100 + number of instruction lines).
 * 
 * Parameters:
 *   - fp:           input assembly file pointer
 *   - instructions: array of instruction names
 *   - registers:    array of register names
 *   - sym_table:    array of labels and their types
 *   - labels_count: number of labels in sym_table
 *   - miss_arr:     array of unresolved references (labels/matrices)
 *   - miss_count:   number of unresolved references found
 *   - code_arr:     final combined array of instructions and data
 *   - code_count:   total number of code words
 *   - ic:           number of instruction words encoded
 *   - dc:           number of data words encoded
 * 
 * Return:
 *   1 on success,
 *   0 if a memory allocation error occurs
 */


int first_pass(FILE *fp, char *instructions[16], char *registers[8], Symbol *sym_table, int labels_count, Miss **miss_arr, int *miss_count, short **code_arr, int *code_count, int *ic, int *dc){

	int i, j, k, l, p, x, y, value, cells;
	int miss_idx = 0;			/* current index in miss_arr array */
	int param_code;				/* 0=number, 1=label, 2=matrix, 3=register */

	short i_arr[MAX_WORD]={0};			/* instruction array: holds the binary representation of all instruction words; initialized to 0 */
	short d_arr[MAX_WORD]={0};			/* data array: holds the binary representation of all data words; initialized to 0 */

	char line[MAX_LEN+1];
	char *token, *ptr, *ptr2;
	char temp[MAX_NAME];
	Miss *temp_arr;


	*miss_arr = malloc(10 * sizeof(Miss));					/* create dynamically miss_arr array */
    if (!*miss_arr) {
        return 0;
    }


	/* reset counters */
	*code_count = 0;
	*ic = 0;
	*dc = 0;


	while(fgets(line, sizeof(line), fp) != NULL){							/* read file line by line */

		token = strtok(line, " ,\t\n");				/* first token */

		if(token == NULL || token[0] == ';')			/* blank or comment line */
			continue;


		/* if entry or external: no encoding */

		if(strcmp(token, ".entry") == 0 || strcmp(token, ".extern") == 0)
			continue;						


		/* if label definition line: store its address in symbol table */

		if(token[strlen(token)-1] == ':'){

			token[strlen(token)-1] = '\0';					/* remove ':' */

			for(i=0; i<labels_count && strcmp(token, sym_table[i].name) != 0; i++)		/* compare token with label names */
				;

			if(strcmp(sym_table[i].type, "data") == 0)				/* data in label */
				sym_table[i].address = (*dc);			/* data address will be update later */

			else						/* instruction in label */
				sym_table[i].address = (*ic);			/* instruction address will be update later */

			token = strtok(NULL, " ,\t\n");			/* moves to the next token to encode the line */
		}



		/* 1- if data encode all the numbers */

		if(strcmp(token, ".data") == 0){

			while((token=strtok(NULL, " ,\t\n")) != NULL){			/* encode sequence of numbers */

				if(token[0] == '-'){
					token++;				/* advance token to the numeric part */
					value = atoi(token);				/* range of token value has been already check */
					d_arr[(*dc)] = (1<<10) - value;				/* encode a negative number on 10 bits using two's complement method */
				}

				else{
					if(token[0] == '+')
						token++;
					value = atoi(token);
					d_arr[(*dc)] = value;
				}
				
				(*dc)++;			/* move to next line in data array */
			}

			continue;
		}



		/* 2- if matrix encode all the matrix numbers */

		if(strcmp(token, ".mat") == 0){

			/* read [x][y] and extract x and y, to calculate number of cells in the matrix */

			token = strtok(NULL, " ,\t\n");		/* token points to first '[' */
			token++;				/* points to x */
			ptr = strchr(token, ']');
			*ptr = '\0';
			x = atoi(token);

			ptr++;					/* exit the first parentheses */
			ptr2 = strchr(ptr, '[');			/* ptr2 points to second '[' */
			ptr2++;					/* point to y */
			ptr = strchr(ptr2, ']');
			*ptr = '\0';
			y = atoi(ptr2);

			cells = x*y;				/* num of cells */
			

			/* encode sequence of numbers after [x][y] */

			while((token = strtok(NULL, " ,\t\n")) != NULL){
				if(token[0] == '-'){
					token++;
					value = atoi(token);
					d_arr[(*dc)] = (1<<10) - value;
				}

				else{
					if(token[0] == '+')
						token++;
					value = atoi(token);
					d_arr[(*dc)] = value;
				}
				
				(*dc)++;
				cells--;
			}

			/* fill remaining cells with 0 */
			while(cells>0){
				(*dc)++;
				cells--;
			}

			continue;
		}



		/* 3- if string encode all the characters */

		if(strcmp(token, ".string") == 0){

			token = strtok(NULL, " ,\t\n"); 			/* token = string with "" */
			token[strlen(token)-1] = '\0';  		/* delete the closing quotation marks */
			token++;                         		/* delete the opening quotation marks */

			for(i=0; token[i] != '\0'; i++){

				if(token[i] == '\\'){				/* start of an escape sequence */
            				i++;

					switch(token[i]){						/* encode escape sequences in the string */
						case 'n': d_arr[(*dc)++] = '\n'; break;
						case 't': d_arr[(*dc)++] = '\t'; break;
						case 'r': d_arr[(*dc)++] = '\r'; break;
						case 'v': d_arr[(*dc)++] = '\v'; break;
						case 'f': d_arr[(*dc)++] = '\f'; break;
						case 'a': d_arr[(*dc)++] = '\a'; break;
						case 'b': d_arr[(*dc)++] = '\b'; break;
						case '\\': d_arr[(*dc)++] = '\\'; break;
						case '"': d_arr[(*dc)++] = '\"'; break;
						case '\'': d_arr[(*dc)++] = '\''; break;
						case '0': d_arr[(*dc)++] = '\0'; break;
						default:					/* unknown escape: store '\' and the character as is */
							d_arr[(*dc)++] = '\\';
							d_arr[(*dc)++] = token[i];
							break;
					}
				}

				else{
					d_arr[(*dc)++] = token[i];
				}

			}			/* end of loop */


			(*dc)++;  					/* null terminator */

			continue;
		}




		/* 4- instruction */

		l = 0;

		for(i=0; i<16 && strcmp(token, instructions[i]); i++)			/* compare token with instructions */
			;

		/* a- encode instruction name in bits 6-9 */

		i_arr[(*ic)] |= (i<<6);

		if(i == 14 || i == 15){			/* rts or stop: 0 param */
			(*ic)++;
			continue;
		}


		/* params */

		for(p=1; p<3 && (token = strtok(NULL, " ,\t\n")) != NULL; p++){	

			for(j=0; j<8 && strcmp(token, registers[j]); j++)
				;

			/* register */

			if(j<8){

				if(!(p == 2 && param_code == 3))			/* if first param is a register: encode second register in same line in bits 2-5 */
					l++;

				if(p == 2 || (p == 1 && i>=5 && i<=13))		/* second param or one param instruction: target register, encoded in bits 2-5 */	
					i_arr[(*ic)+l]|= (j<<2);

				else					/* first param of 2 params instruction: source register, encoded in bits 6-9 */
					i_arr[(*ic)+l]|= (j<<6);
					
				param_code = 3;
			}



			/* number */

			else if(token[0] == '#'){				/* encode 1 next line */

				token++;			/* point to sign number or to the number */
				l++;			/* current free line to encode param after instruction name */
				if(token[0] == '-'){
					token++;
					value = atoi(token);
					i_arr[(*ic)+l] = (i<<8) - value;			/* encode a negative number on 8 bits using two's complement method */
					i_arr[(*ic)+l] <<=2;				/* move the 8 bits at his place */
				}
				else{
					if(token[0] == '+')
						token++;
					value = atoi(token);
					i_arr[(*ic)+l] = (value<<2);
				}

				param_code = 0;
			     }


				/* matrix : skip line and encode the 2 indexes in same line */

				else if( (ptr = strchr(token, '[')) ){

					l++;			/* move to the next line and fill miss array, to encode matrix name at this line in second_pass */
					strncpy(temp, token, ptr-token);			/* save name of matrix in temp */
					temp[strlen(temp)] = '\0';
				

					/* store matrix name and the num of line in code array, to encode into its address in second pass  */

					(*miss_arr)[miss_idx].line = (*ic)+l;			/* store num of line in code_arr */
					strcpy((*miss_arr)[miss_idx].name, temp);		/* store label name */
					miss_idx++;

					l++;				/* move to the next free line to encode matrix indexes */

					/* encode first index */
					ptr2 = strchr(ptr, ']');
					ptr++;
					strncpy(temp, ptr,ptr2-ptr);
					temp[strlen(temp)] = '\0';
					for(j=0; j<8 && strcmp(temp, registers[j]); j++)	/* compare temp with register: index must be a register */
						;
					i_arr[(*ic)+l]|= (j<<6);			/* encode first register in bits 6-9 */

					/* encode second index */
					ptr = strchr(ptr2, '[');
					ptr2 = strchr(ptr, ']');
					ptr++;
					strncpy(temp, ptr,ptr2-ptr);
					temp[strlen(temp)]='\0';
					for(j=0; j<8 && strcmp(temp, registers[j]); j++)			/* compare temp with register */
						;
					i_arr[(*ic)+l] |= (j<<2);			/* encode second register in bits 2-5 */

					param_code = 2;		/* keep param code */
				     }


			

					/* label : skip line in instructions array to encode his address in second pass */

					else{
						l++;
						/* save label name and num of line in miss_arr to write into label address in second pass */

						(*miss_arr)[miss_idx].line = (*ic)+l;
						strcpy((*miss_arr)[miss_idx].name, token);
						miss_idx++;
						param_code = 1;
					}


			/* encode type of param code in current line */

			if(p == 1 && i>=0 && i<=4)			/* source param: encode its code in bits 4-5 */
				i_arr[(*ic)] |= (param_code<<4);
			else						/* if instruction code between 5 and 13 or if second tour => dest param: encode its code in bits 2-3 */
				i_arr[(*ic)] |= (param_code<<2);
		
		
		}				/* end of encode params loop */



		(*ic) = (*ic)+l+1;			/* moves to the next free line in i_arr */
		l = 0;


		/* resize miss_arr if needed each 10 lines */

		if( (miss_idx+1)%10 == 0 ){

			temp_arr = realloc(*miss_arr, (miss_idx+1+10)*sizeof(Miss));
			if(!temp_arr){
				free(*miss_arr);
				return 0;
			}

			(*miss_arr)=temp_arr;
		}


	}				/* end of main loop */




	/* update sym_table addresses */

	for(j=0; j<labels_count; j++){

		if(strcmp(sym_table[j].type, "code") == 0)
			sym_table[j].address += 100;
				/* addresses code begin at 100 */
		else if(strcmp(sym_table[j].type, "data") == 0)
			sym_table[j].address += (100+(*ic));				/* addresses data begin after the last code address */

	}


	/* store the number of entries added to miss_arr */

	*miss_count = miss_idx;



	/* create dynamically code_arr */

	*code_count = (*ic)+(*dc);			/* at the end of the main loop, *ic and *dc hold the number of instruction and data lines; the total number of code lines is their sum */

	*code_arr = malloc((*code_count) * sizeof(short));
	if(!(*code_arr)){
		free(*miss_arr);
		return 0;
	}


	/* fill code_arr with instruction lines and data lines */

	for(j=0; j<(*ic); j++)
		(*code_arr)[j] = i_arr[j];

	for(k=0; k<(*dc); j++, k++)			/* continue with j in code_arr */
		(*code_arr)[j] = d_arr[k];


	rewind(fp);
	return 1;
}







