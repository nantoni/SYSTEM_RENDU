#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/time.h>
#include<unistd.h>
#include<sys/wait.h>
#include<time.h>
#include<pthread.h>
#include<getopt.h>


#define SIZE (int)1e8
#define RANDOM_MAX (int) 1e9

// Structure des resultats des tests
typedef struct rResultat {
    int min;
    int max;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} Result;


// contient les indexes des sous-tableaux decoupes
typedef struct rIndiceTab{
    int start;
    int end;
}IndexArray;


static Result results = {
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

int array_num[SIZE];
struct timeval begin, end;

// Remplit le tableau global de SIZE valeurs aleatoires entre 0 et RANDOM_MAX
void fillNumbersArray(){
	srand(time(NULL));
	int i;
    for( i=0;i < SIZE; i++) {
        array_num[i] = rand() % RANDOM_MAX;
    }
}

// Affiche le contenu du tableau global
void printArray(){
	int i;
	for(i=0; i<SIZE; i++){
		printf("indice %d : %d \n",i,array_num[i]);
	}
}

// Trouve la valeur minimum du tableau global
void * searchMinTab(void* args){
	IndexArray * indexArray = (IndexArray *) args;

  // Recuperation du mutex
	pthread_mutex_lock(&results.mutex);

  // Cherche la valeur MIN
	for(int i=indexArray->start;i<indexArray->end;i++) {
        if(results.min > array_num[i]) {
            results.min = array_num[i];
        }
    }

  // Liberation du mutex
  pthread_mutex_unlock(&results.mutex);

	return NULL;
}

// Trouve la valeur maximum du tableau global
void * searchMaxTab(void* args){
	IndexArray * indexArray = (IndexArray *) args;

  // Recuperation du mutex
	pthread_mutex_lock(&results.mutex);

  // Cherche la valeur MAX
	for(int i=indexArray->start;i<indexArray->end;i++) {
        if(results.max < array_num[i]) {
            results.max = array_num[i];
        }
    }

  // Liberation du mutex
	pthread_mutex_unlock(&results.mutex);

	return NULL;
}

// Affiche les valeurs des resultats
void printResult(Result result){
	printf("Min = %d , Max = %d \n",result.min, result.max);
}

// Decoupe le tableau global en sous tableaux en fonction du nombre de threads
IndexArray * splitArray(int numberOfThread){
	int sizeSubArray = SIZE / numberOfThread;
	int rest = SIZE % numberOfThread;
	IndexArray * indexArray = malloc(sizeof(indexArray)*numberOfThread);

	for(int i = 0; i < numberOfThread; i++){
		indexArray[i].start = i*sizeSubArray;
		indexArray[i].end = indexArray[i].start+sizeSubArray;
		if(i == numberOfThread - 1 && rest > 0) {
            indexArray[i].end += rest;
        }
	}

  // Retourne les indexes de debut et de fin
  // Les sous tableaux sont seulement des délimitations du tableau global, ils ne sont pas crees
	return indexArray;
}

// Creer le nombre de threads voulu (defini dans le Main)
int createThreads(int numberOfThread, void *(fct) (void *)) {
    pthread_t threads[numberOfThread];
    IndexArray * indexArray = splitArray(numberOfThread);

    gettimeofday (&begin, NULL);
    for(int i = 0; i < numberOfThread; i++) {
        if (pthread_create(&threads[i], NULL, fct, (void *) &indexArray[i])) {
            return EXIT_FAILURE;
        }
    }

    for(int i = 0; i < numberOfThread; i++) {
        if(pthread_join(threads[i], NULL)) {
            return EXIT_FAILURE;
        }
    }

    gettimeofday (&end, NULL);
    printf("Temps de recherche avec %d threads : %fs\n", numberOfThread, (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec)/1000000.0));
    return 0;
}

/**
 *
 * Does the whole creating initialing threads
 * param1 : numberOfThread int
 * return : int.
 * */
void treatThreads(int numberOfThread, void *(fct) (void *)) {

    results.min = RANDOM_MAX;
    results.max = 0;

    if(numberOfThread > 0) {
        // Creation des threads
        createThreads(numberOfThread, fct);
    } else {  // Ou recherche sans threads

        IndexArray indexArray;
        indexArray.start = 0;
        indexArray.end = SIZE;

        // On regarde l'heure avant et apres la recherche dans le tableaux
        // Puis on les soustrait pour connaitre le temps de la recherche
        gettimeofday (&begin, NULL);
        searchMinTab((void *) &indexArray);
        gettimeofday (&end, NULL);
        printf("Temps de recherche du MIN avec %d threads : %fs\n", numberOfThread, (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec)/1000000.0));

        // On regarde l'heure avant et apres la recherche dans le tableaux
        // Puis on les soustrait pour connaitre le temps de la recherche
        gettimeofday (&begin, NULL);
        searchMaxTab((void *) &indexArray);
        gettimeofday (&end, NULL);
        printf("Temps de recherche du MAX avec %d threads): %fs\n", numberOfThread, (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec)/1000000.0));
    }
}

int main(int argc, char *argv[])
{
    fillNumbersArray();

	  printf("Recherche sans threads \n");
    treatThreads(0, NULL);
    printf("\n--------------------\n\n");
    treatThreads(2, searchMinTab);
    treatThreads(2, searchMaxTab);
    printf("\n--------------------\n\n");
    treatThreads(4, searchMinTab);
    treatThreads(4, searchMaxTab);
    printf("\n--------------------\n\n");
    treatThreads(8, searchMinTab);
    treatThreads(8, searchMaxTab);

    return 0;
}
