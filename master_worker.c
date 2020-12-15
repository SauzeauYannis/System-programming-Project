#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include "myassert.h"

#include "master_worker.h"

// Bibliothéques ajoutés
#include <unistd.h>   // Pour: write, read, close, execv
#include <string.h>   // Pour: sprintf
#include <stdbool.h>  // Pour: bool


/**********************************************
            Pour les tubes anonymes
 **********************************************/


//============ MANIPULATION DE TUBES ANONYMES ============

// Fournit le côté lecture d'une pipe entrée en paramètre
int readingSidePipe(int pipe[2])
{
    int ret = close(pipe[1]);
    myassert(ret != -1, "La fermeture du côté écriture d'un tube anonyme s'est mal effectuée");

    return pipe[0];
}

// Fournit le côté écriture d'une pipe entrée en paramètre
int writingSidePipe(int pipe[2])
{
    int ret = close(pipe[0]);
    myassert(ret != -1, "La fermeture du côté lecture d'un tube anonyme s'est mal effectuée");

    return pipe[1];
}

// Ferme les file descriptor entrés en paramètre
void closeFD(int fd_prev, int fd_next, int fd_master)
{
    // Ferme les fd et teste s'ils sont bien fermés
    int ret;
    ret = close(fd_prev);
    myassert(ret != -1, "La fermeture d'un FD d'une pipe anonyme s'est mal effectuée");
    if (fd_next != NO_NEXT) // On vérifie qu'il y a bien un tube suivant avant de le libérer
    {
        ret = close(fd_next);
        myassert(ret != -1, "La fermeture d'un FD d'une pipe anonyme s'est mal effectuée");
    }
    ret = close(fd_master);
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

//-------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------

// Permet au worker d'envoyer un nombre au worker suivant
void workerToNextWorker(int fd, int number)
{
    sendData(fd, number);
}

/**********************************************
            Pour les workers
 **********************************************/


// Pour créer un worker
void createWorker(int prime_number, int previous_pipe, int master_pipe)
{
    printf("Creation du worker qui a pour charge le nombre premier %d\n", prime_number);

    // On déclare les chaines de caractéres qui vont
    // être envoyé en argument de l'exec, on sur-dimmenssionne
    // les tableaux mais sans mettre un nombre exorbitant puisque
    // le max_int en C a 10 chiffres (11 avec le moins)
    char *argv[5];   // Arguments de l'exec
    char MtoW[12];
    char WtoM[12];
    char prime[12];

    // On transforme les int en chaine de caractére qu'on met
    // dans les tabelaux qu'on vient de déclarer ci-dessus
    sprintf(prime, "%d", prime_number);
    sprintf(MtoW, "%d", previous_pipe);
    sprintf(WtoM, "%d", master_pipe);
    
    // On intialise les arguments de l'exec
    argv[0] = "worker";
    argv[1] = prime;
    argv[2] = MtoW;
    argv[3] = WtoM;
    argv[4] = NULL;

    // On fait l'exec
    execv(argv[0], argv);
    // Normalement le programme ne va pas jusque là, sinon on lève une erreur
    perror("Problème avec l'exec lors de la création d'un worker");
}