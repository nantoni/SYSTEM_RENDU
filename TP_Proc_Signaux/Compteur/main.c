#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static int loop = 1;
static int count = 0;

void handlerSIGINT(){
	printf("Réception d'un signal SIGINT - compteur : %d\n", count);
	count++;
}

void handlerSIGTERM(){
	printf("Réception d'un signal SIGTERM, fin du programme !\n");
	loop = 0;
}

int main ()
{
    printf("Le programme tourne\n");
    printf("  - CTRL + C pour incrémenter le compteur\n");
    printf("  - kill -15 $PID depuis un autre shell pour terminer le programme\n\n");
    //Réception des signaux
    signal(SIGINT, handlerSIGINT);
    signal(SIGTERM, handlerSIGTERM);

    while (loop == 1){}

    return 0;
}
