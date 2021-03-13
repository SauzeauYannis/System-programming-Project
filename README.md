# System-programming-Project

##### Table of Contents
* [Français](#fr)
  * [Présentation](#fr_pr)
  * [Utilisation](#fr_ut)
  * [Compétences acquises](#fr_cp)
  * [Résultat](#fr_rs)
* [English](#en)
  * [Presentation](#en_pr)
  * [Use](#en_u)
  * [Skills acquired](#en_sk)
  * [Result](#en_rs)

<a name="fr"/>

## Français

<a name="fr_pr"/>

### Présentation

Ce projet a été effectué en troisième année du [CMI Informatique](http://formations.univ-poitiers.fr/fr/index/autre-diplome-niveau-master-AM/autre-diplome-niveau-master-AM/cmi-informatique-JD2XQGVY.html) à l'[UFR SFA Université de Poitiers](https://sfa.univ-poitiers.fr/) dans le cadre de l'enseignement [Programmation avancée en C](http://formations.univ-poitiers.fr/fr/index/autre-diplome-niveau-master-AM/autre-diplome-niveau-master-AM/cmi-informatique-JD2XQGVY/specialite-s5-JD2XSMB7/programmation-avancee-en-c-JBI8HO7J.html).

Ce projet a été développé en binôme et avec l'EDI [Visual Studio Code](https://code.visualstudio.com/) sur Ubuntu.

<a name="fr_ut"/>

### Utilisation

Le but est de determiner si un nombre est premier en se basant sur le crible d’Eratosthène, et plus precisement sur la version pipeline : le [crible de Hoare](https://fr.wikipedia.org/wiki/Crible_d'%C3%89ratosth%C3%A8ne#Version_pipe-line_:_le_Crible_de_Hoare_(1978)).

Pour lancer le programme sous linux, utilisez la commande "./master" en premier, puis lancer autant de "./client" que nécessaire.

Vous intéragissez avec le programme client qui envoie l'ordre au master qui créé des workers pour traiter la demande.

Le client peut demander au master de savoir si un nombre est premier, combien il en a calculé et quel est le plus grand nombre premier qu'il a calculé.

Le client peut aussi calculer si un nombre est premier localement et sans l'aide du master.

Chaque worker à la charge d'un seul nombre premier.

Si vous voulez recompiler le projet, utilisez la commande "make".

<a name="fr_cp"/>

### Compétences acquises

* Programmation système Unix
  * Communication inter-processus
    * Utilisation de sémaphores
    * Utilisation de tubes nommés
    * Utilisation de tubes anonymes
    * Utilisation des fichiers
  * Communication bidirectionnelle
  * Processus léger
* Programmation en C
  * Programmation modulaire
  * Utilisation de débogueur mémoire (Valgrind)
  * Entrées-sorties de fichiers

<a name="fr_rs"/>

### Résultat

Nous avons obtenu la note de 16/20. (résultat en Février)

<a name="en"/>

## English

<a name="en_pr"/>

### Presentation

This project was carried out in the third year of the [CMI Informatique](http://formations.univ-poitiers.fr/fr/index/autre-diplome-niveau-master-AM/autre-diplome-niveau-master-AM/cmi-informatique-JD2XQGVY.html) at the [University of Poitiers](https://www.univ-poitiers.fr/en/) as part of the [Advanced C programming](http://formations.univ-poitiers.fr/fr/index/autre-diplome-niveau-master-AM/autre-diplome-niveau-master-AM/cmi-informatique-JD2XQGVY/specialite-s5-JD2XSMB7/programmation-avancee-en-c-JBI8HO7J.html) teaching programme.

This project was developed in pairs and with [Visual Studio Code](https://code.visualstudio.com/) IDE on Ubuntu.

<a name="en_u"/>

### Use

The goal is to determine if a number is prime based on the Sieve of Eratosthenes, and more precisely on the pipeline version: the Hoare sieve.

To launch the programme under linux, use the "./master" command first, then launch as many "./client" as necessary.

You interact with the client program which sends the order to the master who creates workers to process the request.

The client can ask the master to find out if a number is prime, how many it has calculated, and what is the largest prime number it has calculated.

The client can also calculate whether a number is prime locally and without the help of the master.

Each worker is responsible for a single prime number.

If you want to recompile the project, use the "make" command.

<a name="en_sk"/>

### Skills acquired

* Unix system programming
  * Inter-process communication
    * Use of semaphores
    * Use of named pipelines
    * Use of anonymous pipelines
    * Use of files
  * Bi-directional communication
  * Threads
* Programming in C
  * Modular programming
  * Use of memory debugger (Valgrind)
  * File input/output
  
<a name="en_rs"/>

### Result

We obtained a score of 16/20. (result in February)
