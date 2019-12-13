#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

const int limit = 70;

int receive1 = 0;
int receive2 = 0;

static int *scorePere;
static int *scoreFils;

void SIGUSR1_handler(int signum)
{
	if(signum == SIGUSR1)
	{
			receive1 = 1;
	}
}

void SIGUSR2_handler(int signum)
{
	if(signum == SIGUSR2)
	{
		receive2 = 1;
	}
}

int main(int argc, char const *argv[])
{
	scorePere = mmap(NULL,sizeof *scorePere,PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	scoreFils = mmap(NULL,sizeof *scoreFils,PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	*scorePere = 0;
	*scoreFils = 0;

	signal(SIGUSR1,SIGUSR1_handler);
	signal(SIGUSR2,SIGUSR2_handler);

	srand(time(NULL) ^ time(NULL));

	pid_t pid = fork();

	if(pid == 0){
		while(*scorePere <=13 && *scoreFils <= 13)
		{
				while(receive1 ==0){}

				if(*scoreFils == 13)
				{
					printf("Le fils gagne la partie\n");
					kill(getppid(),SIGTERM);
					exit(EXIT_SUCCESS);
				}

				int frappe = rand() % 100;
				if(frappe <= limit)
				{
          printf("Le fils loupe la balle %d, le père gagne 1 point\n", frappe);
          *scorePere = *scorePere +1;
          printf("Score actuel: PERE %d - %d FILS \n",*scorePere,*scoreFils);
				}

				printf("PONG \n");
				receive1 = 0;
				sleep(1);
				kill(getppid(),SIGUSR2);
		}
  }
  else{
    receive2 = 1;
		printf("Le père engage le match\n");

		while(*scorePere <= 13 && *scoreFils <= 13)
		{
				while(receive2 == 0){}

				if(*scorePere == 13)
				{
					printf("Le père gagne\n");
					kill(pid,SIGTERM);
					exit(EXIT_SUCCESS);
				}

				int frappe = (rand()^10) % 100;
				if(frappe <= limit)
				{
          printf("Le PERE loupe la balle %d, le FILS gagne 1 point\n", frappe);
          *scoreFils = *scoreFils +1;
          printf("Score actuel: PERE %d - %d FILS  \n",*scorePere,*scoreFils);
				}

				printf("PING\n");
				receive2 = 0;
				sleep(1);
				kill(pid,SIGUSR1);
	 }
 }

 return 0;
}
