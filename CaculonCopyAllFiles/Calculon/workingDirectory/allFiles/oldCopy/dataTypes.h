#ifndef DATATYPES_H
#define DATATYPES_H

#define DIFF_FAIL 0x01     // Difference fail, dec 1
#define RUNTIME_ERR 0x02   // runtime error, dec 2
#define TIMEOUT 0x04       // timeout error, dec 4
#define SAFERUN_ERR 0xC0   // according to clints saferun
#define MAX_WORD_LENGTH 63

// Data Struct = Arg
typedef struct Arg {
   char value[MAX_WORD_LENGTH + 1];
   struct Arg *next;
} Arg;

// Data Struct = Tests
typedef struct Test {
   char inFile[MAX_WORD_LENGTH + 1];    // file for reading in test
   char outFile[MAX_WORD_LENGTH + 1];   // file to compare output against
   char errors;                         // holds the binary error results
   int timeout;                         // timeout length for the test
   int numOfArgs;                       // number of arguments for the test
   Arg *arg;                            // linked list for arguments
   struct Test *next;                   // header to the next test
} Test;

// Data Struct = Programs
typedef struct Program {
   char execName[MAX_WORD_LENGTH + 1];    // executable file name
   char tempDirName[MAX_WORD_LENGTH + 1]; // the temp directory name of the program
   Arg *arg;                              // linked lists for the data
   Test *test;                            // executable file name
   struct Program *next;                  // the next program to call
} Program;

Arg* NewArg(char *input);
#endif
