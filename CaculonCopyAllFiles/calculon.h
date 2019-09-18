#ifndef _calculon_h
#define _calculon_h

#include "Util.h"
#define MAX_WORD_LENGTH 100

typedef struct Arg {
	char value[MAX_WORD_LENGTH + 1];
   struct Arg *next;	
}

//Tests
typedef struct Test {
   char inFile[MAX_WORD_LENGTH + 1]; 
   char outFile[MAX_WORD_LENGTH + 1]; 
   int timeout;                   
   int numOfArgs;                 
   Arg *arg;                      
   struct Test *next;              
} Test;

typedef struct Program {
   char *execName;                    
   char *tempDirName;                 
   struct Program *next;             
   Arg *arg;                         
   Test *test;                        
} Program;


static int compileProgram(Program *program);

static void runProgram(Program *program);

static Program* readProgram(FILE *inFile);

static Program* readSuiteToProgram(FILE *inFile);

#endif