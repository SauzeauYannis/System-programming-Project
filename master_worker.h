#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H

// On peut mettre ici des éléments propres au couple master/worker :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (écriture dans un tube, ...)

// Pour savoir si un nombre est premier
    // Le nombre n'est pas premier
#define NUMBER_NOT_PRIME 0
    // Le nombre est premier
#define NUMBER_IS_PRIME 1


// Fonctions pour se mettre à un bout ou un autre d'une pipe anonyme :
    // Fournit le côté lecture d'une pipe entrée en paramètre
int readingSidePipe(int pipe[2]);
    // Fournit le côté écriture d'une pipe entrée en paramètre
int writingSidePipe(int pipe[2]);

// Ferme les file descriptor entrés en paramètre
void closeFD( int fd1, int fd2);

// Permet au worker de dire au master que le nombre envoyé est un nombre premier
void workerIsPrime (int fd, int is_prime);
// Permet au master de savoir si le nombre envoyé au(x) worker(s) est un nombre premier
int masterIsPrime (int fd);

// Permet au master d'envoyer un nombre au premier worker
void masterNumberToCompute(int fd, int number);
// Permet au worker de recevoir le nombre à traiter
int workerNumberToCompute(int fd);

void createWorker_2(int prime_2, int pipe_MtoW, int pipe_WtoM);


#endif
