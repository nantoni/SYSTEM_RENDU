#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define STDIN 0
#define STDOUT 1

int execPsGrep()
{
    // Creation et verification de la creation du pipe
    int pfd[2];
    if (pipe(pfd) < 0)
    {
        printf("Erreur : Le Pipe n'a pas ete cree\n");
        return 0;
    }

    // Creation du processus fils
    int pid = fork();

    // Verification de la creation du processus fils
    if (pid < 0)
    {
        printf("Erreur : Le fils n'a pas ete cree\n");
        return 0;
    }

    // Execution dans le processus fils
    if (pid == 0)
    {
        // Fermeture sortie pipe fils donc entree pipe pere
        close(pfd[0]);

        // Connection de la sortie standard std_out du processus fils avec l'entree du pipe fils
        dup2(pfd[1], STDOUT);

        execlp("ps", "ps", "eaux", NULL);
        printf("Erreur exec ps eaux ");

        // Fermeture de l'entree pipe fils
        close(pfd[1]);
        return 0;
    }
    // Execution dans le processus pere
    else
    {
        // Fermeture sortie pipe pere donc entree pipe fils
        close(pfd[1]);

        // Ouverture du fichier
        int filedesc = open("/dev/null", O_WRONLY | O_APPEND);
        if (filedesc < 0)
        {
            printf("Erreur : Fichier filedesc nest pas créé \n");
            return 0;
        }

        // Connection de l'entree standard std_in du processus pere avec l'entree du pipe pere
        dup2(pfd[0], STDIN);

        // Redirection sortie standard std_out du processus pere vers le fichier ouvert au dessus
        dup2(filedesc, STDOUT);
        execlp("grep", "grep", "^root", (char *)0);
        printf("grep failed");

        // Fermeture de l'entree du pipe pere
        close(pfd[0]);
        return 0;
    }
}

int main(int argc, char **argv)
{
    // Creation du processus fils
    int resFork = fork();

    // Verification de la cration du processus fils
    if (resFork < 0){
      printf("Erreur : Le fils n'a pas été créé\n");
      return 0;
    }

    // Execution dans le processus fils
    if (resFork == 0)
    {
        printf("Processus fils : \n PID %d \n", getpid());
        execPsGrep();
        return EXIT_FAILURE;

    }
    // Execution dans le processus pere
    else
    {
        printf("Processus pere : \n PID %d \n", getpid());

        // Attente de la fin du processus fils
        int wstatus;
        wait(&wstatus);
        if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0)
        {
            write(1, "root est connecte\n", 18);
        }
    }
    return 0;
}
