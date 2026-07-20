# C-assembler

A two-pass assembler written in C that translates assembly code into machine code


# Two-Pass Assembler in C

This project is a complete, robust Two-Pass Assembler written in C (ANSI C90 standard). It translates a custom assembly language into binary machine code, represented in a unique base-4 encoded format using the characters `a`, `b`, `c`, and `d`.

The program is designed to strictly handle memory management, prevent leaks, and provide highly detailed syntax and semantic error reports.

## Features

* **Two-Pass Architecture:**
  * **Pass 1 (first_pass.c):** Parses the source file line-by-line, builds the Symbol Table, processes memory directives (.data, .string, .mat), and records unresolved forward references in a dynamic array.
  * **Pass 2 (second_pass.c):** Resolves missing label addresses from the Symbol Table, determines relocation tags (Absolute, Relocatable, External), and generates the final output files.
* **Extensive Error Handling (error.c):** Detects and reports over 50 distinct syntax and semantic errors (tested against 220+ scenarios) with precise line-number reporting—covering strict label naming, directive validation, matrix indexing, and formatting anomalies.
* **Memory Optimization:** Written with strict dynamic memory tracking (malloc, realloc) to ensure a 100% leak-free execution, verified using Valgrind.

## Generated Output Files

Depending on the input, the assembler generates the following files:
* .ob (Object file): Contains the machine code and memory addresses encoded in base-4 letters (a, b, c, d).
* .ent (Entries file): Created only if .entry labels are defined; lists their names and addresses.
* .ext (Externs file): Created only if .extern labels are referenced; lists the external labels and the exact memory addresses where they are utilized.

---

## Getting Started

### Prerequisites
* GCC compiler
* Make tool

### Compilation

Compile the main executable using the Makefile:
> make assembler

To clean up all object files and the executable:
> make clean

### Running the Assembler

The assembler processes one or multiple source files sequentially. Run the program by passing the names of the target assembly files as arguments, omitting their ".as" extension:

> ./assembler [filename1] [filename2] [filename3] ...

For each argument passed:
1. The program automatically appends the ".as" extension to locate and read the source file.
2. It processes the file through both passes.
3. If no errors are found, it generates the corresponding output files (.ob, .ent, .ext) in the same directory.