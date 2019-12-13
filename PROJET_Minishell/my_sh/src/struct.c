
/*! \file  
    \brief Contains functions to parse and store conditional commands.

    This files contains function to create structures from text commands.
    The parsing is done recursively and main.c has access to the root command to execute in order.
    This file also contains a method to free the structure once it is not needed anymore.
*/

#include "struct.h"

/*! 
    \brief Adds "&&" separated commands as the new root.
    \param Commands separated by OR symbols char* command.
    \return Void.

    Once this chain of OR commands is parsed we replace the root of the conditional commands.
    Then the old root is set as the next command in the new root.
    The IF command chains are fed in reverse in this function to build the structure.
*/
void addRootCommand(char *command)
{

    //Create new command root
    Command *newRoot = processCommand(command);

    //We set the new root's next command as the old root
    newRoot->nextCommand = root;

    //Then the new root takes the old one's place
    root = newRoot;
}

/*! 
    \brief Processes a chain of OR commands and builds the new root from them.
    \param The chain of commands char * command.
    \return The new root is returned Command pointer.
    
    This function takes a chain of OR commands, splits them and builds the new root from the first command.
    The function then takes the remaining part of the chain (The other OR commands) and build the chain by calling itself.
    The new root command is then returned.
*/
Command *processCommand(char *command)
{

    //Create new command
    Command *newCommand = malloc(sizeof(Command));

    //Safety null
    newCommand->nextCommand = NULL;
    newCommand->orCommand = NULL;

    //Explode the command with the or separator
    const char *separator = "&&";
    char *token = strtok(command, separator);

    int marker = 0;
    char *residue = NULL;

    //The first part is stored locally the rest is concatenated and sent to a recursion processing
    while (token)
    {

        char *currentCommand;
        currentCommand = strdup(token);
        token = strtok(NULL, separator);

        //First element is the base command
        if (marker == 0)
        {
            newCommand->command = currentCommand;
        }
        else
        {
            //The rest is the residue
            if (residue != NULL)
            {
                residue = quickConcat(residue, "&&");
                residue = quickConcat(residue, currentCommand);
            }
            else
            {
                residue = currentCommand;
            }
        }

        marker++;
    }

    //If there is a residue we process its chain
    if (residue != NULL)
    {
        newCommand->orCommand = processCommand(residue);
    }
    return newCommand;
}

/*! 
    \brief Tool function to concatenate two char pointers.
    \param The first char * a.
    \param The second char * b.
    \return A char pointer made of the two parameters concatenated char *.

    It concatenates two char pointers and returns a new one made of the two parameters.
*/
char *quickConcat(char *a, char *b)
{
    char *returnCharPointer;
    returnCharPointer = malloc(strlen(a) + strlen(b));
    strcpy(returnCharPointer, a);
    strcat(returnCharPointer, b);
    return returnCharPointer;
}

/*!
    \brief Frees recursively a Command type pointer.
    \param Command to free recursively Command *.
    \return Void.

    It frees a Command type pointer and it's ramifications recursively to free some memory. 
*/
void freeCommand(Command *command)
{
    if (command == NULL)
    {
        return;
    }
    if (command->nextCommand != NULL)
    {
        freeCommand(command->nextCommand);
    }
    if (command->orCommand != NULL)
    {
        freeCommand(command->orCommand);
    }
    free(command);
}

/*!
    \brief Frees the current root command of BugBash.
	\return Void.
    
    Frees the current root command of BugBash recursively if the pointer is not null.
*/
void freeRoot()
{
    if (root != NULL)
    {
        freeCommand(root);
        root = NULL;
    }
}

/*! 
    \brief Get the current root.
	\return Command pointer of the current root Command *.
	
    Will return the current root command of BugBash.
*/
Command *getRootCommand()
{
    return root;
}