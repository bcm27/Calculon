#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdio.h>
#include <stdlib.h>
#include "dataTypes.h"

// global debug flag
extern int __debug;

//***************************************************************************//
// Function: loops through a program and prints all the tests and the values
//           associated with the program such as arguments, etc.
// Requitments: a program is specified to loop check
//***************************************************************************//
void debugPrintProgram(Program *program);

#endif
