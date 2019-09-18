#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PROG 32
#define NUM_ERROR_MSG 3
#define MAX_F_LNGTH 32

int main(){
   char *error[NUM_ERROR_MSG] = {"diff failure", "timeout", "runtime error"};
   int exitcode = 0;
   char *temp_path = {"./temp"};


   printf("************************************************\n");
   printf("Maximum Number of program runs: %d\n",MAX_PROG);
   printf("Possible failures:\n\t%s\n\t%s\n\t%s\n", error[0],
      error[1], error[2]);

   char *fileName = {"/example.txt"};

   if(!mkdir((*temp_path + *fileName), 0777)){
      printf("\tTemp Directory Created: %s\n", temp_path);
   }

   printf("************************************************\n");
   printf("Closing...\n");
   return exitcode;
}
