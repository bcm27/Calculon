#include "fileHandler.h"

void transferFiles(Program* program) {
   Arg *fileToCopy;

   // if the makefile exists copy first
   if (access("Makefile", F_OK) != -1)
      copySingleFile(program->tempDirName, "Makefile");

   // copy each file required for the program
   for (fileToCopy = program->arg; fileToCopy != NULL; 
    fileToCopy = fileToCopy->next) {
      copySingleFile(program->tempDirName, fileToCopy->value);
   }

   // copy each file for the tests and associated with the tests
   for (Test *test = program->test; test != NULL; test = test->next) {
      copySingleFile(program->tempDirName, test->inFile);
      copySingleFile(program->tempDirName, test->outFile);
   }

} // end function

void copySingleFile(char* dirName, char* fileName) {
   char **cmdHeadLine, **cmdTemp;
   char dest[MAX_WORD_LENGTH];
   char slash[] = "/";
   int childPID;

   // allocates space so both pointers share the same memory
   cmdHeadLine = cmdTemp = calloc(sizeof(char *), 4);
   // if the file we want to copy exists

   if(checkFileExists(fileName)) {

      // builds the destination part of the argument
      strcpy(dest, dirName);
      strcat(dest, slash);
      strcat(dest, fileName);

      // starts the cp command creation
      *cmdTemp = "cp";
      *++cmdTemp = fileName; // to create
      *++cmdTemp = dest;    // destination added

      if ((childPID = fork()) < 0) {
         fprintf(stderr, "Error, cannot fork.\n");
         perror("");
      } else if (!childPID) {
         execvp(*cmdHeadLine, cmdHeadLine);
         exit(-1);
      }
      wait(NULL);
   } // end file 

   else 
      printf("File to copy does not exist: %s\n", fileName);
   
   // deallocate the space for the command line variables   
   free(cmdHeadLine);
} // end function

void makeTempDirectory(Program *program) {
   int pid = getpid();
   char strpid[MAX_WORD_LENGTH + 1];

   sprintf(strpid, "%d", pid);
   strcat(program->tempDirName, strpid);
   mkdir(program->tempDirName, 0777);
}

int checkFileExists(char *fileName) {
   //FILE* fp = fopen("codeout.txt", "a+");

   if (access(fileName, F_OK) == -1) {  // file doesnt exists
      printf("Could not find required test file '%s'\n", fileName);
      //fprintf(fp, "Could not find required test file '%s'\n", fileName);
      //fprintf(fp, "Your program exited with nonzero code 1\n");
      exit(1);
   }
   //fclose(fp);
   return 1;
}// end function
