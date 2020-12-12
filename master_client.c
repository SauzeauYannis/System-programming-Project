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
#include <unistd.h>     // Pour: unlink, write, read


/**********************************************
            Pour les tubes nommés
 **********************************************/

//============ MANIPULATIONS DE TUBES NOMMES ============

// **** CREATION ****

// Fonction générale pour créé un tube nommé
const char* createNamedPipe(const char* pipePathName)
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
    const char* pipePathName = createNamedPipe(NAMED_PIPE_CLIENT_MASTER);
    printf("Debug : Tube nommé Client->Master créé avec %s comme nom\n", pipePathName);
    return pipePathName;
}

// Créé le tube nommé du master vers le client et renvoie son nom 
const char* createPipeMasterClient()
{
    const char* pipePathName = createNamedPipe(NAMED_PIPE_MASTER_CLIENT);
    printf("Debug : Tube nommé Master->Client créé avec %s comme nom\n", pipePathName);
    return pipePathName;
}

// **** OUVERTURE ****

// Ouverture du tube en paramètre en mode lecture
int openNamedPipeInReading(const char* pipe)
{
    // Ouvre le tube nommé en lecture et teste s'il a bien été ouvert
    int fd = open(pipe, O_RDONLY);
    myassert(fd != -1, "L'ouverture du tube en lecture s'est mal exécutée");

    // Retourne le file descriptor
    printf("Debug : Tube nommé %s ouvert en lecture avec %d pour fd\n", pipe, fd);
    return fd;
}
//-------------------------------------------------------------------------------------

// Ouverture du tube en paramètre en mode écriture
int openNamedPipeInWriting(const char* pipe)
{
    // Ouvre le tube nommé en écriture et teste s'il a bien été ouvert
    int fd = open(pipe, O_WRONLY);
    myassert(fd != -1, "L'ouverture du tube en écriture s'est mal exécutée");

    // Retourne le file descriptor
    printf("Debug : Tube nommé %s ouvert en écriture avec %d pour fd\n", pipe, fd);
    return fd;
}

// **** FERMETURE **** 

// Fermeture du tube en paramètre
void closeNamedPipe(int fd)
{
    // Ferme le tube et teste s'il a bien été fermé
    int ret = close(fd);
    myassert(ret != -1, "La fermeture du tube a échoué");

    printf("Debug : Fermeture du Tube nommé qui a pour %d pour fd\n", fd);
}

// **** DESTRUCTION ****

// Détruit le tube nommé dont le nom est passé en paramètre
void destroyNamedPipe(const char* name) 
{
    // Détruit le tube et teste s'il a bien été détruit
    int destroy = unlink(name);
    myassert(destroy != 1, "La destruction de tube nomée s'est mal effectué");

    printf("Debug : Destruction du tube nommé : %s\n", name);
}

//============ UTILISATION DE TUBES NOMMES ============

// Fonction général d'envoie d'une données par un tube nommé
void sentData(int fd, int data)
{
    // Envoie la donnée par le tube mis en paramètre
    int ret = write(fd, &data, sizeof(int));
    myassert(ret != -1, "L'envoie d'une donnée par un tube ne s'est pas bien déroulé");
}

// Fonction général de reception d'une données par un tube nommé
int receiveData(int fd)
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
    printf("Debug : Envoie de l'ordre %d au master\n", order);
}

// Le master recoit l'ordre du client
int masterOrderClient(int fd)
{
    int order = receiveData(fd);

    printf("Debug : Le master vient de recevoir l'odre %d du client\n", order);
    return order;
}

// **** Master->Client : ORDER_HOW_MANY_PRIME ****

// Le master envoie au client combien de nombre premier ont été calculés
void masterHowMany(int fd, int how_many)
{
    sentData(fd, how_many);
    printf("Debug : Envoie du nombre de caclul effectuée %d au client\n", how_many);
}
//-------------------------------------------------------------------------------------

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
    printf("Debug : Envoie du plus grand nombre premier %d au client\n", highest_prime);
}
//-------------------------------------------------------------------------------------

// Le client reçoit du master le plus grand nombre premier qui ait été calculé
void clientHighestPrime(int fd)
{
    int highest_prime = receiveData(fd);
    printf("Le plus grand nombre premier calculé est %d\n", highest_prime);
}

// **** RETOUR : ORDER_COMPUTE_PRIME ****


// Le client envoie le nombre premier a traité au master
// void clientSendsPrimeToMaster(int fd, int prime)
// {
//     // TODO : faire cette fonction pour quand l'odre
//     // est égale à ORDER_COMPUTE_PRIME et qu'il faut
//     // envoyé le nombre premier
// }
//-------------------------------------------------------------------------------------

// Le master recoit le nombre premier du client
// int masterReceivePrimeToClient(int fd)
// {
//     // TODO : faire cette fonction pour quand l'odre
//     // est égale à ORDER_COMPUTE_PRIME et qu'il faut
//     // recevoir le nombre premier
// }



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
    key_t key = getKeySemaphore(SEM_ID_CLIENTS);
    printf("Debug : Clé %d du sémaphore entre les clients\n", key);
    return key;
}

// Renvoie la clé du sémaphore entre le master et un client si celle-ci s'est bien générée
key_t getKeySemaphoreMasterClient()
{
    key_t key = getKeySemaphore(SEM_ID_MASTER_CLIENT);
    printf("Debug : Clé %d du sémaphore entre le master et un client générée\n", key);
    return key;
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

// Renvoie l'identifiant du sémaphore entre les clients qui vient d'être créé et initialisé
int creationSemaphoreClients()
{
    int semId = creationSemaphore(getKeySemaphoreClients(), 1);
    printf("Debug : Création et initialisation du sémaphore entre les clients d'id %d\n", semId);
    return semId;
}

// Renvoie l'identifiant du sémaphore entre le master et un client qui vient d'être créé et initialisé
int creationSemaphoreMasterClient()
{
    int semId = creationSemaphore(getKeySemaphoreMasterClient(), 0);
    printf("Debug : Création et initialisation du sémaphore entre le master et un client d'id %d\n", semId);
    return semId;
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
    int semId = getIdSemaphore(getKeySemaphoreClients());
    printf("Debug : Récupération du sémaphore entre les clients d'id %d\n", semId);
    return semId;
}

// Renvoie l'identifiant du sémaphore entre le master et un client qui a déjà était créé
int getIdSemaphoreMasterClient()
{
    int semId = getIdSemaphore(getKeySemaphoreMasterClient());
    printf("Debug : Récupération du sémaphore entre le master et un client d'id %d\n", semId);
    return semId;
}

// **** DESTRUCTION ****

// Détruit le sémaphore dont l'identifiant a été passé en paramètre
void detruireSemaphore(int semId)
{
    // Détruit le sémaphore et vérifie qu'aucun problème ne s'est passé
    int ret = semctl(semId, -1, IPC_RMID);
    myassert(ret != -1, "Le sémaphore ne s'est pas correctement détruit");
    printf("Debug : Destruction du Sem qui a pour id : %d\n", semId);
}

//============ UTILISATION DES SEMAPHORES ============

void operationSemaphore(int semId, int operation)
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
    printf("Debug : Augmentation du Sem qui a pour id : %d\n", semId);
}

// Diminue le sémpahore dont l'id est passé en paramètre
void diminueSemaphore(int semId)
{
    operationSemaphore(semId, SEM_OP_DEC);
    printf("Debug : Diminution du Sem qui a pour id : %d\n", semId);
}