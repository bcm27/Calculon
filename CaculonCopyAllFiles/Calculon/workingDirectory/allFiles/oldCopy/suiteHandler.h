#ifndef SUITEHANDLER_H
#define SUITEHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#include "dataTypes.h" // Arg, Test and Program structs

Program* readProgram(FILE *inFile);

Program* readSuiteToProgram(FILE *inFile);

void execProgram(char *programName, Test *test);

int compileProgram(Program *program);

#endif
