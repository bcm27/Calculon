#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <fcntl.h> // for open

#define DIFF_FAIL 0x01
#define RUNTIME_ERR 0x02
#define TIMEOUT 0x04
#define SAFERUN_ERR 0xC0
#define MAX_WORD_LENGTH 100

// global debug int
int __debug = 0; // 0 false : 1 true
typedef struct dirent DirEntry;

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
   int errors;
} Test;

//Program
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

//***************************************************************************//
// Function: compileProgram checks to make sure the c files are up to date
// Requitments: compileProgram, program has a read in Suitex.suite file, 
//              optional: makefile exists, else gcc will be used
//***************************************************************************//
static int compileProgram(Program *program) {
   int childPID,   // child process ID
         status,   // stores the status bytes
       exitCode,   // used to extract the child exit code
       manyFiles;  // later use: how many c files do we have to compile
   
   char **cmdLine, // used for storing the strings that make up the commands to
                   // execute at the end of the function
        **cmdTemp; // when we need to iterate through the commands w/o loss pos
   char *a;
   
   Arg *itArg,     // temp arg to iterate through program files
       *cHead,     // points to the head file to be compiled
       *cTemp;     // stores the file to be added to the list of compiled files

		 
	Arg *temp, *HeadCFile = NULL, *cFile;
	
   // changes the directory, needs to happen before gcc commands
   chdir(program->tempDirName);
	
	for (temp = program->arg; temp != NULL; temp = temp->next) {
      a = strchr(temp->value, '.');
      if (!a) {
         printf("%s has no file extension found.", temp->value);
      } else {
         if (*(a + 1) == 'c' && *(a + 2) == '\0') {  // if the file is a .c
            manyFiles++;
				
				Arg *rtn = malloc(sizeof(Arg));
            strncpy(rtn->value, temp->value, MAX_WORD_LENGTH);
            rtn->next = NULL;
	
            cFile = rtn;
            cFile->next = HeadCFile;
            HeadCFile = cFile;
         }
      }
   }
   
   // cFiles found plus 4 bytes per each.   
   cmdLine = cmdTemp = calloc(sizeof(char *), manyFiles + 4);
  
   // handles the two different types of compiling
   if (access("Makefile", F_OK) != -1) {
      // if we have a makefile, add the command to the commands to execute
      *cmdTemp++ = "make";
      printf("- Makefile found...\n"); 
   } // end make preparation

   else {
      *cmdTemp++ = "gcc";
      for (cFile = HeadCFile; cFile != NULL; cFile = cFile->next) {
         *cmdTemp = cFile->value;
			if(__debug) { printf("added: %s\n", cFile->value); }
      }
      *cmdTemp++ = "-o";
   }
	
	printf("Command Check: ");
	for (cmdTemp = cmdLine; *cmdTemp != NULL; cmdTemp++) {
      printf(" %s", *cmdTemp);
   }
   printf("\n");
 
   // add the command name to the list	
   *cmdTemp = program->execName;
   fflush(NULL); // flushing buffers just to be safe
   //childPID = fork();

   if ((childPID = fork()) > 0) { // from parent
      ;
   } // end from parent

   else if (childPID == 0) { // from child	
      execvp(*cmdLine, cmdLine); 
      exit(-127); 
   } // end from child   

   else { // failure to create child
      perror("cant create child");
      fflush(NULL);
   }// end error from fork

   // wait for the exit code from the child
   wait(&status);
   // retrieve the terminating result
   exitCode = WEXITSTATUS(status); 

   // if the exit code is a positive then we failed
   if (exitCode) { 
      // print the commands that resulted in the failure
      printf("Failed, commands:");
      for (cmdTemp = cmdLine; *cmdTemp != NULL; cmdTemp++) {
         printf(" %s", *cmdTemp);
      }
      printf("\n");
   } 

   // free up cmds and leave
   free(cmdLine); 
   //free(cmdTemp);
   return exitCode;
}

//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
static void execProgram(char *programName, Test *test) {
   // build file outputs for tests
   int inFileDir = open(test->inFile, O_RDONLY);
   int outFileDir = open("test.output.temp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
   
   // program status holders
   int childPID, status, exitCode;
   
   // variables for arguments from tests that are read in
   char **cmdLine, **cmdTemp;
   
   // paramaters from the program, and string builder arrays
   char timeOut[MAX_WORD_LENGTH + 1],
        clock[MAX_WORD_LENGTH + 1],
        bldStrTimeout[MAX_WORD_LENGTH + 1],
        bldStrClock[MAX_WORD_LENGTH + 1],
        name[MAX_WORD_LENGTH + 1];
	
   // holds the current argument from the test	
   Arg *arg = test->arg;
   // builds the strings for clock timeouts
   strcpy(clock, "-T");
   sprintf(bldStrClock, "%d", 10 * test->timeout);
   strcat(clock, bldStrClock);
   // builds the string for program timeouts
   strcpy(timeOut, "-t");
   sprintf(bldStrTimeout, "%d", test->timeout);
   strcat(timeOut, bldStrTimeout);
   // directory name
   strcpy(name, "./");
   strcat(name, programName);

   // forks child
   if ((childPID = fork()) < 0) // failure
      fprintf(stderr, "ERROR, cannot fork\n");
		
   else if (childPID > 0) { // parent
      close(outFileDir);
      close(inFileDir);
   } 
   else if (childPID == 0) { // child
      cmdLine = cmdTemp = calloc(sizeof(char *), test->numOfArgs + 6);
      // build the command line
      *cmdTemp++ = "SafeRun";
      *cmdTemp++ = "-p50";
      *cmdTemp++ = timeOut;
      *cmdTemp++ = clock;
      *cmdTemp = name;		
      //example: SafeRun -p50 -t1000 -T10 a.out

      for (; arg != NULL; arg = arg->next) {
         *++cmdTemp = arg->value;
         if (__debug) { printf("%s ", arg->value): }
      }
      if (__debug) { printf("\n"); }

      // swap the file outputs
      dup2(inFileDir, 0);
      close(inFileDir);

      dup2(outFileDir, 1);
      dup2(outFileDir, 2);
      close(outFileDir)

      // run whatever program we have created 
      execvp(*cmdLine, cmdLine);
      fprintf(stderr, "ERROR, cannot execute command exec\n");
      // command not found error
      exitCode = 127;
   }

   wait(&status);
   exitCode = WEXITSTATUS(status);

   if (exitCode) { 
      // print the commands that resulted in the failure
      printf("Failed, commands:");
      for (cmdTemp = cmdLine; *cmdTemp != NULL; cmdTemp++) {
         printf(" %s", *cmdTemp);
      }
      printf("\n");		
   }   

   if (exitCode & SAFERUN_ERR) {
      test->errors |= exitCode >> 2 & RUNTIME_ERR;
      test->errors |= exitCode << 2 & TIMEOUT;
   }
}


//***************************************************************************//
// Function: testProgram, tests each individual program and runs it through
//           the tests associated with its main
// Requitments: a program has been read in and compiled w/ recent files
//***************************************************************************//
static void testProgram(Program *program) {
   printf("testing each program\n");
   Test *temp = program->test;

   for (; temp != NULL; temp = temp->next) {
      execProgram(program->execName, temp); 
   }
}

//***************************************************************************//
// Function: testPrintError, runs through the errors associated with a program
// Requitments: that the program has compiled and run its output
//***************************************************************************//
static void testPrintError(Program *program) {
   int totalTests = 0,
       passedTests = 0,
       failures = 0,
       errors = 0;
   Test *test, *hTest = program->test;
   char *programName = program->execName;

   printf("test %s\n\%s\n", test->inFile, test->outFile);
   for (int i = 0; test != NULL; test = test->next, i++) {
      if (test->errors != 0) {
         failures = 1;
         errors = 0;

         printf("%s test %d has failed...\n", programName, i + 1);

         if (test->errors & RUNTIME_ERR) {
            printf("\t-Runtime error\n");
            errors += 1;
         }
         if (test->errors & TIMEOUT) {
            printf("\t-Timeout error\n");
            errors +1;
         }
      } // end 0 errors

      if(!errors)
         printf("%s %d of %d test passed\n", programName, i, i); 
         printf("I: %d", i);
   } // end for loop
}

//***************************************************************************//
// Function: for debugging purposes, prints information associated with prorgam
// Requitments: that a program and data has been populated
//***************************************************************************//
static void debugPrintProgram(Program *prog) {
   // loop through each program and the program it points too
   for (; prog != NULL; prog = prog->next) {
      Arg *arg = malloc(sizeof(arg));
      Test *test = prog->test;
		
      printf("-------\nProgram: %s\n", prog->execName);
      
      // for each program print the following
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
      }
   } // end for each program
   printf("-=-=-=-=-=-=-\n");
}

static void makeTempPIDDir(Program *program) {
   int parentPid = getpid();
   char strPid[MAX_WORD_LENGTH + 1];
 
   sprintf(strPid, "%d", parentPid);
   strcat(program->tempDirName, strPid);
   mkdir(program->tempDirName, 0777);
}

static void CopyFile(char *dirName, char *fileName) {
   char **cmdArgs, **thisArg;
   cmdArgs = thisArg = calloc(sizeof(char *), 4);
   char result[MAX_WORD_LENGTH];
   char slash[] = "/";

   checkFileExists(fileName);

   strcpy(result, dirName);
   strcat(result, slash);
   strcat(result, fileName);

   *thisArg = "cp";
   *++thisArg = fileName;
   *++thisArg = result;
	
	int childPID;
   if ((childPID = fork()) < 0) {
      fprintf(stderr, "Error, cannot fork.\n");
      perror("");
   } else if (!childPID) {
      execvp(*cmdArgs, cmdArgs);
      exit(-1);
   }
   wait(NULL);
	
   free(cmdArgs);
}

static void transferFilesToTempDir(Program *program) {
   Arg *tFile;

   for (tFile = program->arg; tFile != NULL; tFile = tFile->next) {
      CopyFile(program->tempDirName, tFile->value);
   }

   // Copying for all tests in the program input and output files
   for (Test *test = program->test; test != NULL; test = test->next) {
      CopyFile(program->tempDirName, test->inFile);
      CopyFile(program->tempDirName, test->outFile);
   }

   if (access("Makefile", F_OK) != -1) {  // makefile exists
      CopyFile(program->tempDirName, "Makefile");
   }

}


static void deleteTempDirs(char *dirName) {
   DIR *dir;
   DirEntry *dirEntry;

   dir = opendir(".");
   while (NULL != (dirEntry = readdir(dir))) {
      remove(dirEntry->d_name);
   }

   chdir("..");
   closedir(dir);
   remove(dirName);
}
//***************************************************************************//
// Function: RunProgram
// Requitments: 
//***************************************************************************//
static void runProgram(Program *program) { 
   int success = 0;   
   
   for (; program != NULL; program = program->next) {
     
      makeTempPIDDir(program);
      transferFilesToTempDir(program);

      // for each program compile it if the files arent up to date using either
      //  make technology or gcc command line arguments		
      success = compileProgram(program);		

      // once each program is compiled test each programs output
      if (!success) { 
         testProgram(program);
         // print the results of the tests and the pass fail rate
         testPrintError(program);
      }

      else 
         printf("%s exited with bad error code\n", program->execName);


      //deleteTempDirs(program->tempDirName);
   } 

   // removeTempDirectory()
}
//***************************************************************************//
// Function: readTests, reads the test input arguments within the Suitex.suite
// Requitments: the program was passed a Suitex.suite argument and that the
//              file exists.
//***************************************************************************//
static Test* readTest(FILE *inFile) {
   int nextChar = 1; // holds the character that indicats the first line result
   char *nextWord = malloc(MAX_WORD_LENGTH); // storage for the line input
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
 
   if (__debug) { printf("- Tests located...\n"); }
   // while we have not found the end of the line nor the end of the file
   while (nextChar != '\n' && nextChar != EOF) {
      while ((nextChar = getc(inFile)) == ' ')
         ;// ignoring white spaces
      if (nextChar != '\n' && nextChar != EOF) {

         //ignores the param -
         ungetc(nextChar, inFile);
         fscanf(inFile, "%100s", nextWord);         
         
         // creates a blank argument to hold the argument
         Arg *tempArg = malloc(sizeof(Arg));
         strncpy(tempArg->value, nextWord, MAX_WORD_LENGTH);
         tempArg->next = NULL;

         if (oldArg != NULL) { // if this is the second test
            oldArg = oldArg->next = tempArg; // attach it to the linked list

	    if (__debug) {
               printf("Arg Added: (%s) (%s)\n",tempArg->value,
                tempArg->next->value);
            } // end debug statement
         } // end if the second test

         else { // else if this is the first test on our tree
            retTest->arg = oldArg = tempArg;
            if(__debug) {
               printf("Arg Added: (%s) (%s)\n",tempArg->value, 
                tempArg->next->value);				
            } // end debug statement
         } // end if this is the first test

         // increase the number of args per test
         retTest->numOfArgs+=1;
      } // end of line
   } // end of file

   if (__debug == 1) {
      printf("Test Added: %s, %s, %d, %d\n", retTest->inFile, retTest->outFile, 
       retTest->timeout, retTest->numOfArgs);
   }
   return retTest; // return the head of the test tree
}
//***************************************************************************//
// Function: readProgram, reads in the program and test files associated 
// Requitments: requires a Suitex.suite argument and for the files related
//              to the c program to exist, such as header, main.c files, input
//              and output test programs, etc
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
   
   //printf("- Program located...");
   // remove do while and change to while   

   do { // do this while not at the end of the file
	// removes all white spaces from stream
      while ((nextChar = getc(inFile)) == ' ')
         ;
      
      if (nextChar != '\n' && nextChar != EOF) {
         ungetc(nextChar, inFile); //gets the next char from the stream
         fscanf(inFile, "%100s", nextWord);

         // if this is the second program
         if (oldArg != NULL) {
            Arg *tempArg = malloc(sizeof(Arg));
            strncpy(tempArg->value, nextWord, MAX_WORD_LENGTH);
            tempArg->next = NULL;

            oldArg = oldArg->next = tempArg;
         }
         // if this is the first program
         else {
            Arg *tempArg = malloc(sizeof(Arg));
            strncpy(tempArg->value, nextWord, MAX_WORD_LENGTH);
            tempArg->next = NULL;

            retProgram->arg = oldArg = tempArg;
         }         
      } // while not the end of the line
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
             printf("Tried to create test without program, exiting...\n");
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
   int c; // character for argument iteration
  
   printf("**************************************************\n");              
   printf("Optional Arguments:\n\td : Runs debug output options\n");
   printf("**************************************************\n"); 

   // check through arguments for debug option
   for (char **argt = argv; *argt != NULL; ++argt)
      if (**argt == 'd') { __debug = 1; }   
 
   if (__debug && checkFileExists(argv[1]))
         printf("- Input Files Found...\n");  
    
   // create program
   Program *program = readSuiteToProgram(fopen(argv[1], "r"));
   
   // if we want a list of the tests and programs to be printed
   if(__debug) 
      debugPrintProgram(program); 
   

   // if the suite was read correctly, then execute
   if (program != NULL) { runProgram(program); }   
   
   else
      printf("Program: %s: was not read successfully\n", argv[1]);
 
   printf("**************************************************\n");
   printf("Calculon exiting...\n");
   return 0;
}

