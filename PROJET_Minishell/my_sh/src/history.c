
/*! \file  
    \brief Contains history functions

    Contains history functions for command logging purposes.
*/

#include "history.h"

char *fileName;
FILE *f = NULL;
int logFileOpened = 1;

/*!  
    \brief Saves a command.
    \param Command being saved char * command.
    \return Void.

	Saves a given command into a log file.
	Once created the file stays open until the program is terminated.
*/
void saveCommand(char *command)
{

    // Check if the file has already been opened
    if (logFileOpened == 1)
    {
        f = fopen("./logs/history.log", "a");
        if (f == NULL)
        {
            perror("Error opening file.");
        }else{
            logFileOpened = 0;
        }
    }

    fprintf(f, "%s\n", command);
    fflush(f);
}

/*!  
    \brief Closes the log file.
    \return Void.

	Closes the log file if the pointer is not null.
*/
void closeLog()
{
    if (f != NULL)
    {
        fclose(f);
    }
}
