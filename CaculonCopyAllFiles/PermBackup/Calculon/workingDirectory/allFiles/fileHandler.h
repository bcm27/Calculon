#ifndef FILEHANDLER_H
#define FILEHANDLER_H

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// for opening files
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

// for creating files
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

// required for compilation 
#include "dataTypes.h"

//***************************************************************************//
// Function: transfers the files associated with each program, and test
// Requitments: a program to copy the files of
//***************************************************************************//
void transferFiles(Program* program);

//***************************************************************************//
// Function: copies a single file to a particular destination
// Requitments: a file name and directory name
//***************************************************************************//
void copySingleFile(char* dirName, char* fileName);

//***************************************************************************//
// Function: creates a temp file
// Requitments: a program exists
//***************************************************************************//
void makeTempDirectory(Program *program);

//***************************************************************************//
// Function: checks for a particular file within the current working directory
//           and if it does not exist returns 0
// Requitments: a file name
//***************************************************************************//
int checkFileExists(char* fileName);

#endif
