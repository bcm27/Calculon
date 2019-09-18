#ifndef SUITEHANDLER_H
#define SUITEHANDLER_H

// standard included libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// included for opening and file use
#include <sys/wait.h>
#include <sys/types.h>

// required for file descriptors
#include <fcntl.h>

// required for compilation
#include "dataTypes.h"

//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
Program* readProgram(FILE *inFile);

//***************************************************************************//
// Function: reads the program data into the proper containers
// Requitments: a suite file that contains at least one T and P commands
//***************************************************************************//
Program* readSuiteToProgram(FILE *inFile);

//***************************************************************************//
// Function: 
// Requitments: 
//***************************************************************************//
void execProgram(char *programName, Test *test);

//***************************************************************************//
// Function: if a make file exists compiles the program, otherwise constructs
//           a gcc script and exicutes this command upon the files designated
//           by the SuiteX.suite file
// Requitments: program files that are dictated within the SuiteX.suite or
//              a make file
//***************************************************************************//
int compileProgram(Program *program);

#endif
