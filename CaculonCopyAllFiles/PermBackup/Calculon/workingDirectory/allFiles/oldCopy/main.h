#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

typedef struct dirent DirEntry;

#include "dataTypes.h"       // holds the global data types, test, program, arg

int main();
void runProgram(Program* program);

#endif
