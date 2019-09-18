static void CopySingleFile(char *directoryName, char *fileName) { 

   char **cmdArgs, **thisCommand;
   cmdArgs = thisArg = calloc(sizeof(char *), 4);
   char result[MAX_WORD_LENG + 1];
   char slash[] = "/";

   //CheckFileExists(fileName);
   if (access(fileName, F_Ok) == -1) {
      printf("File does not exist: %s\n", fileName);
      exit(-1);
   }

   //create file path to copy inside result
   // example: home/a.out
   strcpy(result, directoryName);
   strcpy(result, slash);
   strcpy(result, fileName);
   
   *thisCommand = "cp";
   *++thisCommand = fileName;
   *++thisCommand = result;
   // Command should now look like: cp fileName path/fileName
   // next we create a child brainwipe and execute the command
   int childPID;
   if ((childPID = fork()) < 0) {
      fprintf(stderr, "Error, cannot fork.\n");
      perror(""); //http://www.cplusplus.com/reference/cstdio/perror/ 
                  //perror needs to be called for fprintf to work
                  //fpintf bypasses any buffers and prints 
   }

   else if (!childPID) {
      execvp(*cmdArgs, cmdArgs);
      fprintf("Error in: CopyingSingleFile command execution\n\n");
      perror("");
      exit(-1); // should never fail...hopefully
   }

   wait(NULL); // wait for all children to finish
   wait(childPID); // just double checking that the cp is finished
   // should be redundant because of the WAIT(NULL)

   free(cmdArgs);
}


//***************************************************************************//
// Function: TransferFiles
// Description: Transfers all files to the temp directory indicated by the 
//              program struct file. Creates a hard copy of makefile, test
//              files and program files
// Dependencies: no errors will be emitted if files are not found, however 
//               nothing will be copied and the function will do nothing
//***************************************************************************//
static void TransferFiles() { 
   Arg *tempFile;
   // if the make file exists, copy it
   if (access("Makefile", F_Ok) != -1)
      CopySingeFile(program->tempDirName, "Makefile");

   // copies the program file
   for(tempFile = program->files; tempFiles != NULL; tempFile = tempFile->next) 
      CopySingleFile(program->tempDirName, tempFile->Value);

   // copy for remaininig tests
   for (Test *test = program->tests; test != NULL; test = test->next) {
      CopySingleFile(program->tempDirName, test->inFile);
      copySingleFile(program->tempDirName, test->outFile);
   }
   // no need to copy over obj files or binaries
   /*
   if (access("Suite1.suite", F_Ok) != -1)
   */
}