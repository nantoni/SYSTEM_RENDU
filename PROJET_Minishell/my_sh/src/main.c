
/*! \file  
    \brief my_sh main.c

    This c file is the core of my_sh from here all commands are entered, parsed and executed.
*/

#include "main.h"

//Main vars
int lastStatus = 0;
int transferFileDesc = NULL;

//For built in support
int exitFileDesc = NULL;
int cdFileDesc = NULL;

/*! 
    \brief Starts the main application with arguments.
    \param Argument array size int argc.
    \param Argument array char *argv[].
    \return Void.

    Starts the main application with arguments.
    Standard main declaration.
*/
void main(int argc, char *argv[])
{
    int runLoop = 1;
    int returnCode = 0;

    //Check for batch mode boot
    if (argc >= 2)
    {

        //Batch mode
        char *check1 = "-c";
        char *check2 = "-C";

        //Run a command in batch mode
        if (strcmp(check1, argv[1]) == 0 || strcmp(check2, argv[1]) == 0)
        {
            if (argc >= 3)
            {
                char *querry = "";
                char *spaceSeparator = " ";
                int i;
                for (i = 2; i < argc; i = i + 1)
                {
                    querry = concat(querry, concat(spaceSeparator, argv[i]));
                }
                printf("Exec in batch mode -> %s\n", querry);
                returnCode = pipeSplit(querry);
                runLoop = 0;
            }
        }
    }

    //Run in console mode
    if (runLoop)
    {

        //Enter loop
        while (1)
        {
            //It's important to clear the transfer file descriptor between commands
            //because it might interact with the stdin of the next command
            if (transferFileDesc != -1)
            {
                transferFileDesc = -1;
            }
            //Prompt the next command
            returnCode = console();
        }
    }

    //Ends by returning the code
    exit(returnCode);
}

/*! 
    \brief Prompts the user for a command then executes it.
    \return Return code of last command int.

    Uses the function getchar(); to build a char pointer of dynamic size.
    There is no limit to a command thanks to the realoc.
    Once a command has been submited it's sent to the pipeSplit(char*) function.
*/
int console()
{
    int max = 20;
    char *prompt = (char *)malloc(max); /* allocate buffer */

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    printf("[%s]$ ", cwd);

    int i = 0;
    while (1)
    {
        int c = getchar();
        if (c == 10)
        { /* at end, add terminating zero */
            prompt[i] = 0;
            break;
        }
        prompt[i] = c;
        if (i == max - 1)
        { /* buffer full */
            max += max;
            prompt = (char *)realloc(prompt, max); /* get a new and larger buffer */
        }
        i++;
    }

    if (strlen(prompt) != 0)
    {

        //First save the command
        saveCommand(prompt);

        //Start processing the command
        pipeSplit(prompt);
    }

    return 0;
}

/*! 
    \brief Prompts the user for a command then executes it.
    \param Raw command pointer char * command.
    \return Return code of the command int.

    Splits the raw command by the pipe "|" symbol, 
    each of the sub commands will run in a different subprocess.
    This function splits the string with strSuperSplit(char *,char *,int) to avoid splitting double pipes "||".
    After all the piped commands are executed the last stdout output is printed to the console from it's file descriptor.
    The function execSpecificBuiltIn(int) is finally executed to execute specific built in commands that need to be run in the main process.
*/
int pipeSplit(char *command)
{

    int lastStatus = 0;

    //Explode the command with the pipe separator
    List *l = strSuperSplit(command, "|", 1);

    //Execute in order
    Node *node = l->first;
    while (node->next != NULL)
    {
        //Then the command is sent to a new process
        startProcess(node->data);

        node = node->next;
    }

    char reading_buf[2];
    while (read(transferFileDesc, reading_buf, 1) > 0)
    {
        write(1, reading_buf, 1); // 1 -> stdout
        reading_buf[1] = '\0';
    }

    //Finally we check for out-of-process specific requests
    lastStatus = execSpecificBuiltIn(lastStatus);

    //Last status is returned
    return lastStatus;
}

/*! 
    \brief Parses the command and fork before executing the chain of conditionals commands.
    \param Sub-command pointer char * command.
    \return Return code of the command int.

    First of all the command is sent to the superParse(char *) function to create the structure of conditional commands.
    Then the main process forks to create a sub-process where the conditinal commands will be executed in order.
    The main process will then enter a wait() function to wait for the child process to finsih.
*/
int startProcess(char *command)
{

    //We parse the command
    superParse(command);

    //We prepare the pipe for this process
    int my_pipe[2];
    if (pipe(my_pipe) == -1)
    {
        fprintf(stderr, "Error creating main pipe\n");
    }

    //We prepare the pipes for the special builtin commands
    int exit_pipe[2];
    int cd_pipe[2];
    if (pipe(exit_pipe) == -1)
    {
        fprintf(stderr, "Error creating exit pipe\n");
    }
    if (pipe(cd_pipe) == -1)
    {
        fprintf(stderr, "Error creating cd pipe\n");
    }

    //We fork
    pid_t pid1 = fork();
    int lastStatus = 0;

    if (pid1 == 0)
    {
        //slave process

        //We close the stdin of the pipes
        close(my_pipe[0]);
        close(exit_pipe[0]);
        close(cd_pipe[0]);

        //We redirect stdout to the pipe
        dup2(my_pipe[1], 1);
        //And the file descriptor to stdin
        dup2(transferFileDesc, 0);

        //Assign file descriptors for builtin commands
        exitFileDesc = exit_pipe[1];
        cdFileDesc = cd_pipe[1];

        //Execute chain of commands and exit with code
        int processExitCode = runRoot();

        exit(processExitCode);
    }
    else
    {
        //owner process

        //Close stdout of the pipes
        close(my_pipe[1]);
        close(exit_pipe[1]);
        close(cd_pipe[1]);

        //We assign the stdout as the transfer file descriptor
        transferFileDesc = my_pipe[0];

        //Then we assign the file descriptors for builtin commands
        exitFileDesc = exit_pipe[0];
        cdFileDesc = cd_pipe[0];

        //Wait for process end
        wait();
    }

    return lastStatus;
}

/*! 
    \brief Splits the command by "||" and feeds it into the conditional structure.
    \param Command pointer to be split char * command.
    \return Void.

    The root of the curent structure is first freed.
    Then the command is split using strSuperSplit(char *,char *,int) to avoid a malformed triple "&".
    Finally the tokens are injected in reverse order in the structure using the addRootCommand(char *) function.
*/
void superParse(char *command)
{

    //Free previous parsed command
    freeRoot();

    //Explode the command with the double ampersand char sequence
    List *l = strSuperSplit(command, "||", 1);

    //Parse in reverse order
    int i;
    for (i = List_length(l) - 1; i >= 0; i--)
    {
        List *l_2 = strSuperSplit(List_get(l, i), ";", 0);
        int j;
        for (j = List_length(l_2) - 1; j >= 0; j--)
        {
            addRootCommand(List_get(l_2, j));
        }
    }
}

/*! 
    \brief Tries to execute a commands as built in, otherwise sends it to the system function.
    \param Command pointer to execute char * command.
    \return Return code of the command int.

    First of all the command is trimed to remove unnecessary trailing and leading spaces.
    Then the command is sent to the execRedirection(char *) function to setup the std's for redirection.
    After that a condition checks for the need to send this process in the background for "&" background executions.
    The command is then split by spaces to compare it with built in expressions.
    Some built in write informations in specific pipes to return specific actions to run in the main process.
    If the command hasn't been processed yet it is sent to syst
*/
int execCommand(char *command)
{

    //Remove leading and trailing white spaces
    command = trimwhitespace(command);

    command = execRedirection(command);

    char *fullCommand = strdup(command);

    //Does the command needs to become a independant process
    char last_letter = command[strlen(command) - 1];
    if (last_letter == '&')
    {
        command[strlen(command) - 1] = '\0';
        printf("Started [%s] in background.\n", command);
        /* On success: The child process becomes session leader */
        if (setsid() < 0)
            exit(EXIT_FAILURE);
        /* Catch, ignore and handle signals */
        //TODO: Implement a working signal handler */
        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        /* Fork for the second time*/
        int pid = fork();
        /* An error occurred */
        if (pid < 0)
            exit(EXIT_FAILURE);
        /* Success: Let the parent terminate */
        if (pid > 0)
            return EXIT_SUCCESS;
        /* Set new file permissions */
        umask(0);
        /* Change the working directory to the root directory */
        /* or another appropriated directory */
        chdir("/");
        /* Close all open file descriptors */
        int x;
        for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
        {
            close(x);
        }
    }

    int count;
    char **splittedCommand = str_split(command, ' ', &count);

    //Builtin commands
    char *cmdCD = "cd";
    char *cmdPWD = "pwd";
    char *cmdEXIT = "exit";
    char *cmdECHO = "echo";

    if (strcmp(cmdCD, splittedCommand[0]) == 0)
    {

        //Tool vars
        int returnCode;
        const char *target;

        //If an argument is provided
        if (count >= 2)
        {

            //Tilde separator declaration
            char *cmdTilde = "~";

            //Target extraction
            target = splittedCommand[1];

            //If the argument is a tilde it's a shortcut for the homedir
            if (strcmp(cmdTilde, splittedCommand[0]) == 0)
            {
                target = getenv("HOME");
            }
        }
        else
        {
            //If there are no args we go the homedir
            target = getenv("HOME");
        }

        //Go ot target
        returnCode = chdir(target);

        //Add a secondary separator (To allow for several redirections in a row)
        target = concat("&", target);

        //Write to CD file descriptor the target
        int i;
        char write_buf[2];
        for (i = 0; target[i] != '\0'; i++)
        {
            write_buf[0] = target[i];
            write_buf[1] = '\0';
            if ((write(cdFileDesc, write_buf, 1)) < 1)
            {
                perror("Write to pipe failed.");
            }
        }

        return returnCode;
    }

    if (strcmp(cmdPWD, splittedCommand[0]) == 0)
    {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
        return 0;
    }

    if (strcmp(cmdEXIT, splittedCommand[0]) == 0)
    {

        //We prepare the exit message
        char *payload = "EXIT";

        //Write to EXIT file descriptor the need to exit
        int i;
        char write_buf[2];
        for (i = 0; payload[i] != '\0'; i++)
        {
            write_buf[0] = payload[i];
            write_buf[1] = '\0';
            if ((write(exitFileDesc, write_buf, 1)) < 1)
            {
                perror("Write to pipe failed.");
            }
        }

        //Premature exit
        exit(0);
    }

    if (strcmp(cmdECHO, splittedCommand[0]) == 0 && count >= 2)
    {
        char *resultToPrint = "";
        int i;
        for (i = 1; i < count; i++)
        {
            resultToPrint = concat(resultToPrint, splittedCommand[i]);
            if (i != (count - 1))
            {
                resultToPrint = concat(resultToPrint, " ");
            }
        }
        printf("%s\n", resultToPrint);
        return 0;
    }

    // Pass exection to handler
    return executeCmd(fullCommand);
}

/*! 
    \brief Redirects the stdout or stdin according to the command then return the part to execute
    \param Command pointer to parse char * command.
    \return Part to execute char *.

    This function support >, >>, < and << operators and redirects the stdin and out accordingly.
    The returned command is the remaining part to be executed in this configuration.
    This function is exclusively called by a child process of the main one.
*/
char *execRedirection(char *command)
{

    //Fault management
    if (command == NULL)
    {
        return "";
    }
    if (*command == '\0')
    {
        return "";
    }

    //Test for '>'
    List *l1 = strSuperSplit(command, ">", 1);
    //If there is a redirection
    if (List_length(l1) > 1)
    {
        //Extract the effective command
        command = List_get(l1, 0);
        char *outFile = List_get(l1, 1);
        redirect(outFile, O_CREAT | O_WRONLY | O_TRUNC, STDOUT_FILENO);
    }

    //Test for '>>'
    List *l2 = strSuperSplit(command, ">>", 1);
    //If there is a redirection
    if (List_length(l2) > 1)
    {
        //Extract the effective command
        command = List_get(l2, 0);
        char *outFile = List_get(l2, 1);
        redirect(outFile, O_APPEND | O_WRONLY, STDOUT_FILENO);
    }

    //Test for '<'
    List *l3 = strSuperSplit(command, "<", 1);
    //If there is a redirection
    if (List_length(l3) > 1)
    {
        //Extract the effective command
        command = List_get(l3, 0);
        char *inFile = List_get(l3, 1);
        redirect(inFile, O_RDONLY, STDIN_FILENO); // Redirect stdin
    }

    //Test for '<<'
    List *l4 = strSuperSplit(command, "<<", 1);
    //If there is a redirection
    if (List_length(l4) > 1)
    {

        //Extract the effective command
        command = List_get(l4, 0);
        char *inputTerminator = List_get(l4, 1);

        //Trim the extra whitespaces to avoid annoying behavior
        inputTerminator = trimwhitespace(inputTerminator);

        //Open a temporary file to receive the input
        char *fileInput = "temp.dat";
        FILE *inputFile = fopen(fileInput, "w");

        //Promp user
        while (1)
        {
            int max = 20;
            char *prompt = (char *)malloc(max); /* allocate buffer */

            int i = 0;
            while (1)
            {
                int c = getchar();
                if (c == 10)
                { /* at end, add terminating zero */
                    prompt[i] = 0;
                    break;
                }
                prompt[i] = c;
                if (i == max - 1)
                { /* buffer full */
                    max += max;
                    prompt = (char *)realloc(prompt, max); /* get a new and larger buffer */
                }
                i++;
            }

            //If the line just entered by the user is the terminator (Not the robot)
            if (strcmp(prompt, inputTerminator) == 0)
            {
                //We close the input file
                fclose(inputFile);
                //We redirect the input file to the stdin
                redirect(fileInput, O_RDONLY, STDIN_FILENO); // Redirect stdin
                //And we break we don't want to inject the terminator
                remove(fileInput);
                break;
            }

            //If the line looks good
            if (strlen(prompt) > 0)
            {
                //We inject it in the input file
                fprintf(inputFile, "%s\n", prompt);
            }
        }
    }

    return command;
}

/*! 
    \brief Once a command has been executed some built in need the main process to do some operations.
    \param Current return status int.
    \return New return status int.

    In the case of the CD built in command, the initial directory change occurs in the child process.
    But the change must be also applied to the parent process.
    To do this a pipe is created where the child sends a request for the parent to change it's directory.
    Here the content of the pipe is interpreted and the working directory of the main process is altered.
    But wait, the CD request might need previous context cd changes to be valid,
    so each time a CD command is processed in the child the pipe is fed the CD request.
    In this function all the CD requests are applied in the order the child did it to sync with him.
    In the case of the exit buit in command it's pretty similar (but far simpler), a specific pipe is parsed to listen for an exit command,
    if a request is in the pipe the main process will terminate here with an exit code of 0.
*/
int execSpecificBuiltIn(int status)
{

    //First we prepare buffers for the result file descriptors
    char *cd_buffer = "";
    char *exit_buffer = "";

    //Then we extract data from the file descriptors
    char reading_buf[2];
    while (read(cdFileDesc, reading_buf, 1) > 0)
    {
        reading_buf[1] = '\0';
        cd_buffer = concat(cd_buffer, reading_buf);
    }
    while (read(exitFileDesc, reading_buf, 1) > 0)
    {
        reading_buf[1] = '\0';
        exit_buffer = concat(exit_buffer, reading_buf);
    }

    //We prepare our tool values
    char *checkEXIT = "EXIT";
    char *checkEmpty = "";
    char *separator = "&";

    //Check for exit request
    if (strcmp(checkEXIT, exit_buffer) == 0)
    {
        //Before exiting we close the log file
        closeLog();
        //Then we exit with 0
        exit(0);
    }

    //If there is a cd request
    if (strcmp(checkEmpty, cd_buffer) != 0)
    {

        //Explode the cd buffer with the separator
        char *token = strtok(cd_buffer, separator);

        //We just need to get the last index
        while (token)
        {

            char *target = strdup(token);
            printf("Moving to %s\n", target);
            status = chdir(target);
            token = strtok(NULL, separator);
            free(target);
        }
    }

    return status;
}

/* Chain ops*/

/*! 
    \brief Run the conditional commands accordingly.
    \return Return code of the last command executed int.

    This function is the entry point of the recursive execution of all the conditional commands,
    the returned value is the last status code of the last command executed.
*/
int runRoot()
{
    lastStatus = 0;
    runLevel(getRootCommand());
    return lastStatus;
}

/*! 
    \brief Runs recursively levels.
    \param Command pointer Command*.
    \return Void.

    Will go recursively through all levels but will stop if the return value of the chain execution is 0.
    This is done to insure that at least one of the OR commands is executed, otherwise it stops.
*/
void runLevel(Command *command)
{
    if (command == NULL)
    {
        return;
    }
    if (runChain(command) > 0)
    {
        runLevel(command->nextCommand);
    }
}

/*! 
    \brief Runs OR commands until one succeeds.
    \param Command pointer Command*.
    \return Chain succes status int.

    Will go recursively through all command of the OR chain until one suceeds.
    If a command succeeds 1 is returned, otherwise if none, 0 is returned.
*/
int runChain(Command *command)
{

    if (command == NULL)
    {
        return 0;
    }

    int internalResult = 0;
    lastStatus = execCommand(command->command);

    if (lastStatus == 0)
    {
        return 1;
    }

    if (command->orCommand != NULL)
    {
        internalResult = runChain(command->orCommand);
    }

    return internalResult;
}

/*! 
    \brief Prints levels recursively.
    \param Command pointer Command*.
    \return Void.

    Will go recursively through all level and print their chains.
    Only used for debugging purposes.
*/
void printLevel(Command *command)
{
    if (command == NULL)
    {
        return;
    }
    printChain(command);
    if (command->nextCommand != NULL)
    {
        printLevel(command->nextCommand);
    }
}

/*! 
    \brief Prints a chain recursively.
    \param Command pointer Command*.
    \return Void.

    Will go recursively through all commands of a chain and print them.
    Only used for debugging purposes.
*/
void printChain(Command *command)
{
    if (command == NULL)
    {
        return;
    }
    if (command->orCommand != NULL)
    {
        printChain(command->orCommand);
    }
}

/*! 
    \brief Split and execute a command with its parameters 
    \param Command pointer Command*.
    \return Execution success status int.

    Splits the command and its parameters into a char* array, 
    then pass it to execvp in a child process to execute it.
    Return 1 if execution succeeds, 0 otherwise.
*/
int executeCmd(Command *cmd)
{
    int length = 0;
    char **args = malloc(sizeof(char *) * 100);
    args = str_split(cmd, ' ', &length);
    args[length + 1] = NULL;

    pid_t c_pid, pid;
    int status;

    c_pid = fork();

    if (c_pid == 0)
    {
        execvp(args[0], args);
        perror("execvp failed");
    }
    else if (c_pid > 0)
    {
        if ((pid = wait(&status)) < 0)
        {
            perror("wait");
            _exit(1);
        }
    }
    else
    {
        perror("fork failed");
        _exit(1);
    }

    free(args);

    return 0;
};
