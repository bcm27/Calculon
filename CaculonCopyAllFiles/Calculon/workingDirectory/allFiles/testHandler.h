#ifndef TESTHANDLER_H
#define TESTHANDLER_H

//standard
#include <stdio.h>
#include <stdlib.h>

// for opening files
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

// for file streams
#include <unistd.h>

// personal file types 
#include "dataTypes.h"

// functions
Test* readTest(FILE *inFile);

void testProgram(Program *program);

void execSingleTest(char* execName, Test *test);

void diffSingleTest(Test* test);

void printStackTestResults(Program* program);

#endif
