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


void execSingleTest(char *execName, Test *test) {
   // used for building the construction of exec commands
   char **cmdHeadLine, **cmdTemp;
   Arg *arg = test->arg;
   // holders for the file descriptors for outputting the test exec results
   int inFD = open(test->inFile, O_RDONLY);
   int outFD = open("test.output.temp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
   // forked proccess results
   int childPID, exitStatus, exitCode;
   // string arrays for the construction of the arguments
   char strCPUtime[MAX_WORD_LENGTH + 1],
        strWCtime[MAX_WORD_LENGTH + 1],
        argCPU[MAX_WORD_LENGTH + 1],
        argWC[MAX_WORD_LENGTH + 1], 
        name[MAX_WORD_LENGTH + 1];

   //allocates memory for the command lines
   cmdHeadLine = cmdTemp = calloc(sizeof(char *), test->numOfArgs + 6);

   // construct cpu time argument
   strcpy(argCPU, "-t");
   sprintf(strCPUtime, "%d", test->timeout);
   strcat(argCPU, strCPUtime);

   // constructs the wall clock time for each program
   strcpy(argWC, "-T");
   sprintf(strWCtime, "%d", 10 * test->timeout);
   strcat(argWC, strWCtime);

   // adds the current file directory
   strcpy(name, "./");
   strcat(name, execName);

   // this is the only forking method that works, borrowed from clints modules
   if ((childPID = fork()) < 0) // if failed
      fprintf(stderr, "Error, cannot fork\n");

   else if (childPID) { // parent
      close(inFD);
      close(outFD);
   }
   else { // child      
      // construct the saferun default arguments
      *cmdTemp++ = "SafeRun";
      *cmdTemp++ = "-p50";
      *cmdTemp++ = argCPU;
      *cmdTemp++ = argWC;
      *cmdTemp = name;
      // adds all test argument params to the command line
      for (; arg != NULL; arg = arg->next) {
         *++cmdTemp = arg->value;
      }
      // changes the file outputs to match the file directory of the temp file
      dup2(inFD, 0);
      dup2(outFD, 1);
      dup2(outFD, 2);
      close(inFD);
      close(outFD);

      execvp(*cmdHeadLine, cmdHeadLine);
      fprintf(stderr, "ERROR: cannot execute\n");
      exit(1);
   }
   wait(&exitStatus);
   exitCode = WEXITSTATUS(exitStatus);

   if (exitCode & SAFERUN_ERR) {      
      test->errors |= exitCode << 2 & TIMEOUT_ERR;
      test->errors |= exitCode >> 2 & RUNTIME_ERR;
   }
}

void diffSingleTest(Test *test) {
   char **cmdHeadLine, **cmdTemp;
   char outFile[MAX_WORD_LENGTH + 1];
   char diff[MAX_WORD_LENGTH + 1];
   int emptyFD = open("/dev/null", O_WRONLY);
   int childPID, exitStatus, exitCode;

   strcpy(outFile, "test.output.temp");

   strcpy(diff, "diff");

   // this is the only forking formate that works, borrowed from clints modules
   if ((childPID = fork()) < 0) // failed
      fprintf(stderr, "ERROR: cannot fork\n");
   else if (childPID) { // parent
      close(emptyFD); // remove FD from parent
   } 
   else { // child
      cmdHeadLine = cmdTemp = calloc(sizeof(char *), 4);
      
      // constructs the commandline to diff
      *cmdTemp++ = diff; // command
      *cmdTemp++ = outFile; // first file to check
      *cmdTemp = test->outFile; // second file to check

      // turn off the commandline output
      dup2(emptyFD, 1); 
      dup2(emptyFD, 2);
      close(emptyFD);

      // execute the diff command
      execvp(*cmdHeadLine, cmdHeadLine);
      fprintf(stderr, "ERROR: cannot execute\n");
      exit(2); // if we fail kill the program, according to documentation:
               // 1 = differences were found
               // 2 =  means trouble with diffing
   } // end child
 
   // get exit codes
   wait(&exitStatus);
   exitCode = WEXITSTATUS(exitStatus);
   // add the diff errors IF ANY
   test->errors |= exitCode & DIFF_FAIL_ERR;
} // end function


void printStackTestResults(Program *program) {
   Test *temp = program->test;
   int hasFailures = 0, hasErrors, i;
   //FILE* fp = fopen("../codeout.txt", "a+");

   for (i = 0; temp != NULL; temp = temp->next, i++) {
      if (temp->errors != 0) {
         // for each test reset the errors
         hasFailures = 1;
         hasErrors = 0;
         printf("%s test %d failed: ", program->execName, i + 1);
         
         // check each error individually
         if (temp->errors & DIFF_FAIL_ERR) {
            printf("diff failure");
            hasErrors = 1;
         }
         // proper order of statements accoridng to output prioritity         
         if (temp->errors & RUNTIME_ERR) {
            if (hasErrors >= 1)
               printf(", runtime error");
            else {
               printf("runtime error");
               hasErrors = 1;
            }
         } 

         if (temp->errors & TIMEOUT_ERR) {
            if (hasErrors >= 1)
               printf(", timeout");
            else
               printf("timeout");
         }

         
         // add new line for formatting
         //fflush(fp);
         printf("\n");
      } // when no errors are found
 
      //fflush(fp);
      if(__debug)
         printf("no errors for test: %s\n", program->execName);

   } // end tests

   
   if (!hasFailures) {
      printf("%s %d of %d tests passed.\n",program->execName, i, i);
      //fprintf(fp,"%s %d of %d tests passed.\n", program->execName, i, i);
      //fflush(fp);
   }
   //fclose(fp);

} // end function
