#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 100
#define WORD_FORMAT "%100s"

//data structures

//Args
typedef struct Arg {
   char value[MAX_WORD_LENGTH + 1];
   struct Arg *next;
} Arg;

//Tests
typedef struct Test {
   char inFile[MAX_WORD_LENGTH + 1	]; // file for reading in tests
   char outFile[MAX_WORD_LENGTH + 1]; // file to compare output against
   int timeout;                    // timeout length for the test
   int numOfArgs;                  // number of arguments for the test
   Arg *arg;                      // linked list for arguments
   struct Test *next;              // header to the next test
} Test;

//Program
typedef struct Program {
   char execName[MAX_WORD_LENGTH + 1];// executable file name
   char tempDirName[MAX_WORD_LENGTH + 1];// the temp directory name of the program
   struct Program *next;              // the next program to call
   Arg *arg;                          // linked lists for the arguments
   Test *test;                        // linked lists for the tests
} Program;

//***************************************************************************//
// Function: CheckFileExists(path + file name to check)
// Requitments: InputFiles are in the same directory as the calculon.c
//***************************************************************************//
static int CheckFileExists(char *fileName) {
   if (access(fileName, F_OK) == -1) {  // file doesnt exists
      printf("File not Found: '%s'\n", fileName);
      return 0;
   }
   return 1;
}

//***************************************************************************//
// Function: RunProgram
// Requitments: 
//***************************************************************************//
//static Program RunProgram(Program *prom) {
//   printf("Hey we ran the program!\n");
	
//	Program retProgram = prom;
   //make the temp dir
   //transfer the files
   //exicute the program
   //delete the temp dir
//   return retProgram;
   //return to parent
//}

//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
static Program* readProgram(FILE *inFile) {
	
   Program *retProgram; // = malloc(sizeof(Program));
   Arg *oldArg = NULL;
	
   int nextChar;
   char nextWord[MAX_WORD_LENGTH + 1];

	// ask why this has to be a DEFINED type
   //(infile, maximum field width, storage)
   fscanf(inFile, WORD_FORMAT, nextWord);
   
   // strncpy(dest, source, size)
   // creates a blank program
   strncpy(retProgram->execName, nextWord, MAX_WORD_LENGTH);
   strcpy(retProgram->tempDirName, ".");
   retProgram->arg = NULL;
   retProgram->test = NULL;
   retProgram->next = NULL;
   
   // remove do while and change to while   
   do { 
	// removes all white spaces from stream
      while ((nextChar = getc(inFile)) == ' ')
         ;
      if (nextChar != '\n' && nextChar != EOF) {
         ungetc(nextChar, inFile); //gets the next char from the stream
         fscanf(inFile, WORD_FORMAT, nextWord);

         if (oldArg != NULL) {
            Arg *tempArg = malloc(sizeof(Arg));
            strncpy(tempArg->value, nextWord, MAX_WORD_LENGTH);
            tempArg->next = NULL;

            oldArg = oldArg->next = tempArg;
         }
     
         else {
            Arg *tempArg = malloc(sizeof(Arg));
            strncpy(tempArg->value, nextWord, MAX_WORD_LENGTH);
            tempArg->next = NULL;

            retProgram->arg = oldArg = tempArg;
         }
         
      }
   } while (nextChar != '\n' && nextChar != EOF);
   
   return retProgram;
}

//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
static Program* ReadSuiteToProgram(FILE *inFile) {
   Program *retProgram = NULL; // The program to fill and return.
   Program *oldProgram = NULL; // Reference for past programs.
	Test *test = NULL;          // Holds the tests for the program.
   int lineChar = 0;           // Iterates through the file, char by char 

   while (!feof(inFile)) { // While we have a suite to read in.
      // Reads in the NEXT char, should really be named nextChar but #lazy
      lineChar = getc(inFile);
      
      if (lineChar != '\n' && lineChar != 'EOF') {
      // While we have stuff to read in the file and we dont have a blank file.
         if (lineChar == 'P') { // get the name of the program to read.
            test = NULL;
            if (oldProgram != NULL)
               oldProgram = oldProgram->next = readProgram(inFile);   
            else
               retProgram = oldProgram = readProgram(inFile);
         } // end P    

         else if (lineChar == 'T') {
         // reads in a test type
            if (oldProgram != NULL) {
               if (test != NULL) //if not the first test
                  test = test->next = NULL;//readTest(inFile);
               else // if this is the first test, update the program test
                  oldProgram->test = test = NULL;//readTest(inFile);
            } // end if program exists
				
            else if (oldProgram == NULL) {
				   printf("Tried to create test without program, exiting...");
					exit(-1);
				}
         } // end T
      } // end while lineChar not end of file
   } // end while not end of file

   return retProgram; // returns the head of the program structure
}

//***************************************************************************//
// Function: main 
// Requitments: InputFiles exist, arg[1] equals the program to be run
//              optional arguments: d for debug 
//***************************************************************************//
int main(int argc, char **argv, char **envp) {
   printf("**************************************************\n");             
	printf("Optional Arguments:\n\t-d : Runs debug output options\n");
   
   // check if the first group of tests exist  
   if (CheckFileExists("Calculon/InputFiles/" + argv[1])) 
      printf("Found Input Files...\n");
   // does saferun exist?
   if (!CheckFileExists("SafeRun/SafeRun.c"))
      printf("SafeRun Executable found...\n");	

   // create program
   Program *prog;

   //fopen is a base c library call, use it instead of open which is an OS call 
   // arg[1] = name of the program to execute
   if ((prog = ReadSuiteToProgram(fopen(argv[1], "r"))) != NULL) {
      //RunProgram(); // if the suite was read correctly, then execute
   }
   else
      printf("Program: %s: was not read successfully\n", argv[1]);
 
   printf("**************************************************\n");
   printf("Caculon exiting...\n");
   return 0;
}

