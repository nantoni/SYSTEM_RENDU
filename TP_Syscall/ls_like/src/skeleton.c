/**
 * \file skeleton.c
 * \brief Basic parsing options skeleton.
 * \author Pierre L. <pierre1.leroy@orange.com>
 * \version 0.1
 * \date 10 septembre 2016
 *
 * Basic parsing options skeleton exemple c file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <getopt.h>

// TP1_includes
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//ls_like
#include<dirent.h>
#include<grp.h>
#include<pwd.h>
#include<time.h>
// TP1_includes


#define STDOUT 1
#define STDERR 2

#define MAX_PATH_LENGTH 4096

/**
 * Binary main loop
 *
 * \return 1 if it exit successfully 
 */
int main(int argc, char **argv)
{

  char *bin_input_param = NULL;

  bin_input_param = argv[1];

  if (bin_input_param == NULL)
  {
    dprintf(STDERR, "Bad usage! See HELP [--help|-h]\n");

    // Freeing allocated data
    exit(EXIT_FAILURE);
  }

  // Printing params
  dprintf(1, "** PARAMS **\n%-8s: %s\n",
          "input", bin_input_param);

  // Business logic must be implemented at this point

  DIR* directory = NULL; 
	struct dirent* entry;
	struct stat fileStat;
	struct passwd *pwd;
	struct group *grp;
	struct tm * infotime;
	char buffer[80];	
  
	//ouverture descripteur du répertoire
	directory = opendir(bin_input_param);
	   
	if(directory == NULL) {
		perror(strerror(errno));
		exit(1);
	}

	//itération lecture d'un répertoire 
	while ((entry = readdir(directory)) != NULL){
		if (!strcmp (entry->d_name, "."))
			continue;
        if (!strcmp (entry->d_name, ".."))    
            continue;     
      
		if (!stat(bin_input_param, &fileStat)){
			printf(" %s - ", entry->d_name);
			printf((S_ISDIR(fileStat.st_mode))  ? "d" : "-");
			printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
			printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
			printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
			printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
			printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
			printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
			printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
			printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
			printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
				
			if ((pwd = getpwuid(fileStat.st_uid)) != NULL)
				  printf(" %-8.8s", pwd->pw_name);

			if ((grp = getgrgid(fileStat.st_gid)) != NULL)
				  printf(":%-8.8s", grp->gr_name);
			
			printf("- %d - ", (int) fileStat.st_size);
			infotime = localtime(&fileStat.st_mtime);       
			strftime(buffer,80,"%d/%m/%Y-%I:%M%p", infotime);
			printf("%s ", buffer );         
			printf(" \n");
		} else{
			perror(strerror(errno));
		}
	}
 	
 //ferme le descripteur
	if (closedir(directory) == -1) /* S'il y a eu un souci avec la fermeture */
		exit(-1);
    
  return EXIT_SUCCESS;
}
