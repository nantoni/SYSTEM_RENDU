
/*! \struct Command
    \brief Command struct .

    This Command contains a char* containing the command, a pointer of the nextCommand and a pointer of or Command . Both pointer are of Command types.
    
*/
typedef struct SCommand{
    char* command;
    struct SCommand* nextCommand;
    struct SCommand* orCommand;
}Command;
 
 
