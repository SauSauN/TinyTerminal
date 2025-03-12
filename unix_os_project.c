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
#define PERM_SIZE 10           // Taille des permissions
#define BLOCK_SIZE 512         // Taille d'un bloc de données
#define NUM_BLOCKS 1024        // Nombre total de blocs
#define NUM_INODES 128         // Nombre total d'inodes

char current_own[NAME_SIZE];  // Utilisateur actuel
char permissions[PERM_SIZE];  // Permissions par défaut

// Structure représentant un inode (métadonnées d'un fichier ou répertoire)
typedef struct {
    char name[MAX_FILENAME];          // Nom du fichier ou du répertoire
    int is_directory;                 // Indicateur si c'est un répertoire (1) ou un fichier (0)
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
superblock sb; 
char block_data[NUM_BLOCKS][BLOCK_SIZE]; // Tableau pour stocker les données des fichiers
file_entry file_table[NUM_INODES];      // Table des noms de fichiers

// Fonction pour initialiser le superbloc
void init_superblock() {
    sb.num_blocks = NUM_BLOCKS;
    sb.num_inodes = NUM_INODES;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        sb.free_blocks[i] = 1; // 1 = libre, 0 = occupé
    }
    for (int i = 0; i < NUM_INODES; i++) {
        sb.free_inodes[i] = 1; // 1 = libre, 0 = occupé
        file_table[i].inode_index = -1; // Aucun inode associé initialement
    }
}

// Fonction pour sauvegarder le système de fichiers dans un fichier
void save_filesystem(Filesystem *fs) {
    FILE *file = fopen(FILESYSTEM_FILE, "wb");
    if (file) {
        fwrite(fs, sizeof(Filesystem), 1, file);
        fwrite(&sb, sizeof(superblock), 1, file);  // Sauvegarder le superbloc
        fwrite(block_data, sizeof(block_data), 1, file);  // Sauvegarder les blocs de données
        fclose(file);
    } else {
        printf("Erreur lors de la sauvegarde du système de fichiers.\n");
    }
}

// Fonction pour initialiser ou charger le système de fichiers
void init_filesystem(Filesystem *fs) {
    FILE *file = fopen(FILESYSTEM_FILE, "rb");
    if (!file) {
        printf("Fichier système non trouvé, initialisation...\n");
        fs->inode_count = 0;
        strcpy(fs->current_directory, "/home");
        init_superblock(); // Initialiser le superbloc
        save_filesystem(fs);
    } else {
        fread(fs, sizeof(Filesystem), 1, file);
        fread(&sb, sizeof(superblock), 1, file);  // Charger le superbloc
        fread(block_data, sizeof(block_data), 1, file);  // Charger les blocs de données
        fclose(file);
    }
}

// Fonction pour afficher le nombre de blocs libres
void print_free_blocks() {
    int free_blocks = 0;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (sb.free_blocks[i] == 1) {
            free_blocks++;
        }
    }
    printf("Blocs libres disponibles : %d/%d\n", free_blocks, NUM_BLOCKS);
}

// Fonction pour réinitialiser le système de fichiers
void reset_filesystem(Filesystem *fs) {
    fs->inode_count = 0;
    strcpy(fs->current_directory, "/home");
    init_superblock(); // Réinitialiser les blocs libres
    save_filesystem(fs);
    printf("Système de fichiers réinitialisé.\n");
}

// Fonction pour créer un répertoire
void create_directory(Filesystem *fs, const char *dirname) {
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return;
    }

    char path[MAX_FILENAME * 2];
    snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);

    strcpy(fs->inodes[fs->inode_count].name, path);
    fs->inodes[fs->inode_count].is_directory = 1;
    fs->inodes[fs->inode_count].size = 0;
    fs->inode_count++;

    save_filesystem(fs);
    printf("Répertoire '%s' créé.\n", path);
}

// Fonction pour supprimer un répertoire
void delete_directory(Filesystem *fs, const char *dirname) {
    char path[MAX_FILENAME * 2];
    snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);

    if (strncmp(dirname, "/home/", strlen("/home/")) == 0) {  
        snprintf(path, sizeof(path), "%s", dirname);
    } else {
        snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);
    }

    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, path) == 0 && fs->inodes[i].is_directory) {
            for (int j = i; j < fs->inode_count - 1; j++) {
                fs->inodes[j] = fs->inodes[j + 1];
            }
            fs->inode_count--;
            save_filesystem(fs);
            printf("Répertoire '%s' supprimé.\n", path);
            return;
        }
    }
    printf("Répertoire '%s' introuvable !\n", path);
}

// Fonction pour changer de répertoire
void change_directory(Filesystem *fs, const char *dirname) {
    if (strcmp(dirname, "..") == 0) {
        if (strcmp(fs->current_directory, "/home") == 0) {
            printf("Vous êtes déjà dans /home.\n");
            return;
        }
        char *last_slash = strrchr(fs->current_directory, '/');
        if (last_slash) {
            *last_slash = '\0';
        }
        printf("Déplacé dans '%s'.\n", fs->current_directory);
        return;
    }

    char path[MAX_FILENAME * 2];
    snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);

    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, path) == 0 && fs->inodes[i].is_directory) {
            strcpy(fs->current_directory, path);
            printf("Déplacé dans '%s'.\n", fs->current_directory);
            return;
        }
    }
    printf("Répertoire '%s' introuvable !\n", path);
}

// Fonction pour créer un fichier
void create_file(Filesystem *fs, const char *filename, size_t size, const char *owner) {
    strcpy(permissions, "rw-r--r--"); // Permissions par défaut

    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);

    // Vérifie si un fichier existe déjà dans le répertoire courant
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0) {
            printf("Le fichier existe déjà !\n");
            return;
        }
    }

    // Crée un nouvel inode pour le fichier
    strcpy(fs->inodes[fs->inode_count].name, full_path);
    fs->inodes[fs->inode_count].size = size;
    fs->inodes[fs->inode_count].is_directory = 0; // Ce n'est pas un répertoire

    // Initialisation des métadonnées
    time_t now = time(NULL); // Récupère l'heure actuelle
    fs->inodes[fs->inode_count].creation_time = now;
    fs->inodes[fs->inode_count].modification_time = now;
    strncpy(fs->inodes[fs->inode_count].owner, owner, MAX_FILENAME);
    strncpy(fs->inodes[fs->inode_count].permissions, permissions, 10);

    fs->inode_count++;
    
    save_filesystem(fs);
    printf("Fichier '%s' créé (%zu octets).\n", full_path, size);
}

// Fonction pour lister le contenu du répertoire courant
void list_directory(Filesystem *fs) {
    printf("Contenu de '%s':\n", fs->current_directory);

    int found = 0;
    for (int i = 0; i < fs->inode_count; i++) {
        char *item_name = strrchr(fs->inodes[i].name, '/');
        if (item_name) {
            item_name++; // Ignorer le '/'
        } else {
            item_name = fs->inodes[i].name;
        }

        char parent_path[MAX_FILENAME * 2];
        snprintf(parent_path, sizeof(parent_path), "%s/%s", fs->current_directory, item_name);

        if (strcmp(fs->inodes[i].name, parent_path) == 0) {
            if (fs->inodes[i].is_directory) {
                printf("[DIR]  %s/\n", item_name);
            } else {
                printf("[FILE] %s (%d octets)\n", item_name, fs->inodes[i].size);
            }
            found = 1;
        }
    }

    if (!found) {
        printf("Le répertoire est vide.\n");
    }
}

// Fonction pour afficher les métadonnées d'un fichier
void show_file_metadata(Filesystem *fs, const char *filename) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);

    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0) {
            printf("Métadonnées de '%s':\n", full_path);
            printf("  Propriétaire: %s\n", fs->inodes[i].owner);
            printf("  Permissions: %s\n", fs->inodes[i].permissions);

            char creation_time[100];
            char modification_time[100];

            // Formater les dates pour un affichage lisible
            strftime(creation_time, sizeof(creation_time), "%Y-%m-%d %H:%M:%S", localtime(&fs->inodes[i].creation_time));
            strftime(modification_time, sizeof(modification_time), "%Y-%m-%d %H:%M:%S", localtime(&fs->inodes[i].modification_time));

            printf("  Date de création: %s\n", creation_time);
            printf("  Date de modification: %s\n", modification_time);
            printf("  Taille: %d octets\n", fs->inodes[i].size);
            return;
        }
    }
    printf("Fichier '%s' introuvable.\n", full_path);
}

// Fonction pour modifier les permissions d'un fichier
void chmod_file(Filesystem *fs, const char *filename, const char *target, const char *new_permissions) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);

    // Parcourir tous les inodes pour trouver le fichier correspondant
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0) {
            // Gérer les permissions en fonction de la cible
            if (strcmp(target, "-Owner") == 0) {
                if (strlen(new_permissions) == 3) {
                    strncpy(fs->inodes[i].permissions, new_permissions, 3);  // Mettre à jour les permissions du propriétaire
                    printf("Permissions de '%s' pour le propriétaire mises à jour en '%s'.\n", filename, new_permissions);
                } else {
                    printf("Les permissions du propriétaire doivent être exactement 3 caractères (rwx).\n");
                }
            } else if (strcmp(target, "-Group") == 0) {
                if (strlen(new_permissions) == 3) {
                    strncpy(fs->inodes[i].permissions + 3, new_permissions, 3);  // Mettre à jour les permissions du groupe
                    printf("Permissions de '%s' pour le groupe mises à jour en '%s'.\n", filename, new_permissions);
                } else {
                    printf("Les permissions du groupe doivent être exactement 3 caractères (rwx).\n");
                }
            } else if (strcmp(target, "-Others") == 0) {
                if (strlen(new_permissions) == 3) {
                    strncpy(fs->inodes[i].permissions + 6, new_permissions, 3);  // Mettre à jour les permissions des autres
                    printf("Permissions de '%s' pour les autres mises à jour en '%s'.\n", filename, new_permissions);
                } else {
                    printf("Les permissions des autres doivent être exactement 3 caractères (rwx).\n");
                }
            } else {
                printf("Option '%s' inconnue !\n", target);
                return;
            }

            // Mettre à jour la date de modification
            fs->inodes[i].modification_time = time(NULL);
            save_filesystem(fs);
            return;
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Fichier '%s' introuvable !\n", filename);
}

// Fonction pour obtenir un inode par son nom
Inode* get_inode_by_name(Filesystem *fs, const char *filename) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);

    // Parcourir tous les inodes pour trouver celui qui correspond au nom du fichier
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0) {
            return &fs->inodes[i];  // Retourner l'inode du fichier trouvé
        }
    }

    // Si le fichier n'est pas trouvé, retourner NULL
    printf("Fichier '%s' introuvable.\n", filename);
    return NULL;
}

// Fonction pour allouer un bloc de données
int allocate_block() {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (sb.free_blocks[i] == 1) { // 1 = libre
            sb.free_blocks[i] = 0; // Marquer le bloc comme occupé
            return i; // Retourner l'index du bloc alloué
        }
    }
    return -1; // Aucun bloc libre disponible
}

// Fonction pour compter les blocs libres
int count_free_blocks() {
    int count = 0;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (sb.free_blocks[i] == 1) {
            count++;
        }
    }
    return count;
}

// Fonction pour écrire du contenu dans un fichier
void write_to_file(Filesystem *fs, const char *filename, const char *content) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);

    // Rechercher le fichier dans les inodes
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0 && !fs->inodes[i].is_directory) {
            // Vérifier si l'utilisateur a les permissions d'écriture
            if (strstr(fs->inodes[i].permissions, "w") == NULL) {
                printf("Permission refusée : vous n'avez pas les droits d'écriture sur ce fichier.\n");
                return;
            }

            // Calculer le nombre de blocs nécessaires
            size_t content_size = strlen(content);
            int blocks_needed = (content_size / BLOCK_SIZE) + 1;

            // Vérifier s'il y a suffisamment de blocs libres
            if (count_free_blocks() < blocks_needed) {
                printf("Espace insuffisant : %d blocs nécessaires, %d blocs libres.\n", blocks_needed, count_free_blocks());
                return;
            }

            // Allouer les blocs et écrire le contenu
            for (int j = 0; j < blocks_needed; j++) {
                int block_index = allocate_block();
                if (block_index == -1) {
                    printf("Erreur d'allocation de bloc.\n");
                    return;
                }
                fs->inodes[i].block_indices[fs->inodes[i].block_count++] = block_index; // Associer le bloc au fichier
                size_t offset = j * BLOCK_SIZE;
                size_t bytes_to_copy = (content_size - offset) < BLOCK_SIZE ? (content_size - offset) : BLOCK_SIZE;
                strncpy(block_data[block_index], content + offset, bytes_to_copy);
            }

            // Mettre à jour la taille du fichier
            fs->inodes[i].size = content_size;

            // Mettre à jour la date de modification
            fs->inodes[i].modification_time = time(NULL);

            // Sauvegarder le système de fichiers
            save_filesystem(fs);
            printf("Contenu écrit dans le fichier '%s'.\n", filename);
            return;
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Fichier '%s' introuvable ou est un répertoire.\n", filename);
}

// Fonction pour lire le contenu d'un fichier
void read_file(Filesystem *fs, const char *filename) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);

    // Rechercher le fichier dans les inodes
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0 && !fs->inodes[i].is_directory) {
            // Vérifier si l'utilisateur a les permissions de lecture
            if (strstr(fs->inodes[i].permissions, "r") == NULL) {
                printf("Permission refusée : vous n'avez pas les droits de lecture sur ce fichier.\n");
                return;
            }

            // Afficher le contenu du fichier
            printf("Contenu du fichier '%s':\n", filename);
            for (int j = 0; j < fs->inodes[i].block_count; j++) {
                int block_index = fs->inodes[i].block_indices[j];
                printf("%s", block_data[block_index]);
            }
            printf("\n");
            return;
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Fichier '%s' introuvable ou est un répertoire.\n", filename);
}

// Fonction pour supprimer un fichier
void delete_file(Filesystem *fs, const char *filename) {
    char full_path[MAX_FILENAME * 2];

    if (strncmp(filename, "/home/", strlen("/home/")) == 0) {  
        snprintf(full_path, sizeof(full_path), "%s", filename);
    } else {
        snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);
    }

    // Rechercher le fichier dans les inodes
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0 && !fs->inodes[i].is_directory) {
            // Libérer les blocs de données associés au fichier
            for (int j = 0; j < fs->inodes[i].block_count; j++) {
                int block_index = fs->inodes[i].block_indices[j];
                sb.free_blocks[block_index] = 1; // Marquer le bloc comme libre
            }

            // Déplacer les inodes suivants pour combler l'espace
            for (int j = i; j < fs->inode_count - 1; j++) {
                fs->inodes[j] = fs->inodes[j + 1];
            }

            // Décrémenter le nombre d'inodes
            fs->inode_count--;

            // Sauvegarder le système de fichiers
            save_filesystem(fs);
            printf("Fichier '%s' supprimé.\n", filename);
            return;
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Fichier '%s' introuvable ou est un répertoire.\n", filename);
}

// Fonction pour vérifier si un répertoire existe
int directory_exists(Filesystem *fs, const char *path) {
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, path) == 0 && fs->inodes[i].is_directory) {
            return 1; // Le répertoire existe
        }
    }
    return 0; // Le répertoire n'existe pas
}

// Fonction pour copier un fichier
void copy_file(Filesystem *fs, const char *filenamedepart, const char *filenamefinal, const char *nomrepertoire) {
    char full_path_source[MAX_FILENAME * 2];
    char full_path_dest[MAX_FILENAME * 2];
    char dest_directory[MAX_FILENAME * 2];

    // Construire le chemin complet du fichier source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, filenamedepart);

    // Vérifier si le nomrepertoire est un chemin complet ou un répertoire relatif
    if (strchr(nomrepertoire, '/') != NULL) {
        // C'est un chemin complet
        if (!directory_exists(fs, nomrepertoire)) {
            printf("Le répertoire '%s' n'existe pas.\n", nomrepertoire);
            return;
        }
        snprintf(full_path_dest, sizeof(full_path_dest), "%s/%s", nomrepertoire, filenamefinal);
    } else {
        // C'est un répertoire relatif au répertoire courant
        snprintf(dest_directory, sizeof(dest_directory), "%s/%s", fs->current_directory, nomrepertoire);
        if (!directory_exists(fs, dest_directory)) {
            printf("Le répertoire '%s' n'existe pas dans le répertoire courant.\n", nomrepertoire);
            return;
        }
        snprintf(full_path_dest, sizeof(full_path_dest), "%s/%s/%s", fs->current_directory, nomrepertoire, filenamefinal);
    }

    // Rechercher le fichier source dans les inodes
    Inode *source_inode = NULL;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && !fs->inodes[i].is_directory) {
            source_inode = &fs->inodes[i];
            break;
        }
    }

    if (!source_inode) {
        printf("Fichier source '%s' introuvable ou est un répertoire.\n", filenamedepart);
        return;
    }

    // Vérifier si le fichier de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_dest) == 0) {
            printf("Le fichier de destination '%s' existe déjà.\n", filenamefinal);
            return;
        }
    }

    // Créer un nouvel inode pour le fichier de destination
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return;
    }

    // Copier les métadonnées du fichier source
    Inode *dest_inode = &fs->inodes[fs->inode_count];
    strcpy(dest_inode->name, full_path_dest);
    dest_inode->is_directory = 0;
    dest_inode->size = source_inode->size;
    dest_inode->creation_time = time(NULL);
    dest_inode->modification_time = time(NULL);
    strncpy(dest_inode->owner, source_inode->owner, MAX_FILENAME);
    strncpy(dest_inode->permissions, source_inode->permissions, 10);

    // Allouer des blocs pour le fichier de destination
    dest_inode->block_count = 0;
    for (int i = 0; i < source_inode->block_count; i++) {
        int block_index = allocate_block();
        if (block_index == -1) {
            printf("Erreur d'allocation de bloc pour la copie.\n");
            return;
        }
        dest_inode->block_indices[dest_inode->block_count++] = block_index;

        // Copier le contenu du bloc source vers le bloc de destination
        strncpy(block_data[block_index], block_data[source_inode->block_indices[i]], BLOCK_SIZE);
    }

    // Incrémenter le nombre d'inodes
    fs->inode_count++;

    // Sauvegarder le système de fichiers
    save_filesystem(fs);
    printf("Fichier '%s' copié vers '%s'.\n", filenamedepart, full_path_dest);
}

// Fonction pour déplacer un fichier
void move_file(Filesystem *fs, const char *filename, const char *nomrepertoire) {
    char full_path_source[MAX_FILENAME * 2];
    char full_path_dest[MAX_FILENAME * 2];
    char dest_directory[MAX_FILENAME * 2];

    // Construire le chemin complet du fichier source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, filename);

    // Vérifier si le nomrepertoire est un chemin complet ou un répertoire relatif
    if (strchr(nomrepertoire, '/') != NULL) {
        // C'est un chemin complet
        if (!directory_exists(fs, nomrepertoire)) {
            printf("Le répertoire '%s' n'existe pas.\n", nomrepertoire);
            return;
        }
        snprintf(full_path_dest, sizeof(full_path_dest), "%s/%s", nomrepertoire, filename);
    } else {
        // C'est un répertoire relatif au répertoire courant
        snprintf(dest_directory, sizeof(dest_directory), "%s/%s", fs->current_directory, nomrepertoire);
        if (!directory_exists(fs, dest_directory)) {
            printf("Le répertoire '%s' n'existe pas dans le répertoire courant.\n", nomrepertoire);
            return;
        }
        snprintf(full_path_dest, sizeof(full_path_dest), "%s/%s/%s", fs->current_directory, nomrepertoire, filename);
    }

    // Rechercher le fichier source dans les inodes
    Inode *source_inode = NULL;
    int source_index = -1;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && !fs->inodes[i].is_directory) {
            source_inode = &fs->inodes[i];
            source_index = i;
            break;
        }
    }

    if (!source_inode) {
        printf("Fichier source '%s' introuvable ou est un répertoire.\n", filename);
        return;
    }

    // Vérifier si le fichier de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_dest) == 0) {
            printf("Le fichier de destination '%s' existe déjà.\n", filename);
            return;
        }
    }

    // Créer un nouvel inode pour le fichier de destination
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return;
    }

    // Copier les métadonnées du fichier source
    Inode *dest_inode = &fs->inodes[fs->inode_count];
    strcpy(dest_inode->name, full_path_dest);
    dest_inode->is_directory = 0;
    dest_inode->size = source_inode->size;
    dest_inode->creation_time = source_inode->creation_time;
    dest_inode->modification_time = time(NULL); // Mettre à jour la date de modification
    strncpy(dest_inode->owner, source_inode->owner, MAX_FILENAME);
    strncpy(dest_inode->permissions, source_inode->permissions, 10);

    // Copier les blocs de données
    dest_inode->block_count = source_inode->block_count;
    for (int i = 0; i < source_inode->block_count; i++) {
        dest_inode->block_indices[i] = source_inode->block_indices[i];
    }

    // Incrémenter le nombre d'inodes
    fs->inode_count++;

    // Supprimer le fichier source
    for (int i = source_index; i < fs->inode_count - 1; i++) {
        fs->inodes[i] = fs->inodes[i + 1];
    }
    fs->inode_count--;

    // Sauvegarder le système de fichiers
    save_filesystem(fs);
    printf("Fichier '%s' déplacé vers '%s'.\n", filename, full_path_dest);
}

// Fonction pour extraire le chemin relatif
char* extract_path(const char* full_path) {
    const char* prefix = "/home/";
    size_t prefix_len = strlen(prefix);

    if (strncmp(full_path, prefix, prefix_len) == 0) {
        return (char*)(full_path + prefix_len);
    }
    return (char*)full_path;
}

// Fonction pour copier un répertoire
void copy_repertoire(Filesystem *fs, const char *repertoirenamedepart, const char *repertoirenamefinal, const char *nomrepertoire) {
    char full_path_source[MAX_FILENAME * 2];
    char full_path_dest[MAX_FILENAME * 2];
    char dest_directory[MAX_FILENAME * 2];

    // Construire le chemin complet du répertoire source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, repertoirenamedepart);

    // Vérifier si le répertoire source existe
    Inode *source_inode = NULL;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && fs->inodes[i].is_directory) {
            source_inode = &fs->inodes[i];
            break;
        }
    }

    if (!source_inode) {
        printf("Répertoire source '%s' introuvable ou n'est pas un répertoire.\n", full_path_source);
        return;
    }

    // Vérifier si le nomrepertoire est un chemin complet ou un répertoire relatif
    if (strchr(nomrepertoire, '/') != NULL) {
        // C'est un chemin complet
        if (!directory_exists(fs, nomrepertoire)) {
            printf("Le répertoire '%s' n'existe pas.\n", nomrepertoire);
            return;
        }
        snprintf(full_path_dest, sizeof(full_path_dest), "%s/%s", nomrepertoire, repertoirenamefinal);
    } else {
        // C'est un répertoire relatif au répertoire courant
        snprintf(dest_directory, sizeof(dest_directory), "%s/%s", fs->current_directory, nomrepertoire);
        if (!directory_exists(fs, dest_directory)) {
            printf("Le répertoire '%s' n'existe pas dans le répertoire courant.\n", nomrepertoire);
            return;
        }
        snprintf(full_path_dest, sizeof(full_path_dest), "%s/%s/%s", fs->current_directory, nomrepertoire, repertoirenamefinal);
    }

    // Vérifier si le répertoire de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_dest) == 0) {
            printf("Le répertoire de destination '%s' existe déjà.\n", full_path_dest);
            return;
        }
    }

    // Créer un nouvel inode pour le répertoire de destination
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return;
    }

    // Copier les métadonnées du répertoire source
    Inode *dest_inode = &fs->inodes[fs->inode_count];
    strcpy(dest_inode->name, full_path_dest);
    dest_inode->is_directory = 1;
    dest_inode->size = source_inode->size;
    dest_inode->creation_time = time(NULL);
    dest_inode->modification_time = time(NULL);
    strncpy(dest_inode->owner, source_inode->owner, MAX_FILENAME);
    strncpy(dest_inode->permissions, source_inode->permissions, 10);

    // Incrémenter le nombre d'inodes
    fs->inode_count++;

    // Parcourir tous les fichiers et sous-répertoires du répertoire source
    for (int i = 0; i < fs->inode_count; i++) {
        if (strstr(fs->inodes[i].name, full_path_source) == fs->inodes[i].name) {
            // Construire le chemin relatif
            char relative_path[MAX_FILENAME * 2];
            strncpy(relative_path, fs->inodes[i].name + strlen(full_path_source), MAX_FILENAME * 2);

            // Construire le chemin de destination
            char new_path[MAX_FILENAME * 4];
            snprintf(new_path, sizeof(new_path), "%s%s", full_path_dest, relative_path);

            // Copier le fichier ou répertoire
            if (fs->inodes[i].is_directory) {
                create_directory(fs, extract_path(new_path));
            } else {
                // Copier le fichier
                int taille = BLOCK_SIZE * fs->inodes[i].block_count;
                char *content = (char *)malloc(taille);
                if (!content) {
                    printf("Échec de l'allocation de mémoire pour le contenu du fichier !\n");
                    return; // Gérer l'échec de l'allocation de mémoire
                }

                // Initialiser le buffer de contenu
                memset(content, 0, taille);

                // Copier le contenu du fichier
                for (int j = 0; j < fs->inodes[i].block_count; j++) {
                    strncat(content, block_data[fs->inodes[i].block_indices[j]], BLOCK_SIZE);
                }
                create_file(fs, extract_path(new_path), fs->inodes[i].size, fs->inodes[i].owner);
                write_to_file(fs, extract_path(new_path), content);
            }
        }
    }
    delete_directory(fs, full_path_dest);

    // Sauvegarder le système de fichiers
    save_filesystem(fs);
    printf("Répertoire '%s' copié vers '%s'.\n", full_path_source, full_path_dest);
}

// Fonction pour afficher l'aide
void help() {
    printf("Commandes disponibles :\n");
    printf("  help.........................Affiche cette aide.\n");
    printf("  exit.........................Quitte le shell.\n");
    printf("  pwd..........................Affiche le répertoire courant.\n");
    printf("  mkdir <nom>..................Crée un répertoire.\n");
    printf("  rmdir <nom>..................Supprime un répertoire.\n");
    printf("  cpdir <src> <dest> <rep>.....Copie un répertoire vers un répertoire.\n");
    printf("  mvdir <src> <rep>............Déplace un répertoire vers un répertoire.\n");
    printf("  cd <nom>.....................Change de répertoire.\n");
    printf("  ls...........................Liste le contenu du répertoire courant.\n");
    printf("  touch <nom> <taille>.........Crée un fichier vide.\n");
    printf("  stat <nom>...................Affiche les métadonnées d'un fichier.\n");
    printf("  chmod <nom> <cible> <perm>...Modifie les permissions d'un fichier.\n");
    printf("  write <nom> <contenu>........Écrit du contenu dans un fichier.\n");
    printf("  cat <nom>....................Affiche le contenu d'un fichier.\n");
    printf("  cp <src> <dest> <rep>........Copie un fichier vers un répertoire.\n");
    printf("  mv <src> <rep>...............Déplace un fichier vers un répertoire.\n");
    printf("  reset........................Réinitialise le système de fichiers.\n");
}

// Fonction principale du shell
void shell(Filesystem *fs, char *current_own) {
    char command[100];

    printf("\nBienvenue dans le système de fichiers %s!\n", current_own);
    printf("Répertoire actuel: %s\n", fs->current_directory);

    while (current_own) {
        printf("\n%s> ", fs->current_directory);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Supprimer le saut de ligne

        if (strncmp(command, "exit", 4) == 0) {
            printf("Arrêt du système de fichiers.\n");
            break;
        } else if (strncmp(command, "help", 4) == 0) {
            help();
        } else if (strncmp(command, "pwd", 3) == 0) {
            printf("%s\n", fs->current_directory);
        } else if (strncmp(command, "mkdir ", 6) == 0) {
            create_directory(fs, command + 6);
        } else if (strncmp(command, "rmdir ", 6) == 0) {
            delete_directory(fs, command + 6);
        } else if (strncmp(command, "cd ", 3) == 0) {
            change_directory(fs, command + 3);
        } else if (strncmp(command, "ls", 2) == 0) {
            list_directory(fs);
        } else if (strncmp(command, "touch ", 6) == 0) {
            char filename[MAX_FILENAME];
            int size;
            sscanf(command + 6, "%s %d", filename, &size);
            create_file(fs, filename, size, current_own);
        } else if (strncmp(command, "stat ", 5) == 0) {
            show_file_metadata(fs, command + 5);
        } else if (strncmp(command, "chmod ", 6) == 0) {
            char filename[MAX_FILENAME];
            char target[10];
            char new_permissions[4];
            sscanf(command + 6, "%s %s %s", filename, target, new_permissions);
            chmod_file(fs, filename, target, new_permissions);
        } else if (strncmp(command, "write ", 6) == 0) {
            char filename[MAX_FILENAME];
            char content[MAX_FILENAME * 2];
            sscanf(command + 6, "%s %[^\n]", filename, content);
            write_to_file(fs, filename, content);
        } else if (strncmp(command, "cat ", 4) == 0) {
            read_file(fs, command + 4);
        } else if (strncmp(command, "reset", 5) == 0) {
            reset_filesystem(fs);
        } else if (strncmp(command, "rm ", 3) == 0) {
            delete_file(fs, command + 3);
        } else if (strncmp(command, "cp ", 3) == 0) {
            char filenamedepart[MAX_FILENAME];
            char filenamefinal[MAX_FILENAME];
            char repertoire[MAX_FILENAME];
            sscanf(command + 3, "%s %s %s", filenamedepart, filenamefinal, repertoire);
            copy_file(fs, filenamedepart, filenamefinal, repertoire);
        } else if (strncmp(command, "mv ", 3) == 0) {
            char filename[MAX_FILENAME];
            char nomrepertoire[MAX_FILENAME];
            sscanf(command + 3, "%s %s", filename, nomrepertoire);
            move_file(fs, filename, nomrepertoire);
        } else if (strncmp(command, "cpdir ", 6) == 0) {
            char dirnamedepart[MAX_FILENAME];
            char direnamefinal[MAX_FILENAME];
            char repertoire[MAX_FILENAME];
            sscanf(command + 6, "%s %s %s", dirnamedepart, direnamefinal, repertoire);
            copy_repertoire(fs, dirnamedepart, direnamefinal, repertoire);
        } else {
            printf("Commande inconnue !\n");
        }
    }
}

// Fonction principale
int main() {
    setlocale(LC_ALL, "en_US.UTF-8"); // Pour gérer les caractères spéciaux
    Filesystem fs;
    init_filesystem(&fs); // Initialiser le système de fichiers
    printf("\nEntrez votre nom: ");
    scanf("%s", current_own);

    // Vider le buffer d'entrée après scanf
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    shell(&fs, current_own); // Lancer le shell
    return 0;
}
