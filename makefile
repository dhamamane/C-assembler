assembler: assembler.o preproc.o error.o table.o first_pass.o second_pass.o
	gcc -ansi -pedantic -g -Wall assembler.o preproc.o error.o table.o first_pass.o second_pass.o -o assembler

assembler.o: assembler.c assembler.h preproc.h error.h table.h first_pass.h second_pass.h
	gcc -c -ansi -pedantic -Wall assembler.c -o assembler.o

preproc.o: preproc.c preproc.h assembler.h
	gcc -c -ansi -pedantic -Wall preproc.c -o preproc.o

error.o: error.c error.h assembler.h
	gcc -c -ansi -pedantic -Wall error.c -o error.o

table.o: table.c table.h assembler.h
	gcc -c -ansi -pedantic -Wall table.c -o table.o

first_pass.o: first_pass.c first_pass.h assembler.h
	gcc -c -ansi -pedantic -Wall first_pass.c -o first_pass.o

second_pass.o: second_pass.c second_pass.h assembler.h
	gcc -c -ansi -pedantic -Wall second_pass.c -o second_pass.o

