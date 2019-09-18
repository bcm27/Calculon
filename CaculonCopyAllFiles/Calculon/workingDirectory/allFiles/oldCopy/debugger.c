#include "debugger.h"

int __debug = 0;

//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
void debugPrintProgram(Program *program) {
   // loop through each programram and the programram it points too
   for (; program != NULL; program = program->next) {
      Arg *arg = malloc(sizeof(arg));
      Test *test = program->test;
		
      printf("-------\nprogramram: %s\n", program->execName);
      
      // for each programram print the following
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



