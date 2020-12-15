#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "myassert.h"

#include "master_client.h"

// Bibliothéques ajoutés

#include <math.h>     // Pour: sqrt
#include <pthread.h>  // Pour: pthread_t, pthread_create, pthread_join

// chaines possibles pour le premier paramètre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<number>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrêt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit être fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculés\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculé\n");
    fprintf(stderr, "   ordre \"" TK_LOCAL  "\" : calcul de nombre premier en local\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static int parseArgs(int argc, char * argv[], int *number)
{
    int order = ORDER_NONE;

    if ((argc != 2) && (argc != 3))
        usage(argv[0], "Nombre d'arguments incorrect");

    if (strcmp(argv[1], TK_STOP) == 0)
        order = ORDER_STOP;
    else if (strcmp(argv[1], TK_COMPUTE) == 0)
        order = ORDER_COMPUTE_PRIME;
    else if (strcmp(argv[1], TK_HOW_MANY) == 0)
        order = ORDER_HOW_MANY_PRIME;
    else if (strcmp(argv[1], TK_HIGHEST) == 0)
        order = ORDER_HIGHEST_PRIME;
    else if (strcmp(argv[1], TK_LOCAL) == 0)
        order = ORDER_COMPUTE_PRIME_LOCAL;
    
    if (order == ORDER_NONE)
        usage(argv[0], "ordre incorrect");
    if ((order == ORDER_STOP) && (argc != 2))
        usage(argv[0], TK_STOP" : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME) && (argc != 3))
        usage(argv[0], TK_COMPUTE " : il faut le second argument");
    if ((order == ORDER_HOW_MANY_PRIME) && (argc != 2))
        usage(argv[0], TK_HOW_MANY" : il ne faut pas de second argument");
    if ((order == ORDER_HIGHEST_PRIME) && (argc != 2))
        usage(argv[0], TK_HIGHEST " : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME_LOCAL) && (argc != 3))
        usage(argv[0], TK_LOCAL " : il faut le second argument");
    if ((order == ORDER_COMPUTE_PRIME) || (order == ORDER_COMPUTE_PRIME_LOCAL))
    {
        *number = strtol(argv[2], NULL, 10);
        if (*number < 2)
             usage(argv[0], "le nombre doit être >= 2");
    }       
    
    return order;
}

/************************************************************************
 * Fonction locale pour le calcul d'un nombre premier
 ************************************************************************/

// Paramètre pour les threads
typedef struct
{
    int testedPrime;
    int caseNumber;
    bool result;
} ThreadData;


// Fonction support d'un thread
void * codeThread(void * arg)
{
    ThreadData *data = (ThreadData *) arg;

    // On ajoute 2 au numéro de la case pour que la
    // première case du tableau corresponde au chiffre
    // 2 et ainsi de suite
    int caseNum = data->caseNumber + 2;
    // Si le nombre a tester est divisible par le nombre
    // dont le thread a la charge alors la case est mise à false
    data->result = (data->testedPrime % caseNum) != 0;
    return NULL;
}

void computeLocalPrime(int number)
{
    // Il y a racine(number)-1 threads qu'on stocke dans un tableau d'id
    int nbThreads = (int) sqrt(number) - 1;
    pthread_t idThreadsArray[nbThreads];
    ThreadData datas[nbThreads];

    // Lancement des threads
    for (int i = 0; i < nbThreads; i++)
    {
        // Ininitalistion des données de chaque thread
        datas[i].testedPrime = number;
        datas[i].caseNumber = i;
        datas[i].result = false;
        // Création du thread numéro i
        int ret = pthread_create(&(idThreadsArray[i]), NULL, codeThread, &(datas[i]));
        myassert(ret == 0, "Un thread s'est mal créé");
    }
    
    // Attente de la fin des threads
    for (int i = 0; i < nbThreads; i++)
    {
        int ret = pthread_join(idThreadsArray[i], NULL);
        myassert(ret == 0, "L'attente de la fin d'un thread s'est mal déroulé");
    }
    
    // On regarde si toutes les cases sont à true
    for (int i = 0; i < nbThreads; i++)
    {
        // Si la case est à false alors le nombre n'est pas premier et on arrête la fonction
        if (!datas[i].result)
        {
            printf("Le nombre %d n'est pas premier\n", number);
            return;
        }  
    }
    
    // Si on arrive ici c'est que toutes les cases testées étaient à true donc le nombre est premier
    printf("Le nombre %d est premier\n", number);
}

/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    int number = 0;
    int order = parseArgs(argc, argv, &number);

    // Si l'ordre est de calculer localement
    // on utilise la fonction avec les threads ci-dessus
    if (order == ORDER_COMPUTE_PRIME_LOCAL)
    {
        computeLocalPrime(number);
    }
    else // Sinon il faut communiquer avec le master
    {
        // On prend le sémaphore qui gére les relations entre les clients
        int semIdClients = getIdSemaphoreClients();
        // On diminue ce sémaphore pour passer en section critique
        diminueSemaphore(semIdClients);

        // Ouverture des tubes
        // Ouverture du tube nommé client vers master en écriture
        int fd_client_master = openNamedPipeInWriting(NAMED_PIPE_CLIENT_MASTER);
        // Ouverture du tube nommé master vers client en lecture
        int fd_master_client = openNamedPipeInReading(NAMED_PIPE_MASTER_CLIENT);

        // Envoie de l'ordre sur le tube nommé client vers master
        clientOrderMaster(fd_client_master, order);

        // Si le client doit transmettre le nombre premier à calculer
        if (order == ORDER_COMPUTE_PRIME)
        {
            clientCompute(fd_client_master, number); // Envoi du nombre à tester
            clientPrime(fd_master_client, number);   // Réception qui indique si le nombre est premire
        }
        else if (order == ORDER_HOW_MANY_PRIME)
            clientHowMany(fd_master_client);
        else if (order == ORDER_HIGHEST_PRIME)
            clientHighestPrime(fd_master_client);
        else if (order == ORDER_STOP)
            clientStop(fd_master_client);
        else // Ne doit normalement jamais aller ici sinon il y a une erreur dans le programme
        {
            fprintf(stderr, "L'ordre du client est inconnu\n");
            return EXIT_FAILURE;
        }

        // On augmente le sémaphore entre les clients pour sortir de la section critique
        augmenteSemaphore(semIdClients);

        // Fermeture des tubes
            // Fermeture du tube nommé client vers master
        closeNamedPipe(fd_client_master);
            // Fermeture du tube nommé master vers client
        closeNamedPipe(fd_master_client);

        // On prend le sémaphore qui gére les relations entre le master et un client
        int semIdMasterClient = getIdSemaphoreMasterClient();
        // On augmente ce sémaphore pour débloquer le master
        augmenteSemaphore(semIdMasterClient);
    }
    
    return EXIT_SUCCESS;
}
