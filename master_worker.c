#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include "myassert.h"

#include "master_worker.h"

// Bibliothéques ajoutés
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>


//static void

/**********************************************
            Pour les tubes anonymes
 **********************************************/

//============ MANIPULATION DE TUBES ANONYMES ============

// Fournit le côté lecture d'une pipe entrée en paramètre
int readingSidePipe(int pipe[2])
{
    // Ferme le côté écriture et teste s'il est bien fermé (POSSIBLE SLMNT DS UN FORK)
    //int ret = close(pipe[1]);
    //myassert(ret != -1, "La fermeture du coté écriture d'une pipe anonyme s'est mal effectuée");

    return pipe[0];
}

// Fournit le côté écriture d'une pipe entrée en paramètre
int writingSidePipe(int pipe[2])
{
    // Ferme le côté lecture et teste s'il est bien fermé (POSSIBLE SLMNT DS UN FORK)
    //int ret = close(pipe[0]);
    //myassert(ret != -1, "La fermeture du coté lecture d'une pipe anonyme s'est mal effectuée");

    return pipe[1];
}

// Ferme les file descriptor entrés en paramètre
void closeFD( int fd1, int fd2)
{
    // Ferme les fd et teste s'ils sont bien fermés
    int ret;
    ret = close(fd1);
    myassert(ret != -1, "La fermeture d'un FD d'une pipe anonyme s'est mal effectuée");
    ret = close(fd2);
    myassert(ret != -1, "La fermeture d'un FD d'une pipe anonyme s'est mal effectuée");
}

//============ UTILISATION DE TUBES ANONYMES ============

// Fonction général d'envoie d'une donnée par un tube anonyme
static void sendData(int fd, int data)
{
    // Envoie la donnée par le tube entré en paramètre
    int ret = write(fd, &data, sizeof(int));
    myassert(ret != -1, "L'écriture d'une donnée dans un tube ne s'est pas bien déroulé");
}

// Fonction général de reception d'une donnée par un tube anonyme
static int receiveData(int fd)
{
    // Lecture de la donnée par le tube entré en paramètre
    int data;
    int ret = read(fd, &data, sizeof(int));
    myassert(ret != -1, "La lecture d'une donnée par un tube a échoué");

    // Retourne la donnée qui vient d'être lu
    return data;
}
//-------------------------------------------------------------------------------------

// Permet au master d'envoyer un nombre au premier worker
void masterNumberToCompute(int fd, int number)
{
    sendData(fd, number);
}

// Permet au worker de recevoir le nombre à traiter
int workerNumberToCompute(int fd)
{
    return receiveData(fd);
}

// Permet au worker de dire au master que le nombre envoyé est un nombre premier
void workerIsPrime(int fd, int is_prime)
{
    sendData(fd, is_prime);
}

// Permet au master de savoir si le nombre envoyé au(x) worker(s) est un nombre premier
int masterIsPrime(int fd)
{
    return receiveData(fd);
}




/**********************************************
            Pour les workers
 **********************************************/


// Pour créer le worker avec la nombre premier 2 
void createWorker_2(int prime_2, int pipe_MtoW_RD, int pipe_WtoM_WR)
{
    printf("Creation du worker_2\n");

    char *argv[5];
    char MtoW[1000];
    char WtoM[1000];
    char prime[1000];

    sprintf(prime, "%d", prime_2);
    sprintf(MtoW, "%d", pipe_MtoW_RD);
    sprintf(WtoM, "%d", pipe_WtoM_WR);
    
    argv[0] = "worker";
    argv[1] = prime;
    argv[2] = MtoW;
    argv[3] = WtoM;
    argv[4] = NULL;

    execv(argv[0],argv);
    perror("Problème avec l'exec de la création du worker_2");
}