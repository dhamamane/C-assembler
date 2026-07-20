#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>


#define MAX_LEN 80	/* maximum characters in an input line */
#define MAX_LINE 100	/* maximun number of lines in input file */
#define MAX_WORD 256	/* maximum number of lines in output file */
#define MAX_NAME 30	/* maximun characters in label name */
#define MAX_MACRO_LEN 102


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


/* Help functions */
FILE *open_file(char *base_name, char *extension, char *mode);
Symbol *allocate_symbol_table(FILE *fp, int *labels_count_out);
void *allocate_macro_names(FILE *fp_as, int *num_macros_out);
