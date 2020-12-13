#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "myassert.h"

#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...
typedef struct worker
{
    int prime;
    int number;
    int pipe_RD;
    int pipe_WR;
    int pipe_WR_master;
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
    data->pipe_RD = fd_RD;
    data->pipe_WR_master = fd_WR;

}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(/* paramètres */)
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
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
    // Déclaration de la structure à remplir
    workerDonnees data;

    parseArgs(argc, argv, &data);
    
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier

    // Phase test
    printf("\tWorker_2 reçoit %d de Master \n", workerNumberToCompute(data.pipe_RD));
    printf("\tWorker_2 envoie %d à Master\n", NUMBER_IS_PRIME);
    workerIsPrime(data.pipe_WR_master, NUMBER_IS_PRIME);
    
    

    loop(/* paramètres */);

    // libérer les ressources : fermeture des files descriptors par exemple
    closeFD(data.pipe_RD, data.pipe_WR_master); //TO DO: ajouter un paramètre data.pipe_WR

    return EXIT_SUCCESS;
}
