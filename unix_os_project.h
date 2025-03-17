#ifndef UNIX_OS_PROJECT_H
#define UNIX_OS_PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h> // Pour setlocale (gestion des caractères spéciaux)
#include <time.h>   // Pour la gestion du temps (création/modification des fichiers)

// Définition des constantes
#define MAX_FILES 100          // Nombre maximal de fichiers
#define MAX_FILENAME 50        // Taille maximale du nom de fichier
#define FILESYSTEM_FILE "my_filesystem.dat" // Nom du fichier contenant le système de fichiers
#define NAME_SIZE 100          // Taille du nom
#define PERM_SIZE 11           // Taille des permissions
#define BLOCK_SIZE 512         // Taille d'un bloc de données
#define NUM_BLOCKS 1024        // Nombre total de blocs
#define NUM_INODES 128         // Nombre total d'inodes
#define GROUP_SIZE 10          // Taille de groupe par personne
#define NUM_USER 20            // Nombre total d'utilisateurs

// Variables globales
extern char current_own[NAME_SIZE];  // Utilisateur actuel
extern char current_group[GROUP_SIZE];  // Groupe actuel
extern char permissions[PERM_SIZE];  // Permissions par défaut

// Structure représentant un inode (métadonnées d'un fichier ou répertoire)
typedef struct {
    char name[MAX_FILENAME];          // Nom du fichier ou du répertoire
    int is_directory;                 // Indicateur si c'est un répertoire (1) ou un fichier (0)
    int size;                         // Taille du fichier en octets
    char group[GROUP_SIZE];           // Groupe associé fichier ou du répertoire
    time_t creation_time;             // Date de création
    time_t modification_time;         // Date de la dernière modification
    char owner[MAX_FILENAME];         // Propriétaire du fichier
    char permissions[PERM_SIZE];      // Permissions du fichier (ex: "-rwxr-xr--"), du répertoire (ex: "drwxr-xr--")
    int block_indices[NUM_BLOCKS];    // Indices des blocs alloués
    int block_count;                  // Nombre de blocs alloués
} Inode;

// Définition de la structure d'un tableau
typedef struct {
    char data[50];  // Chaîne de caractères
} Tab;

// Structure pour associer une personne à un Group
typedef struct {
    char user[NAME_SIZE];      // Nom de l'utilisateur
    Tab group[GROUP_SIZE];     // Groupe associé à l'utilisateur
    int taille;
} User_Group;

// Structure représentant le superbloc (métadonnées du système de fichiers)
typedef struct {
    int num_blocks;               // Nombre total de blocs
    int num_inodes;               // Nombre total d'inodes
    int free_blocks[NUM_BLOCKS];  // Tableau des blocs libres (1 = libre, 0 = occupé)
    int free_inodes[NUM_INODES];  // Tableau des inodes libres (1 = libre, 0 = occupé)
    Inode inodes[NUM_INODES];     // Table des inodes
} superblock;

// Structure représentant le système de fichiers
typedef struct {
    Inode inodes[MAX_FILES];      // Table des inodes
    User_Group group[NUM_USER];   // Table des groupes
    int inode_count;              // Nombre d'inodes utilisés
    int group_count;              // Nombre de groupes utilisés
    char current_directory[MAX_FILENAME]; // Répertoire actuel
} Filesystem;

// Structure pour associer un nom de fichier à un inode
typedef struct {
    char name[50];      // Nom du fichier
    int inode_index;    // Index de l'inode associé
} file_entry;

// Prototypes des fonctions
void save_filesystem(Filesystem *fs);
void user_add_group(Filesystem *fs, const char *groupname);
void user_delete_group(Filesystem *fs, const char *groupname);
void init_superblock();
void init_filesystem(Filesystem *fs);
void print_free_blocks();
void reset_filesystem(Filesystem *fs);
void create_directory(Filesystem *fs, const char *dirname);
void delete_directory(Filesystem *fs, const char *dirname);
void change_directory(Filesystem *fs, const char *dirname);
void create_file(Filesystem *fs, const char *filename, size_t size, const char *owner);
void list_directory(Filesystem *fs);
void show_file_metadata(Filesystem *fs, const char *filename);
void show_directory_metadata(Filesystem *fs, const char *namerep);
void chmod_file(Filesystem *fs, const char *filename, const char *target, const char *new_permissions);
Inode* get_inode_by_name(Filesystem *fs, const char *filename);
int allocate_block();
int count_free_blocks();
void write_to_file(Filesystem *fs, const char *filename, const char *content);
void read_file(Filesystem *fs, const char *filename);
void delete_file(Filesystem *fs, const char *filename);
int directory_exists(Filesystem *fs, const char *path);
void copy_file(Filesystem *fs, const char *filenamedepart, const char *filenamefinal, const char *nomrepertoire);
void move_file(Filesystem *fs, const char *filename, const char *nomrepertoire);
char* extract_path(const char* full_path);
void copy_repertoire(Filesystem *fs, const char *repertoirenamedepart, const char *repertoirenamefinal, const char *nomrepertoire);
void move_directory(Filesystem *fs, const char *repertoirename, const char *nomrepertoire);
void clear_screen();
void help();
void shell(Filesystem *fs, char *current_own);
void init_main(Filesystem *fs);

#endif // UNIX_OS_PROJECT_H
