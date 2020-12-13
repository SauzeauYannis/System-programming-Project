#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_worker.h"

// Bibliothéques ajoutés
#include <unistd.h>     // Pour: fork, pipe
#include <sys/types.h>  // Pour: wait
#include <sys/wait.h>   // Pour: wait

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...
typedef struct worker
{
    int prime;
    int previous_pipe;
    int next_pipe;
    int master_pipe;
} workerDonnees;

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
    fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
    fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
    fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char * argv[], workerDonnees *data)
{
    if (argc != 4)
        usage(argv[0], "Nombre d'arguments incorrect");
    
    // Convertit les arguments en entier
    int prime = atoi(argv[1]);
    int fd_RD = atoi(argv[2]);
    int fd_WR = atoi(argv[3]);
    // Remplit la structure
    data->prime = prime;
    data->previous_pipe = fd_RD;
    data->next_pipe = NO_NEXT;
    data->master_pipe = fd_WR;
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(workerDonnees data)
{
    // boucle infinie :
    //    attendre l'arrivée d'un nombre à tester
    //    si ordre d'arrêt
    //       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
    //       sortir de la boucle
    //    sinon c'est un nombre à tester, 4 possibilités :
    //           - le nombre est premier
    //           - le nombre n'est pas premier
    //           - s'il y a un worker suivant lui transmettre le nombre
    //           - s'il n'y a pas de worker suivant, le créer

    // Boucle infinie
    while (true)
    {
        // Attente de l'arrivée d'un nombre à tester
        int number = workerNumberToCompute(data.previous_pipe);

        // Si c'est un ordre d'arrêt
        if (number == STOP)
        {
            // Si il y a un worker suivant
            if (data.next_pipe != NO_NEXT)
            {
                workerToNextWorker(data.next_pipe, STOP);
                wait(NULL);
            }
            // On sort de la boucle
            break;
        }
        // Si c'est un nombre à tester
        else
        {
            // Si le nombre à tester est égal au nombre premier dont le worker à la charge
            if (number == data.prime)
            {
                workerIsPrime(data.master_pipe, NUMBER_IS_PRIME);  // SUCCES
            }
            // Si le nombre à tester est divisible par le nombre premier dont le worker à la charge
            else if ((number % data.prime) == 0)
            {
                workerIsPrime(data.master_pipe, NUMBER_NOT_PRIME); // ECHEC
            }
            // Si il y a un worker suivant alors lui transmettre le nombre à tester
            else if (data.next_pipe != NO_NEXT)
            {
                workerToNextWorker(data.next_pipe, number);
            }
            // Sinon il faut créer le nouveau worker qui aura la charge du nombre à tester qui est donc premier
            else
            {
                // Création du tube anonyme
                int workerToWorkerPipe[2];
                int ret = pipe(workerToWorkerPipe);
                myassert(ret != -1, "Le tube anonyme Worker->Worker s'est mal créé");

                // Création d'un nouveau processus pour executer le premier worker 
                pid_t ret_fork = fork();
                myassert(ret_fork != -1, "Le fork du master pour créer les workers s'est mal exécuté");

                // Processus fils issus du fork
                if (ret_fork == 0)
                {
                    int fdReadingWorker = readingSidePipe(workerToWorkerPipe);
                    createWorker(number, fdReadingWorker, data.master_pipe);
                }
                int fdWritingWorker = writingSidePipe(workerToWorkerPipe);
                data.next_pipe = fdWritingWorker;
            }
        }
    }
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
    // Déclaration de la structure à remplir
    workerDonnees data;

    // Traitement des arguments envoyé au master lors de sa création
    parseArgs(argc, argv, &data);
    
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier
    workerIsPrime(data.master_pipe, data.prime);

    // Boucle infinie pour tester si un nombre est premier
    loop(data);

    // Libérer les ressources : fermeture des files descriptors par exemple
    closeFD(data.previous_pipe, data.next_pipe, data.master_pipe);

    return EXIT_SUCCESS;
}
