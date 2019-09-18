#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_WORD_LENGTH 100

//Args
typedef struct Arg {
   char value[MAX_WORD_LENGTH + 1];
   struct Arg *next;
} Arg;

//Tests
typedef struct Test {
   char inFile[MAX_WORD_LENGTH + 1]; // file for reading in tests
   char outFile[MAX_WORD_LENGTH + 1]; // file to compare output against
   int timeout;                    // timeout length for the test
   int numOfArgs;                  // number of arguments for the test
   Arg *arg;                       // linked list for arguments
   struct Test *next;              // header to the next test
} Test;

//Program
//typedef struct Program {
//   char execName[MAX_WORD_LENGTH + 1];// executable file name
//   char tempDirName[MAX_WORD_LENGTH + 1];// the temp directory name of the program
//   struct Program *next;              // the next program to call
//   Arg *arg;                          // linked lists for the arguments
//   Test *test;                        // linked lists for the tests
//} Program;
typedef struct Program {
   char *execName;                    // executable file name
   char *tempDirName;                 // the temp directory name of the program
   struct Program *next;              // the next program to call
   Arg *arg;                          // linked lists for the arguments
   Test *test;                        // linked lists for the tests
} Program;

//***************************************************************************//
// Function: CheckFileExists(path + file name to check)
// Requitments: InputFiles are in the same directory as the calculon.c
//***************************************************************************//
static int checkFileExists(char *fileName) {
   if (access(fileName, F_OK) == -1) {  // file doesnt exists
      printf("File not Found: '%s'\n", fileName);
      return 0;
   }
   return 1;
}

static int compileProgram(Program *program) {
   //printf("compile program\n");
   char **execCMD, **tempCMD;
   int childPID, status, exitCode = 1, commandCount = 4; // temp value
   
   printf("fileName: %s\n", program->tempDirName); 
   
   execCMD = tempCMD = calloc(sizeof(char *), commandCount + 4);
   
   // check makefile exists 
   if (checkFileExists("Makefile"))
      *execCMD++ = "make";
   else { // if not then we to manually compile
      printf("We have a gcc file\n");
   }

   // compile the list of arguments
   *execCMD = program->execName;
   // then fork the program
   printf("---make %s\n",program->execName);
 
   if ((childPID = fork()) < 0) {
       fprintf(stderr, "ERROR, you fucked up the fork\n");
   }
   else {
      execvp(*execCMD, execCMD);
      exit(-1); // if failed
   }

   wait(&status);
   exitCode = WEXITSTATUS(status);

   if (exitCode) {
      printf("Failed on: ");
      for (tempCMD = execCMD; *tempCMD != NULL; tempCMD++) {
         printf(" %s", *tempCMD);
      }
      printf("\n");
   }

   return exitCode;
}

static void testProgram(Program *program) {
   int totalTests = 0;
   int passedTests = 0;
   printf("Program Tested\n");
   //Test *currTest = program->test;
   //passedTests = testCases(program);

}

static void debugPrintProgram(Program *prog) {
   for (; prog != NULL; prog = prog->next) {
      Arg *arg = malloc(sizeof(arg));
      Test *test = prog->test;
		
      printf("-------\nProgram: %s\n", prog->execName);
      
      for (int i = 1; test != NULL; test = test->next, i++) {
         printf("-=-=-=-=-=-=-\n");
         printf("-Test Number: %d\n", i);
         printf("-IN File: %s\n", test->inFile);
         printf("-OUT File: %s\n", test->outFile);
         printf("-Test Timeout: %d\n", test->timeout);
         printf("-Test NumberOfArgs: %d\n",test->numOfArgs);
         printf("Args: ");
         for (arg = test->arg; arg != NULL; arg = arg->next)
            printf("%s", arg->value);
         
         printf("\n");
      } // end for each test in program
   } // end for each program
   printf("-=-=-=-=-=-=-\n");
}

//***************************************************************************//
// Function: RunProgram
// Requitments: 
//***************************************************************************//
static void runProgram(Program *program) { 
   for (; program != NULL; program = program->next) {
      compileProgram(program);
      testProgram(program);
      //debugPrintProgram(program);
   } 
   //make the temp dir
   //transfer the files
   //exicute the program
   //delete the temp dir
}

static Test* readTest(FILE *inFile) {
   int nextChar = 1;
   char *nextWord = malloc(MAX_WORD_LENGTH); // storage for test input
   Test *retTest = malloc(sizeof(Test)); // test to return

   Arg *oldArg = NULL;
   
   // fill in a bunch of default data for the test
   retTest->arg = NULL;
   retTest->next = NULL;
   retTest->numOfArgs = 0;
   // scan in some actual data
   fscanf(inFile, "%100s", retTest->inFile);
   fscanf(inFile, "%100s", retTest->outFile);
   fscanf(inFile, "%8d", &retTest->timeout);
 
   //printf("- Tests located...");
   while (nextChar != '\n' && nextChar != EOF) {
      while ((nextChar = getc(inFile)) == ' ')
         ;// ignoring white spaces
      if (nextChar != '\n' && nextChar != EOF) {

         //printf("%d\n",nextChar); //ignores the param -
         ungetc(nextChar, inFile);
         fscanf(inFile, "%100s", nextWord);         

         Arg *tempArg = malloc(sizeof(Arg));
         strncpy(tempArg->value, nextWord, MAX_WORD_LENGTH);
         tempArg->next = NULL;

         if (oldArg != NULL) { // if this is the second test
            oldArg = oldArg->next = tempArg; // attach it to the linked list
	    //printf("Arg Added: (%s) (%s)\n",tempArg->value, tempArg->next->value);
         } 
         else { // else if this is the first test on our tree
            retTest->arg = oldArg = tempArg;
            //printf("Arg Added: (%s) (%s)\n",tempArg->value, tempArg->next->value);				
	    // attach it to the beginning of our tree
         }
         // increase the number of args per test
         retTest->numOfArgs+=1;
      }
   }	
   //if (debug == 1) 
     // printf("Test Added: %s, %s, %d, %d\n", retTest->inFile, retTest->outFile, retTest->timeout, retTest->numOfArgs);
   return retTest; // return the head of the test tree
}
//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
static Program* readProgram(FILE *inFile) {
	
   Program *retProgram = malloc(sizeof(Program));
   Arg *oldArg = NULL;
	
   int nextChar;
   char *nextWord = malloc(MAX_WORD_LENGTH);
   retProgram->execName = malloc(MAX_WORD_LENGTH);
   retProgram->tempDirName = malloc(MAX_WORD_LENGTH);

   //(infile, maximum field width, storage)
   fscanf(inFile, "%100s", nextWord);
   
   // strncpy(dest, source, size) vr strcpy(dest, source)
   strncpy(retProgram->execName, nextWord, MAX_WORD_LENGTH);
   strcpy(retProgram->tempDirName, ".");
   retProgram->arg = NULL;
   retProgram->test = NULL;
   retProgram->next = NULL;
   
	//rintf("- Program located...");
   // remove do while and change to while   
   do { 
	// removes all white spaces from stream
      while ((nextChar = getc(inFile)) == ' ')
         ;
      
      if (nextChar != '\n' && nextChar != EOF) {
         ungetc(nextChar, inFile); //gets the next char from the stream
         fscanf(inFile, "%100s", nextWord);

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
   
   //printf(" Program read in...\n");
   return retProgram;
}

//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
static Program* readSuiteToProgram(FILE *inFile) {
   Program *retProgram = NULL; // The program to fill and return.
   Program *oldProgram = NULL; // Reference for past programs.
   Test *test = NULL;          // Holds the tests for the program.
   int lineChar = 0;           // Iterates through the file, char by char 

   while (!feof(inFile)) { // While we have a suite to read in.
      // Reads in the NEXT char, should really be named nextChar but #lazy
      lineChar = getc(inFile);
       
      if (lineChar == '\n' || lineChar == EOF) break;
      // While we have stuff to read in the file and we dont have a blank file.
      if (lineChar == 'P') { // get the name of the program to read.
         test = NULL;
         if (oldProgram != NULL) { // if this is not the first program
            oldProgram->next = readProgram(inFile);
            oldProgram = oldProgram->next;
         } // if this is the first program, start our tree off accordingly
         else {
            retProgram = readProgram(inFile);
            oldProgram = retProgram;
         }
      } // end P    

      else if (lineChar == 'T') {
         // reads in a test type
         if (oldProgram == NULL) {
             printf("Tried to create test without program, exiting...");
             exit(-1);
         }
         if (test != NULL) //if not the first test
            test = test->next = readTest(inFile);
         else { // if this is the first test, update the program test
            oldProgram->test  = readTest(inFile);
            test = oldProgram->test;
         } // end if program exists
      } // end T
   } // end while not end of file

   return retProgram; // returns the head of the program structure
}

//***************************************************************************//
// Function: main 
// Requitments: InputFiles exist, arg[1] equals the program to be run
//              optional arguments: d for debug 
//***************************************************************************//
int main(int argc, char **argv, char **envp) {
   int debug = 0;   

   printf("**************************************************\n");             
   //printf("Optional Arguments:\n\t-d : Runs debug output options\n");
   
   // check if the first group of tests exist  
   if (checkFileExists(argv[1])) 
      printf("- Found Input Files...\n");
   // does saferun exist?
   //if (checkFileExists("~bin/dev"))
   //   printf("- SafeRun Executable found...\n");	
   
   // create program
   Program *prog = readSuiteToProgram(fopen(argv[1], "r"));
   
   // if we want a list of the tests and programs to be printed
   debugPrintProgram(prog);

   if (prog != NULL)
      runProgram(prog); // if the suite was read correctly, then execute
   else
      printf("Program: %s: was not read successfully\n", argv[1]);
 
   printf("**************************************************\n");
   printf("Caculon exiting...\n");
   return 0;
}

