
# Multi-Threaded Matrix Multiplication

This C++ program demonstrates multi-threaded matrix multiplication using different strategies: Chunk, Mixed, and Mixed-Chunks methods.

## Structure
This directory contains 3 sub-dirs:

- `Input Files`: contains input file `input.txt`
- `Output Files`: contains output file `output.txt`
- `Source Files`: contains source files `Assgn1Src-CO22BTECH11006.cpp`

## Overview

The program allows you to specify the size of the matrix (`n`) and the number of threads (`k`). It reads the matrix from an input file, performs square of matrix using the specified methods, and records the execution time for each thread and method.

## Prerequisites

- C++ compiler
- pthread library (for multi-threading)
- Input file (`input.txt`) containing matrix data
- Output file (`output.txt`) to record results

## Compilation and Execution

To compile the program, navigate to the program's directory and use the following command:
```bash
gcc -o matrix_multiplication Assgn1Src-CO22BTECH11006.c -lpthread
```

To run the program, execute the compiled binary:
```bash
./matrix_multiplication
```

## Input
Create the `input.txt` file in the `Input Files` directory. The file should be in following format:
```bash
n k
A[0][0] A[0][1] ... A[0][n-1]
A[1][0] A[1][1] ... A[1][n-1]
...
A[n-1][0] A[n-1][1] ... A[n-1][n-1]

```
here, n is the size of the matrix and k is the number of threads.


## Output
The program will output the results in the `output.txt` file in the `Output Files` directory.

In the output file, the first line will contain the size of the matrix, the second line will contain the number of threads used.

After that, the program will output the computed square matrix and the execution time for each thread and method.

At the end, the program will print the total execution time for each method.

## Notes

- Ensure that the `Input Files` and `Output Files` directories exist in the current directory or modify the paths accordingly.
- The performance and efficiency can vary based on the system's hardware and available resources.

## Author
- Name: Om Dave
- Roll No.: CO22BTECH11006

