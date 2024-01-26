Multi-Threaded Vampire Number Finder

Requirements
------------
- GCC Compiler (or any standard C compiler)
- POSIX-compliant environment for pthreads

Input 
-----
Enter the values in input.txt in Input Files directory <br>
n (range of numbers to be checked) <br>
k (number of threads)

Compilation and Execution
-------------------------
To compile the program, navigate to the program's directory and use the following command:
<br>gcc -o vampire_finder Assgn2Src-CO22BTECH11006.c -lpthread

To run the program, execute the compiled binary: <br>
./vampire_finder

Usage
-----
The program will scan the values of n(range of numbers to be checked) and k(number of threads) from the input file "input.txt" present in Input Files Folder.

Program Output
--------------
The program will output vampire numbers found within the specified range along with the ID of the thread that found each number. Additionally, it provides the execution time for each thread and the total execution time.

Output File
-----------
The program generates an output file named output.txt in the Output Files directory, containing detailed information about the execution.

Notes
-----
- Ensure that the Output Files directory exists or modify the file path as needed.
- The performance and efficiency can vary based on the system's hardware and available resources.

Author
------
Om Dave (CO22BTECH11006)
