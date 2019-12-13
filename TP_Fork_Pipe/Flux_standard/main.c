#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define STDOUT 1
#define STDERR 2

int main(int argc, char **argv)
{
    // Verifie qu'au moins 1 argument est passe en parametre
    if (argc <= 1)
    {
        dprintf(STDERR, "Il faut saisir au moins un argument\n");
        return 0;
    }
    else
    {
        dprintf(STDOUT, "Premier argument : %s\n", argv[1]);
    }

    // Creation du processus fils
    int pid = fork();

    // Verification de la creation du processus fils
    if (pid < 0)
    {
        printf("Erreur : Le fils n'a pas été cree\n");
        return 0;
    }

    // Execute dans le processus fils
    if (pid == 0)
    {
        // Création fichier temporaire
        char template[] = "./tmp/proc-exercise-XXXXXX";
        int fd = mkstemp(template);

        // Verification de la creation du fichier temporaire
        if (fd < 0)
        {
            dprintf(STDERR, "Erreur : Le fichier temporaire n'a pas ete cree \n");
            return EXIT_FAILURE;
        }

        printf("Processus fils - PID : %d \n", getpid());

        // Redirection de la sortie standard vers le fichier temporaire
        dup2(fd, STDOUT);
        printf("Id du descripteur du fichier temporaire créé:  %d \n", fd);

        // Fermeture du fichier temporaire
        close(fd);

        //Execution du programme avec l'argument passé en commande
        if (execlp(argv[1], argv[1], (char *)NULL) < 0)
        {
            perror("Erreur exec : ");
        }

        exit(1);
    }
    //Processus parents
    else
    {
        printf("Processus pere - PID : %d \n", getpid());

        //Attente de la fin du processus fils (execution du programme)
        int wstatus;
        wait(&wstatus);
        if (WIFEXITED(wstatus))
        {
            printf("Processus pere : That's all folks !!!\n");
        }
    }
}
