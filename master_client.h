#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)

// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master


/***********************
 * Pour les sémaphores
 ***********************/

#include <sys/types.h> // key_t
#include <sys/ipc.h> // key_t

// Nombres de Sémaphores utilisé
#define NB_SEMAPHORE 2
// Indice du tableau de Sémaphore dans la structure du master
#define SEM_CLIENTS 0
#define SEM_MASTER_CLIENT 1

// Fichier choisi pour l'identification des sémaphores
#define SEM_FICHIER "master_client.h"

// Identifiants pour le deuxième paramètre de ftok
// Identifiant pour le sémaphore entre les clients eux-mêmes
#define SEM_ID_CLIENTS 1
// Identifiant pour le sémaphore entre le master et un client
#define SEM_ID_MASTER_CLIENT 2

// Renvoie la clé du sémaphore entre les clients si celle si s'est bien générée
key_t getKeySemaphoreClients();

// Renvoie la clé du sémaphore entre le master et un client si celle si s'est bien générée
key_t getKeySemaphoreMasterClient();

// Renvoie l'identifiant du sémaphore entre les clients qui vient d'être créé
int creationSemaphoreClients();

// Renvoie l'identifiant du sémaphore entre le master et un client qui vient d'être créé
int creationSemaphoreMasterClient();

// Renvoie l'identifiant du sémaphore entre les clients qui a déjà était créé
int getIdSemaphoreClients();

// Renvoie l'identifiant du sémaphore entre le master et un client qui a déjà était créé
int getIdSemaphoreMasterClient();

// Détruit le sémaphore dont l'identifiant a été passé en paramètre
void detruireSemaphore(int semId);

// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation


#endif
