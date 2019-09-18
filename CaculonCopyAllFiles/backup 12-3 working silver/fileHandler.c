#include "fileHandler.h"

void transferFiles(Program* program) {
   Arg *tFile;

   for (tFile = program->arg; tFile != NULL; tFile = tFile->next) {
      copySingleFile(program->tempDirName, tFile->value);
   }

   // Copying for all test in the program input and output arg
   for (Test *test = program->test; test != NULL; test = test->next) {
      copySingleFile(program->tempDirName, test->inFile);
      copySingleFile(program->tempDirName, test->outFile);
   }

   if (access("Makefile", F_OK) != -1) {  // makefile exists
      copySingleFile(program->tempDirName, "Makefile");
   }
}

void copySingleFile(char* dirName, char* fileName) {
   char **cmdArgs, **thisArg;
   cmdArgs = thisArg = calloc(sizeof(char *), 4);
   char result[MAX_WORD_LENGTH];
   char slash[] = "/";
   int childPID;

   checkFileExists(fileName);

   strcpy(result, dirName);
   strcat(result, slash);
   strcat(result, fileName);

   *thisArg = "cp";
   *++thisArg = fileName;
   *++thisArg = result;

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

void MkTempDir(Program *program) {
   int pid = getpid();
   char strpid[MAX_WORD_LENGTH + 1];

   sprintf(strpid, "%d", pid);
   strcat(program->tempDirName, strpid);
   mkdir(program->tempDirName, 0777);
}

int checkFileExists(char *fileName) {
   if (access(fileName, F_OK) == -1) {  // file doesnt exists
      printf("Could not find required test file '%s'\n", fileName);
      return 0;
   }
   return 1;
}
