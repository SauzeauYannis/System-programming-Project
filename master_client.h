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
 * Pour les tubes nommés
 ***********************/

/****** Constantes *******/

// Nombres de tubes nommés utilisés
#define NB_NAMED_PIPES 2

// Indice du tableau de tubes nommés dans la structure du master
    // Indice pour le tube nommé du client vers le master
#define PIPE_CLIENT_MASTER 0
    // Indice pour le tube nommé du master vers le client
#define PIPE_MASTER_CLIENT 1

// Nom des tubes
    // Nom du tube nommé du client vers le master
#define NAMED_PIPE_CLIENT_MASTER "client_to_master"
    // Nom du tube nommé du master vers le client
#define NAMED_PIPE_MASTER_CLIENT "master_to_client"

/****** Fonctions *******/

// Fonctions qui créer les tubes nommés et qui renvoient leurs noms :
const char* createNamedPipe(const char* pipePathName);
const char* createPipeClientMaster();
const char* createPipeMasterClient();

// Fonctions qui ouvrent, en ecritue ou en lecture seul, un tube nommé passé en paramètre
int openNamedPipeInReading(const char* pipe);
int openNamedPipeInWriting(const char* pipe);

// Fonction qui ferme un tube passé en paramètre
void closeNamedPipe(int fd);

// Fonction qui détruit un tube nommé dont le nom est donné en paramètre
void destroyNamedPipe(const char* name);

// Fonctions général d'envoie et réception de données par un tube nommé
void sentData(int fd, int data);
int receiveData(int fd);
// Envoie de l'ordre du client au master qui le réceptionne
void clientOrderMaster(int fd, int order);
int masterOrderClient(int fd);
// Envoie du nombre de calcul au client qui le réceptionne
void masterHowMany(int fd, int how_many);
void clientHowMany(int fd);
// Envoie du plus grand nombre premier au client qui le réceptionne
void masterHighestPrime(int fd, int highest_prime);
void clientHighestPrime(int fd);

// Le client envoie le nombre premier a traité au master
// void clientSendsPrimeToMaster(int fd, int prime);

// Le master recoit le nombre premier du client
// int masterReceivePrimeToClient(int fd);


/***********************
 * Pour les sémaphores
 ***********************/

#include <sys/types.h> // key_t
#include <sys/ipc.h> // key_t 

/****** Constantes *******/

// Nombres de Sémaphores utilisé
#define NB_SEMAPHORE 2

// Indices du tableau de Sémaphore dans la structure du master
    // Indice pour le sémaphore entre les clients
#define SEM_CLIENTS 0
    // Indice pour le sémaphore entre le master et un client
#define SEM_MASTER_CLIENT 1

// Fichier choisi pour l'identification des sémaphores
#define SEM_FICHIER "master_client.h"

// Identifiants pour le deuxième paramètre de ftok
    // Identifiant pour le sémaphore entre les clients eux-mêmes
#define SEM_ID_CLIENTS 1
    // Identifiant pour le sémaphore entre le master et un client
#define SEM_ID_MASTER_CLIENT 2

// Valeur des opérations sur les sémaphores
    // Augmentation d'un sémaphore
#define SEM_OP_INC +1
    // Diminution d'un sémaphore
#define SEM_OP_DEC -1


/****** Fonctions *******/

// Fonctions pour génerer les clé des sémaphores
key_t getKeySemaphore(int semIdNum);
key_t getKeySemaphoreClients();
key_t getKeySemaphoreMasterClient();

// Fonctions pour créer et initialiser des sémaphores
int creationSemaphore(key_t key, int semValInit);
int creationSemaphoreClients();
int creationSemaphoreMasterClient();

// Fonctions pour récupérer des sémaphores existant
int getIdSemaphore(key_t key);
int getIdSemaphoreClients();
int getIdSemaphoreMasterClient();

// Détruit le sémaphore dont l'identifiant a été passé en paramètre
void detruireSemaphore(int semId);

// Augmente le sémpahore dont l'id est passé en paramètre
void augmenteSemaphore(int semId);

// Diminue le sémpahore dont l'id est passé en paramètre
void diminueSemaphore(int semId);


#endif
