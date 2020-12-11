#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin
typedef struct master
{
    int semaphores[NB_SEMAPHORE];
    const char* named_tubes[NB_NAMED_PIPES];
    int how_many;
    int highest;
} masterDonnees;


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(masterDonnees donnees)
{
    // boucle infinie :
    // - ouverture des tubes (cf. rq client.c)
    // - attente d'un ordre du client (via le tube nommé)
    // - si ORDER_STOP
    //       . envoyer ordre de fin au premier worker et attendre sa fin
    //       . envoyer un accusé de réception au client
    // - si ORDER_COMPUTE_PRIME
    //       . récupérer le nombre N à tester provenant du client
    //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
    //             il faut connaître le plus nombre (M) déjà enovoyé aux workers
    //             on leur envoie tous les nombres entre M+1 et N-1
    //             note : chaque envoie déclenche une réponse des workers
    //       . envoyer N dans le pipeline
    //       . récupérer la réponse
    //       . la transmettre au client
    // - si ORDER_HOW_MANY_PRIME
    //       . transmettre la réponse au client
    // - si ORDER_HIGHEST_PRIME
    //       . transmettre la réponse au client
    // - fermer les tubes nommés
    // - attendre ordre du client avant de continuer (sémaphore : précédence)
    // - revenir en début de boucle
    //
    // il est important d'ouvrir et fermer les tubes nommés à chaque itération
    // voyez-vous pourquoi ?

    // boucle infinie
    while (true)
    {
        // Ouverture des tubes
        // Ouverture du tube nommé client vers master en lecture
        int fd_client_master = openPipeInReading(donnees.named_tubes[PIPE_CLIENT_MASTER]);
        // Ouverture du tube nommé master vers client en écriture
        int fd_master_client = openPipeInWriting(donnees.named_tubes[PIPE_MASTER_CLIENT]);

        // Le master recoit l'ordre donné par le client
        int order = masterReceiveOrderToClient(fd_client_master);

        if (order == ORDER_COMPUTE_PRIME)
        {
            // int prime = masterReceivePrimeToClient(fd_client_master);
        }
        else if (order == ORDER_HOW_MANY_PRIME)
        {
            masterSendsHowManyToClient(fd_master_client, donnees.how_many);
        }
        else if (order == ORDER_HIGHEST_PRIME)
        {
            /* code */
        }
        else if (order == ORDER_STOP)
        {
            /* code */
        }
        else // Ne doit normalement jamais aller ici sinon il y a une erreur dans le programme
        {
            fprintf(stderr, "L'ordre du client recu par le master est inconnu");
            break;
        }

        // Fermeture des tubes
        // Fermeture du tube nommé client vers master
        closePipe(fd_client_master);
        // Fermeture du tube nommé master vers client
        closePipe(fd_master_client);

        break; // Pour sortir de la boucle infine
    }
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    // Déclaration de la structure qui stocke les données utiles au master
    masterDonnees donnes;

    // CREATIONS:

    // - Création des sémaphores:
        // Création du sémaphore entre les clients
    donnes.semaphores[SEM_CLIENTS] = creationSemaphoreClients();
        // Création du sémaphore entre le master et un client
    donnes.semaphores[SEM_MASTER_CLIENT] = creationSemaphoreMasterClient();

    // - Création des tubes nommés
        // Création du tube nommé client vers master
    donnes.named_tubes[PIPE_CLIENT_MASTER] = createPipeClientMaster();
        // Création du tube nommé master vers client
    donnes.named_tubes[PIPE_MASTER_CLIENT] = createPipeMasterClient();

    // Initialisation du nombre de calcul fait et du plus grand nombre premier trouvé à 0
    donnes.how_many = 0;
    donnes.highest = 0;

    // Création du premier worker

    // boucle infinie
    loop(donnes);

    // - Destruction des sémaphores:
        // Destruction du sémaphore entre les clients
    detruireSemaphore(donnes.semaphores[SEM_CLIENTS]);
        // Destruction du sémaphore entre le master et un client
    detruireSemaphore(donnes.semaphores[SEM_MASTER_CLIENT]);

    // - Destruction des tubes nommés:
        // Destruction du tube nommé client vers master
    destroyNamedPipe(donnes.named_tubes[PIPE_CLIENT_MASTER]);
        // Destruction du tube nommé master vers client 
    destroyNamedPipe(donnes.named_tubes[PIPE_MASTER_CLIENT]);

    return EXIT_SUCCESS;
}