#ifndef LINKEDSTR
#define LINKEDSTR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct NodeTag {
    char *data;
    struct NodeTag *next;
} Node;

Node *Node_create();
void Node_destroy(Node *node);

typedef struct ListTag {
    struct NodeTag *first;
} List;

List *List_create();
void List_destroy(List *list);

void List_append(List *list, char *str);
void List_insert(List *list, int index, char *str);

char *List_get(List *list, int index);
int List_find(List *list, char *str);

void List_remove(List *list, int index);
char *List_pop(List *list, int index);

int List_length(List *list);
void List_print(List *list);

#endif