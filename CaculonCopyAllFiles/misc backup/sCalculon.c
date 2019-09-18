#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_WORD_LENGTH 63 // the max word length allowed by a string

#define DIFF_FAIL 0x01     // Difference fail, dec 1
#define RUNTIME_ERR 0x02   // runtime error, dec 2
#define TIMEOUT 0x04       // timeout error, dec 4
#define SAFERUN_ERR 0xC0   // according to clints saferun

// global debug int
int __debug = 0; // 0 false : 1 true

typedef struct dirent DirEntry;

// Arg
typedef struct Arg {
   char value[MAX_WORD_LENGTH + 1];
   struct Arg *next;
} Arg;

// Tests
typedef struct Test {
   char inFile[MAX_WORD_LENGTH + 1];    // file for reading in test
   char outFile[MAX_WORD_LENGTH + 1];   // file to compare output against
   char errors;                         // holds the binary error results
   int timeout;                         // timeout length for the test
   int numOfArgs;                       // number of arguments for the test
   Arg *arg;                            // linked list for arguments
   struct Test *next;                   // header to the next test
} Test;

//Programs
typedef struct Program {
   char execName[MAX_WORD_LENGTH + 1];    // executable file name
   char tempDirName[MAX_WORD_LENGTH + 1]; // the temp directory name of the program
   Arg *arg;                              // linked lists for the data
   Test *test;                            // executable file name
   struct Program *next;                  // the next program to call
} Program;

static Arg *NewArg(char *str) {
   Arg *rtn = malloc(sizeof(Arg));
   strncpy(rtn->value, str, MAX_WORD_LENGTH);
   rtn->next = NULL;
   return rtn;
}


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
// Function: 
// Requitments: 
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
         printf("arg: ");
         for (arg = test->arg; arg != NULL; arg = arg->next)
            printf("%s", arg->value);
         
         printf("\n");
      }
   } // end for each program
   printf("-=-=-=-=-=-=-\n");
}


//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
static Program *ReadProgram(FILE *inFile) {
	
	Program *retProgram = malloc(sizeof(Program));
   Arg *oldArg = NULL;
	
   int nextChar;
   char *nextWord = malloc(MAX_WORD_LENGTH);
   //retProgram->execName = malloc(MAX_WORD_LENGTH);
   //retProgram->tempDirName = malloc(MAX_WORD_LENGTH);
   
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
				
            retProgram->arg = oldArg = NewArg(nextWord);
			}
      } // while not the end of the line

   } while (nextChar != '\n' && nextChar != EOF);

   return retProgram;
}
//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
static Test *readTest(FILE *in) {
   int nextChar = 1; // holds the character that indicats the first line result
   char *nextWord = malloc(MAX_WORD_LENGTH); // storage for the line input
	Test *retTest = malloc(sizeof(Test)); // test to return
   Arg *oldArg = NULL;
	
	// fill in a bunch of default data for the test
   retTest->arg = NULL;
   retTest->next = NULL;
   retTest->timeout = 0;
   retTest->numOfArgs = 0;
   retTest->errors = 0;
   retTest->inFile[0] = retTest->outFile[0] = '\0';
	
	// fill out some actual data
   fscanf(in, "%100s", retTest->inFile);
   fscanf(in, "%100s", retTest->outFile);
   fscanf(in, "%10d", &retTest->timeout);
	
   if (__debug) { printf("- Tests located...\n"); }
	
	// while we have not found the end of the line nor the end of the file
   while (nextChar != '\n' && nextChar != EOF) {
      while ((nextChar = getc(in)) == ' ')
		   ; // ignores white spaces

      if (nextChar != '\n' && nextChar != EOF) {
			// ignores the param
         ungetc(nextChar, in);
         fscanf(in, "%100s", nextWord);
			
			//blank argument to hold the current argument
			Arg *tempArg = malloc(sizeof(Arg));
         strncpy(tempArg->value, nextWord, MAX_WORD_LENGTH);
         tempArg->next = NULL;
			
         if (oldArg != NULL) { // if this is the second test
            oldArg = oldArg->next = tempArg;	    
				
				if (__debug) {
               printf("Arg Added: (%s) (%s)\n",tempArg->value,
               tempArg->next->value);
            } // end debug statement				
			} // end if the second test
			
         else {
            retTest->arg = oldArg = tempArg;
				
				if(__debug) {
               printf("Arg Added: (%s) (%s)\n",tempArg->value, 
               tempArg->next->value);				
            } // end debug statement
			} // end if this is the first test
			
			// increase the number of args per test
         retTest->numOfArgs++;
      } // end of line
   } // end of file 

	if (__debug == 1) {
      printf("Test Added: %s, %s, %d, %d\n", retTest->inFile, retTest->outFile, 
       retTest->timeout, retTest->numOfArgs);
   }
   return retTest; // return the head of the test tree
}


//Read in an entire suite, returns the head of the list of programs
static Program *ReadSuite(FILE *in) {
   int nextChar;
   Program *rtn = NULL, *lastProg = rtn;
   Test *lastTest = NULL;

   while (!feof(in)) {

      nextChar = getc(in);

      if (nextChar != '\n' && nextChar != EOF) {
         //Read in a program
         if (nextChar == 'P') {
            lastTest = NULL;
            if (lastProg != NULL)
               lastProg = lastProg->next = ReadProgram(in);
            else
               rtn = lastProg = ReadProgram(in);
         }
            //Read in a test
         else if (nextChar == 'T') {
            if (lastProg != NULL) {
               if (lastTest != NULL) {
                  lastTest = lastTest->next = readTest(in);
               } else {
                  lastProg->test = lastTest = readTest(in);
               }
            } else {
               fprintf(stderr, "Program has to exist before a test can be run\n");
            }
         } else {
            fprintf(stderr, "Must be a P or a T\n");
         }
      }
   }

   return rtn;
}

static void ForkExec(char **cmdArgs) {
   int childPID;
   if ((childPID = fork()) < 0) {
      fprintf(stderr, "Error, cannot fork.\n");
      perror("");
   } else if (!childPID) {
      execvp(*cmdArgs, cmdArgs);
      exit(-1);
   }
   wait(NULL);
}

static void MkTempDir(Program *program) {
   int pid = getpid();
   char strpid[MAX_WORD_LENGTH + 1];

   sprintf(strpid, "%d", pid);
   strcat(program->tempDirName, strpid);
   mkdir(program->tempDirName, 0777);
}


static void CheckFileExists(char *fileName) {
   if (access(fileName, F_OK) == -1) {  // file doesnt exists
      printf("Could not find required test file '%s'\n", fileName);
      exit(1);
   }
}

static void CopyFile(char *dirName, char *fileName) {
   char **cmdArgs, **thisArg;
   cmdArgs = thisArg = calloc(sizeof(char *), 4);
   char result[MAX_WORD_LENGTH];
   char slash[] = "/";

   CheckFileExists(fileName);

   strcpy(result, dirName);
   strcat(result, slash);
   strcat(result, fileName);

   *thisArg = "cp";
   *++thisArg = fileName;
   *++thisArg = result;

   ForkExec(cmdArgs);
   free(cmdArgs);
}

static void TransFiles(Program *program) {
   Arg *tFile;

   for (tFile = program->arg; tFile != NULL; tFile = tFile->next) {
      CopyFile(program->tempDirName, tFile->value);
   }

   // Copying for all test in the program input and output arg
   for (Test *test = program->test; test != NULL; test = test->next) {
      CopyFile(program->tempDirName, test->inFile);
      CopyFile(program->tempDirName, test->outFile);
   }

   if (access("Makefile", F_OK) != -1) {  // makefile exists
      CopyFile(program->tempDirName, "Makefile");
   }
}

// compiles the source arg & moves into the temp dir
static int CompileProg(Program *program) {
   int cCount = 0, nullFD = open("/dev/null", O_WRONLY);
   int childPID, status, exitCode;
   Arg *itArg, *HeadCFile = NULL, *cFile;
   char **cmdArgs, **thisArg;
   char *a;

   chdir(program->tempDirName);

   for (itArg = program->arg; itArg != NULL; itArg = itArg->next) {
      a = strchr(itArg->value, '.');
      if (!a) {
         printf("%s has no file extension found.", itArg->value);
      } else {
         if (*(a + 1) == 'c' && *(a + 2) == '\0') {  // if the file is a .c
            cCount++;
				
				Arg *argTemp = malloc(sizeof(Arg));
            strncpy(argTemp->value, itArg->value, MAX_WORD_LENGTH);
            argTemp->next = NULL;
   
            cFile = argTemp;
            cFile->next = HeadCFile;
            HeadCFile = cFile;
         }
      }
   }

   cmdArgs = thisArg = calloc(sizeof(char *), cCount + 4);

   if (access("Makefile", F_OK) != -1) {
	//if(checkFileExists("Makefile") {
      *thisArg++ = "make";
   } else {
      *thisArg++ = "gcc";
      for (cFile = HeadCFile; cFile != NULL; cFile = cFile->next) {
         *thisArg++ = cFile->value;
      }
      *thisArg++ = "-o";
   }
	//
   for (thisArg = cmdArgs; *thisArg != NULL; thisArg++) {
      printf(" %s", *thisArg);
   }
   printf("\n");
   //
	*thisArg = program->execName;
	
   if ((childPID = fork()) < 0) {
      fprintf(stderr, "Error, cannot fork.\n");
      perror("");
   } else if (childPID) {
      close(nullFD);
   } else {
      dup2(nullFD, 1);
      close(nullFD);
      execvp(*cmdArgs, cmdArgs);
      exit(-1);
   }
   wait(&status);
   exitCode = WEXITSTATUS(status);

   if (exitCode) {
      printf("Failed:");
      for (thisArg = cmdArgs; *thisArg != NULL; thisArg++) {
         printf(" %s", *thisArg);
      }
      printf("\n");
   }

   free(cmdArgs);
   return exitCode;
}

static void ExecTest(char *execName, Test *test) {
   char **cmdArgs, **thisArg;
   Arg *arg = test->arg;
   int inFD = open(test->inFile, O_RDONLY);
   int outFD = open("test.output.temp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
   int childPID, exitStatus, exitCode;
   char strCPUtime[MAX_WORD_LENGTH + 1];
   char strWCtime[MAX_WORD_LENGTH + 1];
   char argCPU[MAX_WORD_LENGTH + 1];
   char argWC[MAX_WORD_LENGTH + 1];
   char name[MAX_WORD_LENGTH + 1];

   strcpy(argCPU, "-t");
   sprintf(strCPUtime, "%d", test->timeout);
   strcat(argCPU, strCPUtime);

   strcpy(argWC, "-T");
   sprintf(strWCtime, "%d", 10 * test->timeout);
   strcat(argWC, strWCtime);

   strcpy(name, "./");
   strcat(name, execName);


   if ((childPID = fork()) < 0)
      fprintf(stderr, "Error, cannot fork.\n");
   else if (childPID) { /* We are parent! */
      close(inFD);
      close(outFD);
   } else { /* We are child! */

      cmdArgs = thisArg = calloc(sizeof(char *), test->numOfArgs + 6);

      *thisArg++ = "SafeRun";
      *thisArg++ = "-p30";
      *thisArg++ = argCPU;
      *thisArg++ = argWC;
      *thisArg = name;

      for (; arg != NULL; arg = arg->next) {
         *++thisArg = arg->value;
      }
      dup2(inFD, 0);
      dup2(outFD, 1);
      dup2(outFD, 2);
      close(inFD);
      close(outFD);

      execvp(*cmdArgs, cmdArgs);
      fprintf(stderr, "Error, cannot execute.\n");
      exit(1);
   }
   wait(&exitStatus);
   exitCode = WEXITSTATUS(exitStatus);

   if (exitCode & SAFERUN_ERR) {
      test->errors |= exitCode >> 2 & RUNTIME_ERR;
      test->errors |= exitCode << 2 & TIMEOUT;
   }
}

static void DiffTest(Test *test) {
   char **cmdArgs, **thisArg;
   int nullFD = open("/dev/null", O_WRONLY);
   int childPID, exitStatus, exitCode;
   char outFile[MAX_WORD_LENGTH + 1];
   char diff[MAX_WORD_LENGTH + 1];

   strcpy(outFile, "test.output.temp");

   strcpy(diff, "diff");


   if ((childPID = fork()) < 0)
      fprintf(stderr, "Error, cannot fork.\n");
   else if (childPID) { // We are parent!
      close(nullFD);
   } else { // We are child!

      cmdArgs = thisArg = calloc(sizeof(char *), 4);

      *thisArg++ = diff;
      *thisArg++ = outFile;
      *thisArg = test->outFile;

      dup2(nullFD, 1);
      dup2(nullFD, 2);
      close(nullFD);

      execvp(*cmdArgs, cmdArgs);
      fprintf(stderr, "Error, cannot execute.\n");
      exit(1);

   }

   wait(&exitStatus);
   exitCode = WEXITSTATUS(exitStatus);
   test->errors |= exitCode & DIFF_FAIL;
}

static void PrintResults(Program *program) {
   Test *temp = program->test;
   int hasFailures = 0, hasErrors, i;

   for (i = 0; temp != NULL; temp = temp->next, i++) {
      if (temp->errors != 0) {
         hasFailures = 1;
         hasErrors = 0;
         printf("%s test %d failed: ", program->execName, i + 1);

         if (temp->errors & DIFF_FAIL) {
            printf("diff failure");
            hasErrors = 1;
         }

         if (temp->errors & RUNTIME_ERR) {
            if (hasErrors)
               printf(", runtime error");
            else {
               printf("runtime error");
               hasErrors = 1;
            }
         }

         if (temp->errors & TIMEOUT) {
            if (hasErrors)
               printf(", timeout");
            else
               printf("timeout");
         }

         printf("\n");

      }
   }

   if (!hasFailures)
      printf("%s %d of %d test passed.\n", program->execName, i, i);
}


static void ExecProgram(Program *program) {
   Test *temp = program->test;

   for (; temp != NULL; temp = temp->next) {
      ExecTest(program->execName, temp);
      DiffTest(temp);

   }
}

// clears, deletes the temp dir & moves back to parent dir
static void DeleteDir(char *dirName) {
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

static void RunProgram(Program *program) {
   for (; program != NULL; program = program->next) {
      MkTempDir(program);
      TransFiles(program);
      if (!CompileProg(program)) {
         ExecProgram(program);
         PrintResults(program);
      }
      DeleteDir(program->tempDirName); // clears, deletes the temp dir & moves back to parent dir
   }
}


int main(int argc, char **argv, char **envp) {
	printf("**************************************************\n");              
   printf("Optional Arguments:\n\td : Runs debug output options\n");
   printf("**************************************************\n"); 

	// check through arguments for debug option
   for (char **argt = argv; *argt != NULL; ++argt)
      if (**argt == 'd') { __debug = 1; }   
 
   if (__debug && checkFileExists(argv[1]))
         printf("- Input Files Found...\n");  
			
   Program *program = ReadSuite(fopen(argv[1], "r"));
	
	// if we want a list of the test and programs to be printed
   if(__debug) 
      debugPrintProgram(program); 
	
	// if the program was read in correctly then run the program
   if (program != NULL)
      RunProgram(program);
	
	// else something went wrong
   else
	   printf("Program: %s: was not read successfully\n", argv[1]);
	
	printf("**************************************************\n");
   printf("Calculon exiting...\n");
	
   return 0;
}

