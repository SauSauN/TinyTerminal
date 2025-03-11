# Mini-Shell pour un Système de Fichiers

## 1. Présentation du Sujet
Le but du mini-projet est de réaliser un petit gestionnaire de fichiers. La « partition » du file system sera un fichier UNIX de taille suffisante pour y faire des essais probants, et les opérations de base (création, suppression, copie et déplacement de fichiers et/ou répertoires) devront être disponibles. En complément de ces dernières, une gestion des droits d’accès ainsi que la notion de lien seront également implantées.

La gestion de l’espace libre (par exemple suite à la suppression de fichier(s)) est un aspect important du projet. Il est essentiel de s'assurer que la mémoire est efficacement allouée et libérée afin d'éviter la fragmentation et d'optimiser l'utilisation de l'espace disque.

## 2. Fonctionnalités et Exemples d'Utilisation
Le shell supporte les commandes suivantes :

- `help` : Affiche l'aide des commandes disponibles.
  ```sh
  help
  ```

- `exit` : Quitte le shell.
  ```sh
  exit
  ```

- `pwd` : Affiche le répertoire courant.
  ```sh
  pwd
  ```

- `mkdir <nom>` : Crée un répertoire.
  ```sh
  mkdir mon_dossier
  ```

- `rmdir <nom>` : Supprime un répertoire.
  ```sh
  rmdir mon_dossier
  ```

- `cpdir <src> <dest> <rep>` : Copie un répertoire vers un répertoire cible.
  ```sh
  cpdir source_dossier destination_dossier rep1
  ```

- `cd <nom>` : Change de répertoire.
  ```sh
  cd mon_dossier
  ```

- `ls` : Liste le contenu du répertoire courant.
  ```sh
  ls
  ```

- `touch <nom> <taille>` : Crée un fichier vide de la taille spécifiée.
  ```sh
  touch mon_fichier.txt 1024
  ```

- `stat <nom>` : Affiche les métadonnées d'un fichier.
  ```sh
  stat mon_fichier.txt
  ```

- `chmod <nom> <cible> <perm>` : Modifie les permissions d'un fichier.
  ```sh
  chmod mon_fichier.txt owner rwx
  ```

- `write <nom> <contenu>` : Écrit du contenu dans un fichier.
  ```sh
  write mon_fichier.txt "Ceci est du texte."
  ```

- `cat <nom>` : Affiche le contenu d'un fichier.
  ```sh
  cat mon_fichier.txt
  ```

- `cp <src> <dest> <rep>` : Copie un fichier vers un répertoire cible.
  ```sh
  cp mon_fichier.txt copie_fichier.txt mon_dossier
  ```

- `mv <src> <rep>` : Déplace un fichier vers un répertoire cible.
  ```sh
  mv mon_fichier.txt mon_dossier
  ```

- `reset` : Réinitialise le système de fichiers.
  ```sh
  reset
  ```

## 3. Compilation et Exécution

### Prérequis
- Un compilateur C (GCC recommandé)
- Un système compatible UNIX ou Windows avec un terminal

### Compilation
```sh
gcc -o shell filesystem.c shell.c -Wall -Wextra
```

### Exécution
```sh
./shell
```

## 4. Utilisation
Lorsque le programme démarre, il demande le nom de l'utilisateur, puis affiche le répertoire courant. Vous pouvez alors entrer les commandes listées ci-dessus.

## 5. Structure du Projet
- `main.c` : Point d'entrée du programme
- `shell.c` : Implémente la boucle du shell et le traitement des commandes
- `filesystem.c` : Gère les opérations sur les fichiers et répertoires
- `filesystem.h` : Déclarations des fonctions et structures du système de fichiers

## 6. Auteur

