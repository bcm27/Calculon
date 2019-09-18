#include <stdio.h>
#include "main.h"


// most header files are included elsewhere for data requirements however
// every file used at compile time has been included for redundency
#include "debugger.h"        // holds debugging variables and functions
#include "fileHandler.h"     // handles the copying and transfer of files
#include "suiteHandler.h"    // handles the suite population
#include "testHandler.h"     // handles the testing for each program

void runProgram(Program *program) {
   
   for (; program != NULL; program = program->next) {
      makeTempDirectory(program);  // makes a temp directory per program
      transferFiles(program); // tranfers the files to the temp dir
      	
      if (!compileProgram(program)) { // if the program compiled         
      // run through each test and check the difference
         for (Test *temp = program->test; temp != NULL; temp = temp->next) {
	    execSingleTest(program->execName, temp);
	    diffSingleTest(temp);
         }
	 // print the results for each test
         printStackTestResults(program);
      }
      //else 
      //   printf("%s exited with bad error code\n", program->execName);
				
      // deletes the temporary directory
      DIR *dir;
      DirEntry *dirEntry;

      dir = opendir(".");
      while (NULL != (dirEntry = readdir(dir))) {
         remove(dirEntry->d_name);
      }
      // go back to the parent directory
      chdir("..");
      closedir(dir);
      remove(program->tempDirName);
   } // for each program
}

//***************************************************************************//
// Function: main
// Requitments: 
//***************************************************************************/
int main(int argc, char **argv, char **envp) {
   //printf("**************************************************\n");              
   //printf("Optional Arguments:\n\td : Runs debug output options\n");
   //printf("**************************************************\n"); 
//   int fp = open("codeout.txt", O_WRONLY | O_CREAT, 0777);
   //FILE* fp = open(*argv + "/codeout.txt", "a+");
//   dup2(fp, 1);
//   dup2(fp, 2);

   // check through arguments for debug option
   for (char **argt = argv; *argt != NULL; ++argt)
      if (**argt == 'd') { __debug = 1; }   
 
   if (__debug && checkFileExists(argv[1]))
         printf("- Input Files Found...\n");  
			
   // argv[1] is the suite name, ie Suite1.suite
   Program *program = readSuiteToProgram(fopen(argv[1], "r"));
	
   // if we want a list of the test and programs to be printed
   if(__debug) 
      debugPrintProgram(program); 
	
   // if the program was read in correctly then run the program
   if (program != NULL)
      runProgram(program);
	
   // else something went wrong
   else
      printf("Program: %s: was not read successfully\n", argv[1]);

   if (__debug) {	
      printf("**************************************************************\n");
      printf("Calculon exiting...\n'CTR+D' exits SafeRun\n");	
   }

   return 0;
}
