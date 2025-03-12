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
- **Copier le contenu d'un fichier vers un autre :**
  ```bash
  -cp <fichier1> <fichier2>
  ```
  Exemple :
  ```bash
  -cp file1.txt file2.txt
  ```

- `mv <src> <rep>` : Déplace un fichier vers un répertoire cible.
  ```sh
  mv mon_fichier.txt mon_dossier
  ```

- `reset` : Réinitialise le système de fichiers.
  ```sh
  reset
  ```

## 3. Structure des inodes

```c
typedef struct {
    char name[MAX_FILENAME];          // Nom du fichier ou du répertoire
    int is_directory;                 // 1 = répertoire, 0 = fichier
    int size;                         // Taille du fichier en octets
    time_t creation_time;             // Date de création
    time_t modification_time;         // Date de la dernière modification
    char owner[MAX_FILENAME];         // Propriétaire du fichier
    char permissions[10];             // Permissions du fichier (ex: "rwxr-xr--")
    int block_indices[NUM_BLOCKS];    // Indices des blocs alloués
    int block_count;                  // Nombre de blocs alloués
} Inode;
```

### Explication :
- **name** : Nom du fichier ou dossier.
- **is_directory** : Indique si l’inode représente un fichier (0) ou un dossier (1).
- **size** : Taille en octets du fichier.
- **creation_time / modification_time** : Dates de création et de dernière modification.
- **owner** : Nom du propriétaire du fichier.
- **permissions** : Permissions sous forme de chaîne (ex : "rwxr-xr--").
- **block_indices** : Tableau contenant les indices des blocs de données utilisés.
- **block_count** : Nombre total de blocs utilisés.

---

## 4. Structure du superbloc

```c
typedef struct {
    int num_blocks;               // Nombre total de blocs
    int num_inodes;               // Nombre total d'inodes
    int free_blocks[NUM_BLOCKS];  // Tableau des blocs libres (1 = libre, 0 = occupé)
    int free_inodes[NUM_INODES];  // Tableau des inodes libres (1 = libre, 0 = occupé)
    Inode inodes[NUM_INODES];     // Table des inodes
} superblock;
```

### Explication :
- **num_blocks** : Nombre total de blocs disponibles.
- **num_inodes** : Nombre total d’inodes.
- **free_blocks** : Tableau qui indique quels blocs sont libres.
- **free_inodes** : Tableau qui indique quels inodes sont libres.
- **inodes** : Table des inodes contenant les métadonnées des fichiers.

---

## 5. Structure du système de fichiers

```c
typedef struct {
    Inode inodes[MAX_FILES];      // Table des inodes
    int inode_count;              // Nombre d'inodes utilisés
    char current_directory[MAX_FILENAME]; // Répertoire actuel
} Filesystem;
```

### Explication :
- **inodes** : Tableau contenant les inodes du système de fichiers.
- **inode_count** : Nombre total d’inodes actuellement utilisés.
- **current_directory** : Stocke le chemin du répertoire courant.

---

## 6. Structure d’entrée de fichier

```c
typedef struct {
    char name[50];      // Nom du fichier
    int inode_index;    // Index de l'inode associé
} file_entry;
```

### Explication :
- **name** : Nom du fichier associé.
- **inode_index** : Index de l’inode correspondant.

## 7. Compilation et Exécution

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

## 8. Utilisation
Lorsque le programme démarre, il demande le nom de l'utilisateur, puis affiche le répertoire courant. Vous pouvez alors entrer les commandes listées ci-dessus.

## 9. Structure du Projet
- `main.c` : Point d'entrée du programme
- `shell.c` : Implémente la boucle du shell et le traitement des commandes
- `filesystem.c` : Gère les opérations sur les fichiers et répertoires
- `filesystem.h` : Déclarations des fonctions et structures du système de fichiers

## 6. Auteur

