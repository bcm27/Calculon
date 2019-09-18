#include "dataTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Arg* NewArg(char* input) {
   Arg *retArg = malloc(sizeof(Arg));
   strncpy(retArg->value, input, MAX_WORD_LENGTH);
   retArg->next = NULL;

   return retArg;
}
