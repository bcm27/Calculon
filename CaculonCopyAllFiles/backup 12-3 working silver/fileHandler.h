#ifndef FILEHANDLER_H
#define FILEHANDLER_H

//standard
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

// required for compile
#include "dataTypes.h"

void transferFiles(Program* program);

void copySingleFile(char* dirName, char* fileName);

void MkTempDir(Program *program);

int checkFileExists(char* fileName);

#endif
