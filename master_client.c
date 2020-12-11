#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

// include ajoutés

#include <sys/types.h>  // Pour: ftok, semget, semctl, mkfifo, open
#include <sys/ipc.h>    // Pour: ftok, semget, semctl
#include <sys/sem.h>    // Pour: semget, semctl
#include <sys/stat.h>   // Pour: mkfifo, open
#include <fcntl.h>      // Pour: open
#include <unistd.h>     // Pour: unlink, write, read


// Fonctions éventuelles proposées dans le .h

/***********************
 * Pour les tubes nommés
 ***********************/

// Créé le tube nommé du client vers le master et renvoie son nom 
const char* createPipeClientMaster()
{
    // Crée le tube nommé en lecture/ecriture et teste s'il est bien créé 
    int pipe = mkfifo(NAME_PIPE_CLIENT_MASTER, 0641);
    myassert(pipe != -1, "Le tube nommé du client vers le master s'est mal créé");

    // Retourne le nom du tube s'il est bien créé
    printf("Debug : Tube nommé Client->Master créé\n");
    return NAME_PIPE_CLIENT_MASTER;
}

// Créé le tube nommé du master vers le client et renvoie son nom 
const char* createPipeMasterClient()
{
    // Crée le tube nommé en lecture/ecriture et teste s'il est bien créé 
    int pipe = mkfifo(NAME_PIPE_MASTER_CLIENT, 0641);
    myassert(pipe != -1, "Le tube nommé du master vers le client s'est mal créé");

    // Retourne le nom du tube s'il est bien créé
    printf("Debug : Tube nommé Master->Client créé\n");
    return NAME_PIPE_MASTER_CLIENT;
}

// Ouverture du tube en paramètre en mode lecture
int openPipeInReading(const char* pipe)
{
    // Ouvre le tube nommé en lecture et teste s'il a bien été ouvert
    int fd = open(pipe, O_RDONLY);
    myassert(fd != -1, "L'ouverture du tube en lecture s'est mal exécutée");

    // Retourne le file descriptor
    printf("Debug : Tube nommé %s ouvert en lecture avec %d pour fd\n", pipe, fd);
    return fd;
}

// Ouverture du tube en paramètre en mode écriture
int openPipeInWriting(const char* pipe)
{
    // Ouvre le tube nommé en écriture et teste s'il a bien été ouvert
    int fd = open(pipe, O_WRONLY);
    myassert(fd != -1, "L'ouverture du tube en écriture s'est mal exécutée");

    // Retourne le file descriptor
    printf("Debug : Tube nommé %s ouvert en écriture avec %d pour fd\n", pipe, fd);
    return fd;
}

// Fermeture du tube en paramètre
void closePipe(int fd)
{
    // Ferme le tube et teste s'il a bien été fermé
    int ret = close(fd);
    myassert(ret != -1, "La fermeture du tube a échoué");

    printf("Debug : Fermeture du Tube nommé qui a pour %d pour fd\n", fd);
}

// Détruit le tube nommé dont le nom est passé en paramètre
void destroyNamedPipe(const char* name) 
{
    // Détruit le tube et teste s'il a bien été détruit
    int destroy = unlink(name);
    myassert(destroy != 1, "La destruction de tube nomée s'est mal effectué");

    printf("Debug : Destruction du tube nommé : %s\n", name);
}

// Le client envoie l'ordre au master
void clientSendsOrderToMaster(int fd, int order)
{
    // Envoie l'ordre par le tube mis en paramètre
    int ret = write(fd, &order, sizeof(int));
    myassert(ret != -1, "L'envoie de l'ordre au master ne s'est pas bien déroulé");

    printf("Debug : Envoie de l'ordre %d au master\n", order);
}

// Le client envoie le nombre premier a traité au master
// void clientSendsPrimeToMaster(int fd, int prime)
// {
//     // TODO : faire cette fonction pour quand l'odre
//     // est égale à ORDER_COMPUTE_PRIME et qu'il faut
//     // envoyé le nombre premier
// }

// Le master recoit l'ordre du client
int masterReceiveOrderToClient(int fd)
{
    // Lecture de l'ordre envoyé par le client pour le master et teste si ce c'est bien effectué
    int order;
    int ret = read(fd, &order, sizeof(int));
    myassert(ret != -1, "La lecture de l'ordre du client par le master a échoué");

    printf("Debug : Le master vient de recevoir l'odre %d du client\n", order);
    return order;
}

// Le master recoit le nombre premier du client
// int masterReceivePrimeToClient(int fd)
// {
//     // TODO : faire cette fonction pour quand l'odre
//     // est égale à ORDER_COMPUTE_PRIME et qu'il faut
//     // recevoir le nombre premier
// }

// Le master envoie au client combien de nombre premier ont été calculés
void masterSendsHowManyToClient(int fd, int how_many)
{
    // Envoie le nombre de caclul effectué
    int ret = write(fd, &how_many, sizeof(int));
    myassert(ret != -1, "L'envoie du nombre de caclul effectué au client ne s'est pas bien déroulé");

    printf("Debug : Envoie du nombre de caclul effectuée %d au client\n", how_many);
}

// Le client envoie au master combien de nombre premier ont été calculés
int clientReceiveHowManyToMaster(int fd)
{
    // Lecture du nombre de caclul effectué envoyé par le master pour le client et teste si ce c'est bien effectué
    int how_many;
    int ret = read(fd, &how_many, sizeof(int));
    myassert(ret != -1, "La lecture du nombre de caclul effectué du master par le client a échoué");

    printf("Debug : Le client vient de recevoir le nombre de caclul effectué %d du master\n", how_many);
    return how_many;
}

/***********************
 * Pour les sémaphores
 ***********************/

// Renvoie la clé du sémaphore entre les clients si celle-ci s'est bien générée
key_t getKeySemaphoreClients()
{
    // Génére la clé et test si il n'y a pas eu d'erreur
    key_t key = ftok(SEM_FICHIER, SEM_ID_CLIENTS);
    myassert(key != -1, "La clé du sémaphore entre les clients s'est mal générée");

    // Retourne la clé si celle si s'est bien générée
    printf("Debug : Clé SemaphoreClients générée\n");
    return key;
}

// Renvoie la clé du sémaphore entre le master et un client si celle-ci s'est bien générée
key_t getKeySemaphoreMasterClient()
{
    // Génére la clé et test si il n'y a pas eu d'erreur
    key_t key = ftok(SEM_FICHIER, SEM_ID_MASTER_CLIENT);
    myassert(key != -1, "La clé du sémaphore entre le master et un client s'est mal générée");

    // Retourne la clé si celle si s'est bien générée
    printf("Debug : Clé SemaphoreMasterClient générée\n");
    return key;
}

// Renvoie l'identifiant du sémaphore entre les clients qui vient d'être créé
int creationSemaphoreClients()
{
    // Créé le sémpahore entre les clients et vérifie que tout s'est bien passé
    key_t key = getKeySemaphoreClients();
    int semId = semget(key, 1, IPC_CREAT | IPC_EXCL | 0641);
    myassert(semId != -1, "Le sémaphore entre les clients ne s'est pas créé correctement");

    // Retourne l'identifiant si celui si a bien été généré
    printf("Debug : Id SemaphoreClients générée\n");
    return semId;
}

// Renvoie l'identifiant du sémaphore entre le master et un client qui vient d'être créé
int creationSemaphoreMasterClient()
{
    // Créé le sémpahore entre le master et un client et vérifie que tout s'est bien passé
    key_t key = getKeySemaphoreMasterClient();
    int semId = semget(key, 1, IPC_CREAT | IPC_EXCL | 0641);
    myassert(semId != -1, "Le sémaphore entre le master et un client ne s'est pas créé correctement");

    // Retourne l'identifiant si celui si a bien été généré
    printf("Debug : Id SemaphoreMasterClient générée\n");
    return semId;
}

// Renvoie l'identifiant du sémaphore entre les clients qui a déjà était créé
int getIdSemaphoreClients()
{
    // Récupére le sémpahore entre les clients et vérifie que tout s'est bien passé
    key_t key = getKeySemaphoreClients();
    int semId = semget(key, 1, 0);
    myassert(semId != -1, "L'identifiant du sémaphore entre les clients ne s'est pas récupéré correctement");

    // Retourne l'identifiant si celui si a bien été généré
    return semId;
}

// Renvoie l'identifiant du sémaphore entre le master et un client qui a déjà était créé
int getIdSemaphoreMasterClient()
{
    // Récupére le sémpahore entre le master et un client et vérifie que tout s'est bien passé
    key_t key = getKeySemaphoreMasterClient();
    int semId = semget(key, 1, 0);
    myassert(semId != -1, "L'identifiant du sémaphore entre le master et un client ne s'est pas récupéré correctement");

    // Retourne l'identifiant si celui si a bien été généré
    return semId;
}

// Détruit le sémaphore dont l'identifiant a été passé en paramètre
void detruireSemaphore(int semId)
{
    // Détruit le sémaphore et vérifie qu'aucun problème ne s'est passé
    int ret = semctl(semId, -1, IPC_RMID);
    myassert(ret != -1, "Le sémaphore ne s'est pas correctement détruit");

    printf("Debug : Destruction du Sem qui a pour id : %d\n", semId);
}