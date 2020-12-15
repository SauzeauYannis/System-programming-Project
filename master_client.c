#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

// Bibliothéques ajoutés

#include <sys/types.h>  // Pour: ftok, semget, semctl, mkfifo, open, semop
#include <sys/ipc.h>    // Pour: ftok, semget, semctl, semop
#include <sys/sem.h>    // Pour: semget, semctl, semop
#include <sys/stat.h>   // Pour: mkfifo, open
#include <fcntl.h>      // Pour: open
#include <unistd.h>     // Pour: unlink, write, read, close


/**********************************************
            Pour les tubes nommés
 **********************************************/

//============ MANIPULATIONS DE TUBES NOMMES ============

// **** CREATION ****

// Fonction générale pour créé un tube nommé
static const char* createNamedPipe(const char* pipePathName)
{
    // Crée le tube nommé en lecture/ecriture et teste s'il est bien créé 
    int pipe = mkfifo(pipePathName, 0641);
    myassert(pipe != -1, "Un tube nommé s'est mal créé");

    // Retourne le nom du tube s'il est bien créé
    return pipePathName;
}
//-------------------------------------------------------------------------------------

// Créé le tube nommé du client vers le master et renvoie son nom 
const char* createPipeClientMaster()
{
    return createNamedPipe(NAMED_PIPE_CLIENT_MASTER);
}

// Créé le tube nommé du master vers le client et renvoie son nom 
const char* createPipeMasterClient()
{
    return createNamedPipe(NAMED_PIPE_MASTER_CLIENT);
}

// **** OUVERTURE ****

// Fonction générale pour ouvrir un tube nommé
static int openNamedPipe(const char* pipe, int flag)
{
    // Ouvre le tube nommé en lecture et teste s'il a bien été ouvert
    int fd = open(pipe, flag);
    myassert(fd != -1, "L'ouverture d'un tube nommé s'est mal exécutée");

    // Retourne le file descriptor
    return fd;
}
//-------------------------------------------------------------------------------------

// Ouverture du tube en paramètre en mode lecture
int openNamedPipeInReading(const char* pipe)
{
    return openNamedPipe(pipe, O_RDONLY);
}

// Ouverture du tube en paramètre en mode écriture
int openNamedPipeInWriting(const char* pipe)
{
    return openNamedPipe(pipe, O_WRONLY);
}

// **** FERMETURE **** 

// Fermeture du tube en paramètre
void closeNamedPipe(int fd)
{
    // Ferme le tube et teste s'il a bien été fermé
    int ret = close(fd);
    myassert(ret != -1, "La fermeture du tube a échoué");
}

// **** DESTRUCTION ****

// Détruit le tube nommé dont le nom est passé en paramètre
void destroyNamedPipe(const char* name) 
{
    // Détruit le tube et teste s'il a bien été détruit
    int destroy = unlink(name);
    myassert(destroy != 1, "La destruction de tube nomée s'est mal effectué");
}

//============ UTILISATION DE TUBES NOMMES ============

// Fonction général d'envoie d'une données par un tube nommé
static void sentData(int fd, int data)
{
    // Envoie la donnée par le tube mis en paramètre
    int ret = write(fd, &data, sizeof(int));
    myassert(ret != -1, "L'envoie d'une donnée par un tube ne s'est pas bien déroulé");
}

// Fonction général de reception d'une données par un tube nommé
static int receiveData(int fd)
{
    // Lecture de la donnée par le tube mis en paramètre
    int data;
    int ret = read(fd, &data, sizeof(int));
    myassert(ret != -1, "La lecture d'une donnée par un tube a échoué");

    // Retourne la donnée qui vient d'être lu
    return data;
}
//-------------------------------------------------------------------------------------

// **** Client->Master : Order ****

// Le client envoie l'ordre au master
void clientOrderMaster(int fd, int order)
{
    sentData(fd, order);
}

// Le master recoit l'ordre du client
int masterOrderClient(int fd)
{
    return receiveData(fd);
}

// **** Master->Client : ORDER_HOW_MANY_PRIME ****

// Le master envoie au client combien de nombre premier ont été calculés
void masterHowMany(int fd, int how_many)
{
    sentData(fd, how_many);
}

// Le client reçoit du master combien de nombre premier ont été calculés
void clientHowMany(int fd)
{
    int how_many = receiveData(fd);
    printf("Il y a %d nombres premiers calculés\n", how_many);
}

// **** Master->Client : ORDER_HIGHEST_PRIME ****

// Le master envoie au client le plus grand nombre premier qui ait été calculé
void masterHighestPrime(int fd, int highest_prime)
{
    sentData(fd, highest_prime);
}

// Le client reçoit du master le plus grand nombre premier qui ait été calculé
void clientHighestPrime(int fd)
{
    int highest_prime = receiveData(fd);
    printf("Le plus grand nombre premier calculé est %d\n", highest_prime);
}

// **** Master->Client : ORDER_STOP ****

// Le master envoie un accusé de reception au client
void masterStop(int fd, int confirm_receipt)
{
    sentData(fd, confirm_receipt);
}

// Le client reçoit du master un accusé de reception
void clientStop(int fd)
{
    int confirm_receipt = receiveData(fd);
    printf("L'accusé de reception %d de l'arrêt du master a bien été reçu\n", confirm_receipt);
}

// **** Client->Master : ORDER_COMPUTE_PRIME ****

// Le client envoie au master le nombre premier qu'il doit tester
void clientCompute(int fd, int compute_prime)
{
    sentData(fd, compute_prime);
}

// Le master recoit le nombre premier qu'il doit tester
int masterCompute(int fd)
{
    return receiveData(fd);
}

// **** Master->Client : ORDER_COMPUTE_PRIME ****

// Le master envoie au client si le nombre est premier ou pas
void masterPrime(int fd, int is_prime)
{
    sentData(fd, is_prime);
}

// Le client recoit la réponse si le nombre est premier ou pas
void clientPrime(int fd, int number_tested)
{
    int is_prime = receiveData(fd);
    if (is_prime == NUMBER_IS_PRIME)
        printf("Le nombre %d est premier\n", number_tested);
    else if (is_prime == NUMBER_NOT_PRIME)
        printf("Le nombre %d n'est pas premier\n", number_tested);
    else // Ne devrait jamais venir ici sinon bug en amont
        fprintf(stderr, "Le nombre %d n'a pas réussi à être testé\n", number_tested);
}


/**********************************************
            Pour les sémaphores
 **********************************************/

//============ MANIPULATIONS DES SEMAPHORES ============

// **** CLE ****

// Fonction générale pour génerer la clé d'un sémaphore
key_t getKeySemaphore(int semIdNum)
{
    // Génére la clé et test si il n'y a pas eu d'erreur
    key_t key = ftok(SEM_FICHIER, semIdNum);
    myassert(key != -1, "La clé d'un sémaphore s'est mal générée");

    // Retourne la clé si celle si s'est bien générée
    return key;
}
//-------------------------------------------------------------------------------------

// Renvoie la clé du sémaphore entre les clients si celle-ci s'est bien générée
key_t getKeySemaphoreClients()
{
    return getKeySemaphore(SEM_ID_CLIENTS);
}

// Renvoie la clé du sémaphore entre le master et un client si celle-ci s'est bien générée
key_t getKeySemaphoreMasterClient()
{
    return getKeySemaphore(SEM_ID_MASTER_CLIENT);
}

// **** CREATION ****

// Fonction générale pour créer et initialiser un sémaphore
int creationSemaphore(key_t key, int semValInit)
{
    // Créé le sémpahore et vérifie que tout s'est bien passé
    int semId = semget(key, 1, IPC_CREAT | IPC_EXCL | 0641);
    myassert(semId != -1, "Un sémaphore ne s'est pas créé correctement");

    // Initialise le sémaphore et vérifie qu'aucun problème ne s'est passé
    int ret = semctl(semId, 0, SETVAL, semValInit);
    myassert(ret != -1, "Le sémaphore ne s'est pas correctement initialisé");

    return semId;
}
//-------------------------------------------------------------------------------------

// Créé le sémaphore entre les clients et l'initialise à 1
int creationSemaphoreClients()
{
    return creationSemaphore(getKeySemaphoreClients(), 1);
}

// Créé le sémaphore entre le master et un client et l'initialise à 0
int creationSemaphoreMasterClient()
{
    return creationSemaphore(getKeySemaphoreMasterClient(), 0);
}

// **** RECUPERATION ****

// Fonction générale pour récupérer un sémaphore existant
int getIdSemaphore(key_t key)
{
    // Récupére le sémpahore et vérifie que tout s'est bien passé
    int semId = semget(key, 1, 0);
    myassert(semId != -1, "L'identifiant d'un sémaphore ne s'est pas récupéré correctement");

    // Retourne l'identifiant si celui si a bien été généré
    return semId;
}
//-------------------------------------------------------------------------------------

// Renvoie l'identifiant du sémaphore entre les clients qui a déjà était créé et initialisé
int getIdSemaphoreClients()
{
    return getIdSemaphore(getKeySemaphoreClients());
}

// Renvoie l'identifiant du sémaphore entre le master et un client qui a déjà était créé
int getIdSemaphoreMasterClient()
{
    return getIdSemaphore(getKeySemaphoreMasterClient());
}

// **** DESTRUCTION ****

// Détruit le sémaphore dont l'identifiant a été passé en paramètre
void detruireSemaphore(int semId)
{
    // Détruit le sémaphore et vérifie qu'aucun problème ne s'est passé
    int ret = semctl(semId, -1, IPC_RMID);
    myassert(ret != -1, "Le sémaphore ne s'est pas correctement détruit");
}

//============ UTILISATION DES SEMAPHORES ============

static void operationSemaphore(int semId, int operation)
{
    // On effectue l'opération sur le sémaphore qui a pour id le paramètre de la fonction
    struct sembuf augmente = {0, operation, 0};
    int ret = semop(semId, &augmente, 1);
    myassert(ret != -1, "L'opération sur le sémaphore s'est mal déroulé");
}
//-------------------------------------------------------------------------------------

// Augmente le sémpahore dont l'id est passé en paramètre
void augmenteSemaphore(int semId)
{
    operationSemaphore(semId, SEM_OP_INC);
}

// Diminue le sémpahore dont l'id est passé en paramètre
void diminueSemaphore(int semId)
{
    operationSemaphore(semId, SEM_OP_DEC);
}