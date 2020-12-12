#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "myassert.h"

#include "master_client.h"

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
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    int number = 0;
    int order = parseArgs(argc, argv, &number);

    // order peut valoir 5 valeurs (cf. master_client.h) :
    //      - ORDER_COMPUTE_PRIME_LOCAL
    //      - ORDER_STOP
    //      - ORDER_COMPUTE_PRIME
    //      - ORDER_HOW_MANY_PRIME
    //      - ORDER_HIGHEST_PRIME
    //
    // si c'est ORDER_COMPUTE_PRIME_LOCAL
    //    alors c'est un code complètement à part multi-thread
    // sinon
    //    - entrer en section critique :
    //           . pour empêcher que 2 clients communiquent simultanément
    //           . le mutex est déjà créé par le master
    //    - ouvrir les tubes nommés (ils sont déjà créés par le master)
    //           . les ouvertures sont bloquantes, il faut s'assurer que
    //             le master ouvre les tubes dans le même ordre
    //    - envoyer l'ordre et les données éventuelles au master
    //    - attendre la réponse sur le second tube
    //    - sortir de la section critique
    //    - libérer les ressources (fermeture des tubes, ...)
    //    - débloquer le master grâce à un second sémaphore (cf. ci-dessous)
    // 
    // Une fois que le master a envoyé la réponse au client, il se bloque
    // sur un sémaphore ; le dernier point permet donc au master de continuer
    //
    // N'hésitez pas à faire des fonctions annexes ; si la fonction main
    // ne dépassait pas une trentaine de lignes, ce serait bien.

    if (order == ORDER_COMPUTE_PRIME_LOCAL)
    {
        printf("TODO : ORDER_COMPUTE_PRIME_LOCAL\n");
    }
    else
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
            // clientSendsPrimeToMaster(fd_client_master, number);
        }
        else if (order == ORDER_HOW_MANY_PRIME)
        {
            clientHowMany(fd_master_client);
        }
        else if (order == ORDER_HIGHEST_PRIME)
        {
            clientHighestPrime(fd_master_client);
        }
        else if (order == ORDER_STOP)
        {
            /* code */
        }
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
