
/*! \file  
    \brief tools.c, tool functions for the main.

    This c file provides the main.c with usefull char pointer tools to split or concatenate for example.
*/

#include "tools.h"

/*! 
    \brief Tool function to concatenate two char pointers.
    \param The first char * str1.
    \param The second char * str2.
    \return A char pointer made of the two parameters concatenated char *.

    It concatenates two char pointers and returns a new one made of the two parameters.
*/
char* concat(const char* str1, const char* str2){
    char* result;
    asprintf(&result, "%s%s", str1, str2);
    return result;
}

/*! 
    \brief Splits a char pointer by a delimiter.
    \param Char pointer to split char * a_str.
    \param Delimiter char a_delim.
    \param Length of the original char pointer int * length.
    \return Char double pointer.

    Is used to split by a simple character.
*/
char** str_split(char* a_str, const char a_delim,int* length){
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;


    /* Count how many elements will be extracted. */
    while (*tmp)
    {

        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }



    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);



    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        
    }


    
    *length = count - 1;

    return result;
}

/*! 
    \brief Splits a char pointer by a multi character delimiter.
    \param Char pointer to split char * input.
    \param Delimiter pointer char * delimiter.
    \return Token char pointer.

    Is used to split by a multi character delimiter.
*/
char *multi_tok(char *input, char *delimiter){
    static char *string;
    if (input != NULL)
        string = input;

    if (string == NULL)
        return string;

    char *end = strstr(string, delimiter);
    if (end == NULL) {
        char *temp = string;
        string = NULL;
        return temp;
    }

    char *temp = string;

    *end = '\0';
    string = end + strlen(delimiter);
    return temp;
}

/*! 
    \brief Splits a char pointer by a defined delimiter, will return a List of strings.
    \param Char pointer to split char * input.
    \param Delimiter pointer char * delimiter.
    \param Option for the parser int.
    \return String list from the linkedstring library List *.

    This function allows for strict match split.
    This means that for example if the delimiter is "&" and it encounters "&&" the split won't be done.
    An other thing is that this function return directly a linked list of strings, simplifying the usual token management.
*/
List *strSuperSplit(char *input,char *delimiter,int option){
    
    //Return list of results
    List *returnList = List_create();
    
    //Invalid cases
    if(input == NULL || *input == '\0'){
        return NULL;
    }
    if(delimiter == NULL || *delimiter == '\0'){
        List_append(returnList, input);
        return returnList;
    }
    
    //We need to keep track of the index of the scope for token splitting
    int tokenStartIndex = 0;
    //We will also need the total size of the string
    int totalLength = 0;
    
    //First we need to match the first char of the string
    int i;
    for(i = 0; input[i] != '\0'; i++){
        //If it matches
        if(input[i] == delimiter[0]){
            int isValidSplit = 0; //Default false
            int targetNewStartIndex = 0;
            int j; //To parse dele
            //Now we need to exactly match all the chars of the delimiter
            for(j = 0; delimiter[j] != '\0'; j++){
                //We extract the input counterpart
                char reference = input[i + j];
                //If counterpart exists
                if(reference != '\0'){
                    //If the char matches
                    if(reference == delimiter[j]){
                        //If we are at the last index of the delimiter
                        if(delimiter[j+1] == '\0'){
                            isValidSplit = 1;
                            //Option 1 modifer (No last char of delimiter around the split)
                            if(option == 1){
                                char before = input[i-1];
                                char after = input[i+j+1];
                                //Invalidation test
                                if(delimiter[j] == before || delimiter[j] == after){
                                    isValidSplit = 0;
                                }
                            }
                            //Extra validation for last minute invalidation
                            if(isValidSplit == 1){
                                targetNewStartIndex = i+j+1;
                            }
                        }
                    }else{
                        //We cancel the search
                        break;
                    }
                }
            }
            //If the split point is valid
            if(isValidSplit == 1){
                //For that we first prepare a char array to store the token
                int allocSize = i+1;
                char *token = malloc( sizeof(char) * ( allocSize + 1 ) );
                //Target in the token to fill with the content
                int tokenTarget = 0;
                //We copy the token from the og string to the token
                for(j = tokenStartIndex; j < i; j++){
                    token[tokenTarget] = input[j];
                    tokenTarget++;
                }
                //Close properly the char pointer 
                token[tokenTarget] = '\0';
                //We add the string so far to the list
                List_append(returnList, token);
                //Finally we need to change the tokenStartIndex to the first char after the last match
                tokenStartIndex = targetNewStartIndex;
            }
        }
        totalLength++;  
    }
    //Once the chain has been parsed we just need to push the remainder
    //For that we first prepare a char array to store the token
    int allocSize = totalLength - tokenStartIndex;
    char *token = malloc( sizeof(char) * ( allocSize + 1 ) );
    //Target in the token to fill with the content
    int tokenTarget = 0;
    //We copy the token from the og string to the token
    for(i = tokenStartIndex; i < totalLength; i++){
        token[tokenTarget] = input[i];
        tokenTarget++;
    }
    //Close properly the char pointer 
    token[tokenTarget]='\0';
    //We add the string to the list
    List_append(returnList, token);
    
    //Return the list containing the splitted string

    return returnList;
}

/*! 
    \brief Replaces a specific charcter in a char pointer.
    \param Char pointer to process char * input.
    \param Char to change char target.
    \param Char to replace char replacer.
    \return The same pointer but with the target changed to the replacer char *.

    This function allow for strict charcter change.
    That means that if the target is "#" and the parser encounters "##" the charcters will be ignored.
    The returned pointer is the same as the input but with the character replaced.
*/
char *replaceSingleChar(char *input,char target,char replacer){
    
    char *parsingPointer = input;
    
    //For each char of the pointer
    while(*parsingPointer != '\0'){
        //If current char is the target
        if(*parsingPointer == target){
            //We need to check if the char is alone
            int isAlone = 1;
            //Checking forward neighbor
            char *forwardNeighbor = parsingPointer;
            forwardNeighbor++;
            if(*forwardNeighbor != '\0'){
                if(*forwardNeighbor == target){
                    isAlone = 0;
                }
            }
            //Checking backward neighbor
            char *backwardNeighbor = parsingPointer;
            backwardNeighbor--;
            if(*backwardNeighbor != '\0'){
                if(*backwardNeighbor == target){
                    isAlone = 0;
                }
            }
            //If is alone
            if(isAlone == 1){
                //We replace
                *parsingPointer = replacer;
            }
        }
        //Next char
        parsingPointer++;
    }
    
}

/*! 
    \brief Remove leading and trailing spaces.
    \param Input pointer char * str.
    \return Char pointer without leading and trailing spaces char *.

    This function remove both leading and trailing white spaces.
    The spaces between word inside the pointer are kept.
*/
char *trimwhitespace(char *str){
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

/*! 
    \brief Redirects a std to or from a file
    \param Char pointer of the file name char * filename.
    \param Flags for the file opening int flags.
    \param Std code to redirect int destfd.
    \return File descriptor of the opened file.

    This function redirects a std to or from a file.
    It is usefull for flux redirection.
    The returned integer is the file descriptor of the opened file.
*/
int redirect(char *filename,int flags, int destfd){
    int fd;
    fd=open(filename,flags,S_IRUSR|S_IWUSR);
    if(fd!=-1){
        if(dup2(fd,destfd)==-1){
            close(fd);
            fd=-1;
        }else{
            close(fd);
        }
    }
    return fd;
}

/*! 
    \brief Cross platform sleep function
    \param Time in miliseconds to wait int milliseconds.
    \return Void.

    This function will make the current thread sleep for a given amount of miliseconds.
*/
void sleep_ms(int milliseconds){
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}