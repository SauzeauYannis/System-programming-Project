#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

// Bibliothéques ajoutés
#include <unistd.h>     // Pour: fork, pipe
#include <sys/types.h>  // Pour: wait
#include <sys/wait.h>   // Pour: wait

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// Définition de la structure des données pour le master
typedef struct
{
    int semaphores[NB_SEMAPHORE];
    const char* named_tubes[NB_NAMED_PIPES];
    int masterToWorkerPipe[SIZE_PIPE];
    int workerToMasterPipe[SIZE_PIPE];
    int fdWritingMaster;
    int fdReadingMaster;
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
    // Boucle infinie
    while (true)
    {
        printf("MASTER : En attente d'une instruction d'un client...\n");

        // Ouverture des tubes:
            // Ouverture du tube nommé client vers master en lecture
        int fd_client_master = openNamedPipeInReading(donnees.named_tubes[PIPE_CLIENT_MASTER]);
            // Ouverture du tube nommé master vers client en écriture
        int fd_master_client = openNamedPipeInWriting(donnees.named_tubes[PIPE_MASTER_CLIENT]);

        // Le master recoit l'ordre donné par le client
        int order = masterOrderClient(fd_client_master);

        // Si l'ordre indique de calculer un nombre premier
        if (order == ORDER_COMPUTE_PRIME)
        {
            // On récupère le nombre premier à calculer
            int compute_prime = masterCompute(fd_client_master);
            // Avant d'envoyer le nombre dans la pipeline on doit s'assurrer d'envoyer tous les
            // nombres compris entre le nombre à calculer et le plus grand nombre trouvé
            if (compute_prime > donnees.highest)
            {
                // On envoie les nombres un par un et on regarde si il y a un nouveau nombre premier qui a été trouvé
                for (int i = (donnees.highest + 1); i < compute_prime; i++)
                {
                    masterNumberToCompute(donnees.fdWritingMaster, i);
                    int result = masterIsPrime(donnees.fdReadingMaster);
                    // Si le nombre parcouru est premier il est alors le plus grand nombre premier
                    if (result == NUMBER_IS_PRIME || result > 1)
                    {
                        donnees.how_many++;
                        donnees.highest = i;
                    }
                }
            }
            

            // On envoie le nombre à tester au premier worker
            masterNumberToCompute(donnees.fdWritingMaster, compute_prime);

            // On regarde les résultat que nous a envoyé un worker
            int result = masterIsPrime(donnees.fdReadingMaster);
            if (result > 1)
            {
                result = masterIsPrime(donnees.fdReadingMaster);
            }
            // Si le nombre est premier on l'indique au client
            if (result == NUMBER_IS_PRIME)
            {
                masterPrime(fd_master_client, NUMBER_IS_PRIME);
                // Si le nombre testé est plus grand que le plus grand nombre 
                // premier stocké dans la mémoire on le remplace
                if (compute_prime > donnees.highest)
                {
                    donnees.how_many++;
                    donnees.highest = compute_prime;
                }
            }
            // Si il n'est pas premier on l'indique aussi au client
            else if (result == NUMBER_NOT_PRIME)
            {
                masterPrime(fd_master_client, NUMBER_NOT_PRIME);
            }    
        }
        else if (order == ORDER_HOW_MANY_PRIME)
            masterHowMany(fd_master_client, donnees.how_many);      // Envoi du nombre de nombre premier calculés
        else if (order == ORDER_HIGHEST_PRIME)
            masterHighestPrime(fd_master_client, donnees.highest);  // Envoi du plus grand nombre premier
        else if (order == ORDER_STOP)
        {
            // On envoie l'ordre d'arret au premier worker
            masterNumberToCompute(donnees.fdWritingMaster, STOP);
            // On attend la fin du premier worker
            wait(NULL);
            masterStop(fd_master_client, ORDER_STOP);               // Envoi d'un accusé de réception au client
        }
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

        // Si l'ordre est de stoper le master on sort de la boucle
        if (order == ORDER_STOP)
            break;
    }
}

/************************************************************************
 * Initialisation des données
 ************************************************************************/

masterDonnees initDonnees()
{
    // Déclaration de la structure qui stocke les données utiles au master
    masterDonnees donnees;

    // - Création des sémaphores:
        // Création du sémaphore entre les clients
    donnees.semaphores[SEM_CLIENTS] = creationSemaphoreClients();
        // Création du sémaphore entre le master et un client
    donnees.semaphores[SEM_MASTER_CLIENT] = creationSemaphoreMasterClient();
    printf("Les sémaphores se sont créés et initialisé correctement\n");

    // - Création des tubes nommés
        // Création du tube nommé client vers master
    donnees.named_tubes[PIPE_CLIENT_MASTER] = createPipeClientMaster();
        // Création du tube nommé master vers client
    donnees.named_tubes[PIPE_MASTER_CLIENT] = createPipeMasterClient();
    printf("Les tubes nommés se sont créés correctement\n");

    // Initialisation du nombre de calcul fait et du plus grand nombre premier trouvé à 0
    donnees.how_many = 0;
    donnees.highest = 0;
    
    // Création des deux premiers tubes anonymes
    int ret;
        // Tube anonyme du master vers le worker
    ret = pipe(donnees.masterToWorkerPipe);
    myassert(ret != -1, "Le tube anonyme Master->Worker s'est mal créé");
        // Tube anonyme du worker vers le master
    ret = pipe(donnees.workerToMasterPipe);
    myassert(ret != -1, "Le tube anonyme Worker->Master s'est mal créé");
    printf("Les tubes anonymes se sont créés correctement\n");

    return donnees;
}

/************************************************************************
 * Destruction des données
 ************************************************************************/

void detruireDonnees(masterDonnees donnees)
{
    // - Destruction des sémaphores:
        // Destruction du sémaphore entre les clients
    detruireSemaphore(donnees.semaphores[SEM_CLIENTS]);
        // Destruction du sémaphore entre le master et un client
    detruireSemaphore(donnees.semaphores[SEM_MASTER_CLIENT]);
    printf("Les sémaphores se sont détruit correctement\n");

    // - Destruction des tubes nommés:
        // Destruction du tube nommé client vers master
    destroyNamedPipe(donnees.named_tubes[PIPE_CLIENT_MASTER]);
        // Destruction du tube nommé master vers client 
    destroyNamedPipe(donnees.named_tubes[PIPE_MASTER_CLIENT]);
    printf("Les tubes nommés se sont détruit correctement\n");
}

/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    // On initialise les données
    masterDonnees donnees = initDonnees();

    // Premier nombre premier à passer au premier worker
    int firstPrimeNumber = 2;
    
    // Création d'un nouveau processus pour executer le premier worker 
    pid_t ret_fork = fork();
    myassert(ret_fork != -1, "Le fork du master pour créer le premier worker s'est mal exécuté");

    // Processus fils issus du fork
    if (ret_fork == 0)
    {
        // On initialise les files descriptors des tubes anonymes dont le premier worker a besoin
        donnees.fdReadingMaster = readingSidePipe(donnees.masterToWorkerPipe);
        donnees.fdWritingMaster = writingSidePipe(donnees.workerToMasterPipe);
        // On créé le premier worker avec le premier nombre premier (ici 2)
        createWorker(firstPrimeNumber, donnees.fdReadingMaster, donnees.fdWritingMaster); // Exec donc le fils s'arrête ici
    }

    // Processus père issus du fork
    // On initialise les files descriptors des tubes anonymes dont le master a besoin
    donnees.fdWritingMaster = writingSidePipe(donnees.masterToWorkerPipe);
    donnees.fdReadingMaster = readingSidePipe(donnees.workerToMasterPipe);
    // On envoie au premier worker le le premier nombre premier (ici 2)
    masterNumberToCompute(donnees.fdWritingMaster, firstPrimeNumber);
    // On vérifie que le premier worker a reconnu 2 comme étant un nombre premier
    int isPrime = masterIsPrime(donnees.fdReadingMaster);
    if ((isPrime == NUMBER_IS_PRIME) || isPrime > 1 )
    {
        // L'initialisation du premier worker est faite
        printf("Le premier worker a bien reconnu %d comme étant un nombre premier\n", firstPrimeNumber);
        printf("Vous pouvez maintenant lancer un client pour tester d'autres nombres premier\n");
        donnees.how_many++;
        donnees.highest = firstPrimeNumber;
    }
    else // Si le programme vient ici c'est qu'il y a eu un problème lors de la création du premier worker
    {
        fprintf(stderr, "La création du premier worker ne s'est pas bien déroulée\n");
        // Destruction des données
        detruireDonnees(donnees);
        return EXIT_FAILURE;
    }

    // Boucle infinie pour la communication avec le client
    loop(donnees);

    // Destruction des données
    detruireDonnees(donnees);

    return EXIT_SUCCESS;
}