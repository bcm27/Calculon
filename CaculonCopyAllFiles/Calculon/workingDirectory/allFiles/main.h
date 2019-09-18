#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

typedef struct dirent DirEntry;

#include "dataTypes.h"       // holds the global data types, test, program, arg


//***************************************************************************//
// Function: creates a temp folder for each program, copies the req files into
//           the folder and compiles the code
// Requitments: a program and all functioning data has been populated
//***************************************************************************//
void runProgram(Program* program);


//***************************************************************************//
// Function: 
// Requitments: the program is run with the argument Suite<number>.suite
//***************************************************************************//
int main();

#endif
