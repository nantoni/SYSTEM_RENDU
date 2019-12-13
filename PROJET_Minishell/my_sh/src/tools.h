#ifndef TOOLS
#define TOOLS

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "linkedstring.h"

//Tools
char* concat(const char* str1, const char* str2);
char** str_split(char* a_str, const char a_delim,int* length);
char *multi_tok(char *input, char *delimiter);
char *replaceSingleChar(char *input,char target,char replacer);
List *strSuperSplit(char *input,char *delimiter,int option);
char *trimwhitespace(char *str);
int redirect(char *filename,int flags, int destfd);
void sleep_ms(int milliseconds);

#endif