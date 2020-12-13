#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

// Pour test (à mettre ds le master_worker.c)
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>


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
    int nb_to_compute;
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
    while (true)
    {
        // Ouverture des tubes:
            // Ouverture du tube nommé client vers master en lecture
        int fd_client_master = openNamedPipeInReading(donnees.named_tubes[PIPE_CLIENT_MASTER]);
            // Ouverture du tube nommé master vers client en écriture
        int fd_master_client = openNamedPipeInWriting(donnees.named_tubes[PIPE_MASTER_CLIENT]);

        // Le master recoit l'ordre donné par le client
        int order = masterOrderClient(fd_client_master);

        if (order == ORDER_COMPUTE_PRIME)
        {
            int compute_prime = masterCompute(fd_client_master);
            printf("MASTER : En train de calculer si %d est un nombre premier\n", compute_prime);

            // TODO : Test de la primalité avec les workers
            if (true)
                masterPrime(fd_master_client, NUMBER_IS_PRIME);
            else
                masterPrime(fd_master_client, NUMBER_NOT_PRIME);
        }
        else if (order == ORDER_HOW_MANY_PRIME)
            masterHowMany(fd_master_client, donnees.how_many);
        else if (order == ORDER_HIGHEST_PRIME)
            masterHighestPrime(fd_master_client, donnees.highest);
        else if (order == ORDER_STOP)
            masterStop(fd_master_client, ORDER_STOP);
        else // Ne doit normalement jamais aller ici sinon il y a une erreur dans le programme
        {
            fprintf(stderr, "L'ordre du client recu par le master est inconnu\n");
            break;
        }

        // Fermeture des tubes
            // Fermeture du tube nommé client vers master
        closeNamedPipe(fd_client_master);
            // Fermeture du tube nommé master vers client
        closeNamedPipe(fd_master_client);

        // On diminue le sémaphore entre le master et le client pour attendre la fin du client
        diminueSemaphore(donnees.semaphores[SEM_MASTER_CLIENT]);

        printf("MASTER : En attente d'une instruction d'un client...\n");

        // Si l'ordre est de stoper le master on sort de la boucle
        if (order == ORDER_STOP)
            break;
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
    printf("Les sémaphores se sont créés et initialisé correctement\n");

    // - Création des tubes nommés
        // Création du tube nommé client vers master
    donnes.named_tubes[PIPE_CLIENT_MASTER] = createPipeClientMaster();
        // Création du tube nommé master vers client
    donnes.named_tubes[PIPE_MASTER_CLIENT] = createPipeMasterClient();
    printf("Les tubes nommés se sont créés correctement\n");

    // Initialisation du nombre de calcul fait et du plus grand nombre premier trouvé à 0
    donnes.how_many = 0;
    donnes.highest = 0;

    // TODO : Création du premier worker
    
    //faire un tube anonyme
    int anoPipeMtoW[2];
    int anoPipeWtoM[2];
    int ret;
    //pid_t ret_fork;

    ret = pipe(anoPipeMtoW);
    myassert(ret != -1, "La pipe anonyme Master->Worker s'est mal créée");
    ret = pipe(anoPipeWtoM);
    myassert(ret != -1, "La pipe anonyme Worker->Master s'est mal créée");

    // Phase test
    int number = 3;
    printf("\tMaster envoie %d à Worker_2\n", number);
    masterNumberToCompute(writingSidePipe(anoPipeMtoW), number);
    createWorker_2(2, readingSidePipe(anoPipeMtoW), writingSidePipe(anoPipeWtoM));
    close(writingSidePipe(anoPipeMtoW));
    printf("\tMaster reçoit %d de Worker_2\n", masterIsPrime(readingSidePipe(anoPipeWtoM)));
    close(readingSidePipe(anoPipeWtoM));
    
    /* TO DO:
    ret_fork = fork();
    myassert(ret_fork != -1, "Le fork du master pour créer les workers s'est mal exécuté");

    if (ret_fork == 0)
    {
    }
    
    for (int i = 0; i < count; i++)
    {
 
    }
    */    

    printf("MASTER : En attente d'une instruction d'un client...\n");

    // boucle infinie
    loop(donnes);

    printf("MASTER : En cours d'extinction...");

    // - Destruction des sémaphores:
        // Destruction du sémaphore entre les clients
    detruireSemaphore(donnes.semaphores[SEM_CLIENTS]);
        // Destruction du sémaphore entre le master et un client
    detruireSemaphore(donnes.semaphores[SEM_MASTER_CLIENT]);
    printf("Les sémaphores se sont détruit correctement\n");

    // - Destruction des tubes nommés:
        // Destruction du tube nommé client vers master
    destroyNamedPipe(donnes.named_tubes[PIPE_CLIENT_MASTER]);
        // Destruction du tube nommé master vers client 
    destroyNamedPipe(donnes.named_tubes[PIPE_MASTER_CLIENT]);
    printf("Les tubes nommés se sont détruit correctement\n");

    return EXIT_SUCCESS;
}