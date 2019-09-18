#include "testHandler.h"

#include "debugger.h"
#include "suiteHandler.h"

Test *readTest(FILE *in) {
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


void testProgram(Program *program) {
   printf("testing each program\n");
   Test *temp = program->test;

   for (; temp != NULL; temp = temp->next) {
      execProgram(program->execName, temp); 
   }
}


void execTest(char *execName, Test *test) {
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

void diffTest(Test *test) {
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


void printTestResults(Program *program) {
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
