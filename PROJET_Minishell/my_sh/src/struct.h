#ifndef STRUCT
#define STRUCT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "typedef.h"

Command* root;
 
void addRootCommand();
Command* processCommand(char*command);
char* quickConcat(char*a,char*b);
void freeCommand(Command* command);
void freeRoot();
Command* getRootCommand();

#endif