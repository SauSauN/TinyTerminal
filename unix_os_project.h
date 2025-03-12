#ifndef UNIX_OS_PROJECT_H
#define UNIX_OS_PROJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

// Constantes
#define MAX_FILES 100          // Nombre maximal de fichiers
#define MAX_FILENAME 50        // Taille maximale du nom de fichier
#define FILESYSTEM_FILE "my_filesystem.dat" // Nom du fichier système
#define NAME_SIZE 100          // Taille du nom
#define PERM_SIZE 10           // Taille des permissions
#define BLOCK_SIZE 512         // Taille d'un bloc de données
#define NUM_BLOCKS 1024        // Nombre total de blocs
#define NUM_INODES 128         // Nombre total d'inodes

// Structure représentant un inode (métadonnées d'un fichier ou répertoire)
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
    int inode_count;              // Nombre d'inodes utilisés
    char current_directory[MAX_FILENAME]; // Répertoire actuel
} Filesystem;

// Structure pour associer un nom de fichier à un inode
typedef struct {
    char name[50];      // Nom du fichier
    int inode_index;    // Index de l'inode associé
} file_entry;

// Variables globales
extern superblock sb; 
extern char block_data[NUM_BLOCKS][BLOCK_SIZE]; // Tableau pour stocker les données des fichiers
extern file_entry file_table[NUM_INODES];      // Table des noms de fichiers

// Fonctions du système de fichiers
void init_superblock();
void save_filesystem(Filesystem *fs);
void init_filesystem(Filesystem *fs);
void print_free_blocks();
void reset_filesystem(Filesystem *fs);
void create_directory(Filesystem *fs, const char *dirname);
void delete_directory(Filesystem *fs, const char *dirname);
void change_directory(Filesystem *fs, const char *dirname);
void create_file(Filesystem *fs, const char *filename, size_t size, const char *owner);
void list_directory(Filesystem *fs);
void show_file_metadata(Filesystem *fs, const char *filename);
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

// Fonctions du shell
void help();
void shell(Filesystem *fs, char *current_own);

#endif // UNIX_OS_PROJECT_H
