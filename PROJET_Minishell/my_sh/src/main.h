#ifndef MAIN
#define MAIN
 
// #include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include "linkedstring.h"
#include "history.h"
#include "struct.h"
#include "tools.h"

//ref 
void main(int argc,char *argv[]);
int console();

//Main stuff
// void main(int argc,char *argv[]);
// int console();
int pipeSplit(char *command);
int startProcess(char *command);
void superParse(char*command);
void parseComma(char *command);
int execCommand(char *command);
char *execRedirection(char *command);
int execSpecificBuiltIn(int status);

//Chain management
int runRoot();
void runLevel(Command* command);
int runChain(Command* command);
void printRoot();
void printLevel(Command* command);
void printChain(Command* command);

int executeCmd();

#endif
