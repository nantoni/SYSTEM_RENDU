/**
 * \file skeleton.c
 * \brief Basic parsing options skeleton.
 * \author Pierre L. <pierre1.leroy@orange.com>
 * \version 0.1
 * \date 10 septembre 2016
 *
 * Basic parsing options skeleton exemple c file.
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#include<getopt.h>

// TP1_includes
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
// TP1_includes

#define STDOUT 1
#define STDERR 2

#define MAX_PATH_LENGTH 4096


#define USAGE_SYNTAX "[OPTIONS] -i INPUT -o OUTPUT"
#define USAGE_PARAMS "OPTIONS:\n\
  -c, --copy : perform a copy of INPUT_FILE to a new OUTPUT_FILE with new permissions\n\
  -p, --print : prints the INPUT_FILE backward\n\
***\n\
  -i, --input  INPUT_FILE  : input file\n\
  -o, --output OUTPUT_FILE : output file\n\
***\n\
  -v, --verbose : enable *verbose* mode\n\
  -h, --help    : display this help\n\
"

/**
 * Procedure which displays binary usage
 * by printing on stdout all available options
 *
 * \return void
 */
void print_usage(char *bin_name) {
    dprintf(1, "USAGE: %s %s\n\n%s\n", bin_name, USAGE_SYNTAX, USAGE_PARAMS);
}


/**
 * Procedure checks if variable must be free
 * (check: ptr != NULL)
 *
 * \param void* to_free pointer to an allocated mem
 * \see man 3 free
 * \return void
 */
void free_if_needed(void *to_free) {
    if (to_free != NULL) free(to_free);
}


/**
 *
 * \see man 3 strndup
 * \see man 3 perror
 * \return
 */
char *dup_optarg_str() {
    char *str = NULL;

    if (optarg != NULL) {
        str = strndup(optarg, MAX_PATH_LENGTH);

        // Checking if ERRNO is set
        if (str == NULL)
            perror(strerror(errno));
    }

    return str;
}


/**
 * Binary options declaration
 * (must end with {0,0,0,0})
 *
 * \see man 3 getopt_long or getopt
 * \see struct option definition
 */
static struct option binary_opts[] =
        {
                {"help",    no_argument,       0, 'h'},
                {"verbose", no_argument,       0, 'v'},
                {"input",   required_argument, 0, 'i'},
                {"output",  required_argument, 0, 'o'},
                {"copy",    no_argument,       0, 'c'},
                {"print",   no_argument,       0, 'p'},
                {0,         0,                 0, 0}
        };

/**
 * Binary options string
 * (linked to optionn declaration)
 *
 * \see man 3 getopt_long or getopt
 */
const char *binary_optstr = "hvi:o:cp";
// option char that require an argument is followed by a colon ':'


// ls like
// dirent -> man 2 readdir
// stat -> man 2 stat
// passwd -> man passwd
// group -> man 5 group
// tm -> man 3 tm


// Tp_1 Methods

const char file_copy(const char *input_file, const char *output_file) {

    int fd_in;

    fd_in = open(input_file, O_RDONLY);

    if (fd_in < 0) {
        printf("Opening file : Failed\n");
        printf("Error no is : %d\n", errno);
        printf("Error description is : %s\n", strerror(errno));

        return 1;
    }

    int fd_out;

    fd_out = open(output_file, O_WRONLY | O_CREAT, S_IRWXU);

    if (fd_out < 0) {
        printf("Opening file : Failed\n");
        printf("Error no is : %d\n", errno);
        printf("Error description is : %s\n", strerror(errno));

        return 1;
    }

    ssize_t ret_read = 0;

    size_t BUFF_SIZE = 256;
    char *buf = (char *) calloc(BUFF_SIZE, sizeof(char));

    while ((ret_read = read(fd_in, buf, BUFF_SIZE))) {

        if (ret_read < 0) {
            printf("Reading file : Failed\n");
            printf("Error no is : %d\n", errno);
            printf("Error description is : %s\n", strerror(errno));

            return 1;
        }

        write(fd_out, buf, BUFF_SIZE);
    }

    return 0;
}

int check_text_file(const char *file_to_check) {
    struct stat st;
    if (stat(file_to_check, &st) == 0 && st.st_mode & S_IXUSR)
        return 0;
    return 1;
}


int get_file_size(int *file_descriptor) {

    struct stat statbuf;

    int ret = fstat(*file_descriptor, &statbuf);

    if (ret < 0) {
        printf("Getting file stats : Failed\n");
        printf("Error no is : %d\n", errno);
        printf("Error description is : %s\n", strerror(errno));

        return -1;
    }

    return statbuf.st_size;
}


const char file_print(const char *input_file) {

    int fd_in;

    fd_in = open(input_file, O_RDONLY);

    if (fd_in < 0) {
        printf("Opening file : Failed\n");
        printf("Error no is : %d\n", errno);
        printf("Error description is : %s\n", strerror(errno));

        return 1;
    }

    ssize_t ret_read = 0;

    size_t BUFF_SIZE = 1;

    char *buf = (char *) calloc(BUFF_SIZE, sizeof(char));

    int lseek_index = get_file_size(&fd_in);
    printf("File size : %d\n", lseek_index);

    while (lseek_index > 0) {

        lseek(fd_in, lseek_index - 1, SEEK_SET);

        ret_read = read(fd_in, buf, BUFF_SIZE);

        if (ret_read < 0) {
            printf("Reading file : Failed\n");
            printf("Error no is : %d\n", errno);
            printf("Error description is : %s\n", strerror(errno));

            return 1;
        }

        printf("%c", buf[0]);

        lseek_index--;
    }

    printf("\n");


    return 0;
}

// Tp_1 Methods



/**
 * Binary main loop
 *
 * \return 1 if it exit successfully 
 */
int main(int argc, char **argv) {
    /**
     * Binary variables
     * (could be defined in a structure)
     */
    short int is_verbose_mode = 0;
    short int copy_file = 0;
    short int print_file = 0;
    char *bin_input_param = NULL;
    char *bin_output_param = NULL;

    // Parsing options
    int opt = -1;
    int opt_idx = -1;

    while ((opt = getopt_long(argc, argv, binary_optstr, binary_opts, &opt_idx)) != -1) {
        switch (opt) {
            case 'c':
                copy_file = 1;
                break;
            case 'p':
                print_file = 1;
                break;
            case 'i':
                //input param
                if (optarg) {
                    bin_input_param = dup_optarg_str();
                }
                break;
            case 'o':
                //output param
                if (optarg) {
                    bin_output_param = dup_optarg_str();
                }
                break;
            case 'v':
                //verbose mode
                is_verbose_mode = 1;
                break;
            case 'h':
                print_usage(argv[0]);

                free_if_needed(bin_input_param);
                free_if_needed(bin_output_param);

                exit(EXIT_SUCCESS);
            default :
                break;
        }
    }

    /**
     * Checking binary requirements
     * (could defined in a separate function)
     */
    if (bin_input_param == NULL || (bin_output_param == NULL && print_file != 1)) {
        dprintf(STDERR, "Bad usage! See HELP [--help|-h]\n");

        // Freeing allocated data
        free_if_needed(bin_input_param);
        free_if_needed(bin_output_param);
        // Exiting with a failure ERROR CODE (== 1)
        exit(EXIT_FAILURE);
    }


    // Printing params
    dprintf(1, "** PARAMS **\n%-8s: %s\n%-8s: %s\n%-8s: %d\n",
            "input", bin_input_param,
            "output", bin_output_param,
            "verbose", is_verbose_mode);

    // Business logic must be implemented at this point

    /* LOREM IPSUM DOT SIR AMET */
    if (copy_file) {
        char ret = file_copy(bin_input_param, bin_output_param);

        if (ret > 0) {
            free_if_needed(bin_input_param);
            free_if_needed(bin_output_param);

            exit(EXIT_FAILURE);
        }
    }

    if (print_file) {

        if (check_text_file(bin_input_param) > 0) {

            printf("File type is not text\n");

            free_if_needed(bin_input_param);
            free_if_needed(bin_output_param);

            exit(EXIT_FAILURE);
        }

        char ret = file_print(bin_input_param);

        if (ret > 0) {
            free_if_needed(bin_input_param);
            free_if_needed(bin_output_param);

            exit(EXIT_FAILURE);
        }


    }


    // Freeing allocated data
    free_if_needed(bin_input_param);
    free_if_needed(bin_output_param);


    return EXIT_SUCCESS;
}
