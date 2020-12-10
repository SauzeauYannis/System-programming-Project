#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

// include ajoutés

#include <sys/types.h> // ftok, semget, semctl
#include <sys/ipc.h> // ftok, semget, semctl
#include <sys/sem.h> // semget, semctl

// fonctions éventuelles proposées dans le .h

/***********************
 * Pour les sémaphores
 ***********************/

// Renvoie la clé du sémaphore entre les clients si celle si s'est bien générée
key_t getKeySemaphoreClients()
{
    // Génére la clé et test si il n'y a pas eu d'erreur
    key_t key = ftok(SEM_FICHIER, SEM_ID_CLIENTS);
    myassert(key != -1, "La clé du sémaphore entre les clients s'est mal générée");

    // Retourne la clé si celle si s'est bien générée
    printf("Debug : Clé SemaphoreClients générée\n");
    return key;
}

// Renvoie la clé du sémaphore entre le master et un client si celle si s'est bien générée
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