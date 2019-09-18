#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdio.h>
#include <stdlib.h>
#include "dataTypes.h"

extern int __debug;

void debugPrintProgram(Program *program);
Arg* NewArg(char *input);

#endif
