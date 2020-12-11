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
void loop(/* paramètres */)
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

    // Création du premier worker

    // boucle infinie
    loop(/* paramètres */);

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