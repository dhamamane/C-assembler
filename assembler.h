#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>


#define MAX_LEN 80	/* maximum characters in an input line */
#define MAX_LINE 100	/* maximun number of lines in input file */
#define MAX_WORD 256	/* maximum number of lines in output file */
#define MAX_NAME 30	/* maximun characters in label name */


/* macro to convert a macro value into a string */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)



/* structure for a symbol table entry (label) */
typedef struct{
	char name[MAX_NAME+1];				/* label name (null-terminated) */
	short address;				/* address of label in memory */
	char *type;			/* type of label: "data", "code" or "external" */
}Symbol;



/* structure to track missing labels that will be resolved in second pass */
typedef struct{
	short line;			/* line index in code_arr where the label is referenced */
	char name[MAX_NAME+1];			/* name of the label */
}Miss;







