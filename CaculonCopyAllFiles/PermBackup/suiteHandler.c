#include "suiteHandler.h"
#include "testHandler.h" // for tests
#include "debugger.h" // personal debugging stuff

Program* readProgram(FILE *inFile) {
 
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
} // end read program 


Program* readSuiteToProgram(FILE *inFile) {
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
} // end read suite to program

void execProgram(char *programName, Test *test) {
   // build file outputs for tests
   int inFileDir = open(test->inFile, O_RDONLY);
   int outFileDir = open("test.output.temp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
   
   // program status holders
   int childPID, status, exitCode;
   
   // variables for arguments from tests that are read in
   char **cmdHeadLine, **cmdTemp;
   
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
      cmdHeadLine = cmdTemp = calloc(sizeof(char *), test->numOfArgs + 6);
      // build the command line
      *cmdTemp++ = "SafeRun";
      *cmdTemp++ = "-p50";
      *cmdTemp++ = timeOut;
      *cmdTemp++ = clock;
      *cmdTemp = name;  
      //example: SafeRun -p50 -t1000 -T10 a.out

      for (; arg != NULL; arg = arg->next) {
         *++cmdTemp = arg->value;

         if (__debug) 
            printf("%s ", arg->value);
      } // end of adding suite arguments to cmd

      if (__debug) 
         printf("\n");

      // swap the file outputs
      dup2(inFileDir, 0);
      close(inFileDir);

      dup2(outFileDir, 1);
      dup2(outFileDir, 2);
      close(outFileDir);

      // run whatever program we have created 
      execvp(*cmdHeadLine, cmdHeadLine);
      fprintf(stderr, "ERROR: cannot execute command exec\n");
      // command not found error
      exitCode = 127;
   }

   wait(&status);
   exitCode = WEXITSTATUS(status);

   if (exitCode) { 
      // print the commands that resulted in the failure
      printf("Failed, commands:");
      for (cmdTemp = cmdHeadLine; *cmdTemp != NULL; cmdTemp++) {
         printf(" %s", *cmdTemp);
      }
      printf("\n");     
   }   

   if (exitCode & SAFERUN_ERR) {
      test->errors |= exitCode >> 2 & RUNTIME_ERR;
      test->errors |= exitCode << 2 & TIMEOUT_ERR;
   }
} // end exec program

int compileProgram(Program *program) {
   int numbOfcurrentFiles = 0, nullFD = open("/dev/null", O_WRONLY);
   int childPID, status, exitCode;
   Arg *itArg, *HeadcurrentFile = NULL, *currentFile;
   char **cmdHeadLine, **cmdTemp;
   char *extension;

   chdir(program->tempDirName);
   printf("program path changed: %s\n", program->tempDirName);

   for (itArg = program->arg; itArg != NULL; itArg = itArg->next) {
      extension = strchr(itArg->value, '.');
      if (!extension) {
         printf("%s has no file extension found.", itArg->value);
      } else {
         // if the file is a .c
         if (*(extension + 1) == 'c' && *(extension + 2) == '\0') {  
            numbOfcurrentFiles++;
            
            Arg *argTemp = malloc(sizeof(Arg));
            strncpy(argTemp->value, itArg->value, MAX_WORD_LENGTH);
            argTemp->next = NULL;
   
            currentFile = argTemp;
            currentFile->next = HeadcurrentFile;
            HeadcurrentFile = currentFile;
         }
      }
   }

   cmdHeadLine = cmdTemp = calloc(sizeof(char *), numbOfcurrentFiles + 4);

   // dont know why function does not work here
   if (access("Makefile", F_OK) != -1) {
   //if(checkFileExists("Makefile") {
      *cmdTemp++ = "make";
   } 
   else {
      *cmdTemp++ = "gcc";
      for (currentFile = HeadcurrentFile; currentFile != NULL;
       currentFile = currentFile->next) {
         *cmdTemp++ = currentFile->value;
      }
      *cmdTemp++ = "-o";
   }
   //
   if (__debug) {
      for (cmdTemp = cmdHeadLine; *cmdTemp != NULL; cmdTemp++) {
         printf(" %s", *cmdTemp);
      }
      printf("\n");
   }
   //
   *cmdTemp = program->execName;
   
   if ((childPID = fork()) < 0) { // fork failed
      fprintf(stderr, "ERROR: cannot fork.\n");
   } 
   else if (childPID) { // parent
      close(nullFD);
   }
   else { // parent
      dup2(nullFD, 1);
      close(nullFD);
      execvp(*cmdHeadLine, cmdHeadLine);
      exit(-1);
   }
   wait(&status);
   exitCode = WEXITSTATUS(status);

   if (exitCode) {
      printf("Failed:");
      for (cmdTemp = cmdHeadLine; *cmdTemp != NULL; cmdTemp++) {
         printf(" %s", *cmdTemp);
      }
      printf("\n");
   }

   free(cmdHeadLine);
   return exitCode;
}
