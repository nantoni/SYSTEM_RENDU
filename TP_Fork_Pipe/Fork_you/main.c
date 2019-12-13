#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    // Creation du processus fils
    pid_t pid = fork();

    // Verification de la creation du processus fils
    if (pid < 0)
    {
        printf("Erreur : Le fils n'a pas été cree\n");
        return 0;
    }

    // Execution dans le processus fils
    if(pid == 0){
        printf("Processus fils - [PID PERE : %d]\n", getppid());
        printf("Processus fils - [PID : %d]\n", getpid());

        // Retourne le dernier chiffre du PID
        exit(getpid() % 10);
    }
    // Execution dans le processus pere
    else{
      printf("Processus pere - [PID FILS : %d]\n", pid);
      // Attente de la fin du processus fils
      int wstatus;
      wait(&wstatus);
      if(WIFEXITED(wstatus)){
          printf("Code retour fils : %d\n", WEXITSTATUS(wstatus) );
      }else{
          printf("Le fils ne s'est pas terminé normalement");
      }
    }
    return 0;
}
