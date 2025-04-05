#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h> // Pour setlocale (gestion des caractères spéciaux)
#include <sys/types.h> // Pour ino_t et autres types POSIX
#include <time.h>   // Pour la gestion du temps (création/modification des fichiers)
//#include <pthread.h> // Pour les threads

// Définition des constantes
#define MAX_FILES 1000                      // Nombre maximal de fichiers
#define MAX_FILENAME 50                     // Taille maximale du nom de fichier
#define MAX_DIRECTORY 50                    // Taille maximale du nom de répertoire
#define FILESYSTEM_FILE "my_filesystem.dat" // Nom du fichier contenant le système de fichiers
#define TRACE_FILE "trace_execution.txt"    // Nom du fichier contenant les traces d'exécution
#define NAME_SIZE 10                        // Taille du nom d'utilisateur
#define PERM_SIZE 11                        // Taille des permissions
#define BLOCK_SIZE 512                      // Taille d'un bloc de données
#define NUM_BLOCKS 1024                     // Nombre total de blocs
#define NUM_INODES 128                      // Nombre total d'inodes
#define GROUP_SIZE 10                       // Taille de groupe par personne
#define NUM_USER 20                         // Nombre total d'utilisateurs
#define FILE_SIZE 12                        // Taille du fichier par défaut
#define MAX_CONTENT 100                     // Taille maximale du contenu
#define NUM_LIEN_MAX 10                     // Nombre maximal de liens
#define MAX_PATH 50                         // Taille maximale du chemin
#define GROUP_FILE "./users/groups"         // Répertoire des groupes
#define MAX_PASSWORD 50                     // Taille maximale du mot de passe
#define MAX_HISTORY 100                     // Nombre maximum de commandes dans l'historique
#define MAX_CMD_LENGTH 100                  // Longueur maximale d'une commande

typedef struct {
    char commands[MAX_HISTORY][MAX_CMD_LENGTH];
    int count;
    int current;  // Pour la navigation dans l'historique
} CommandHistory;

// Définition de la structure d'un tableau
typedef struct{
    char data[50];  // Tableau de chaîne de caractères
} Tab;

// Structure représentant un lien symbolique ou physique
typedef struct {
    Tab hardLink[NUM_LIEN_MAX];      // Tableau de liens physiques
    Tab symbolicLink[NUM_LIEN_MAX];  // Tableau de liens symboliques
} Lien;

// Structure représentant un inode (métadonnées d'un fichier ou répertoire)
typedef struct {
    ino_t inode_number;               // Numéro d'inode unique
    ino_t parent_inode_number;        // Numéro d'inode parent (pour les répertoires)
    char name[MAX_FILENAME];          // Nom du fichier ou du répertoire
    int is_directory;                 // Indicateur si c'est un répertoire (1) ou un fichier (0)
    int is_link;                      // Indicateur si c'est un lien (1) ou non (0)
    int is_group;                     // Indicateur si c'est un groupe 
    int is_file;                      // Indicateur si c'est un fichier (1) ou non (0)    
    int size;                         // Taille du fichier en octets
    char group[GROUP_SIZE];           // Groupe associé fichier ou du répertoire
    time_t creation_time;             // Date de création
    time_t modification_time;         // Date de la dernière modification
    char owner[MAX_FILENAME];         // Propriétaire du fichier
    char permissions[PERM_SIZE];      // Permissions du fichier (ex: "-rwxr-xr--"),du reperdtoire (ex: "drwxr-xr--")
    int block_indices[NUM_BLOCKS];    // Indices des blocs alloués
    int block_count;                  // Nombre de blocs alloués
    int num_liens;                    // Nombre de liens physiques
    Lien lien;                        // Lien symbolique et physique
} Inode;

// Structure pour associer une personne à un Group
typedef struct {
    char user[NAME_SIZE];        // Nom du l'utilisateur
    Tab group[GROUP_SIZE];       // Groupe associé a l'utilisateur
    int taille;                  // Taille du groupe
    char password[MAX_PASSWORD]; // Mot de passe de l'utilisateur
    int is_root;                    // Indicateur si l'utilisateur est root (1) ou non (0)
    int is_admin;                // Indicateur si l'utilisateur est admin (1) ou non (0)
    char root_pwd[MAX_PASSWORD]; // Mot de passe root
} User_Group;

// Structure représentant le superbloc (métadonnées du système de fichiers)
typedef struct {
    ino_t next_inode_number;  // Prochain numéro disponible
    int num_blocks;               // Nombre total de blocs
    int num_inodes;               // Nombre total d'inodes
    int free_blocks[NUM_BLOCKS];  // Tableau des blocs libres (1 = libre, 0 = occupé)
    int free_inodes[NUM_INODES];  // Tableau des inodes libres (1 = libre, 0 = occupé)
    Inode inodes[NUM_INODES];     // Table des inodes
} superblock;

// Structure représentant le système de fichiers
typedef struct {
    Inode inodes[MAX_FILES];          // Table des inodes
    User_Group group[NUM_USER];       // Table des groupe
    int inode_count;                  // Nombre d'inodes utilisés
    int user_count;                  // Nombre de groupes utilisés
    char current_directory[MAX_PATH]; // Répertoire actuel
    CommandHistory history;           // Historique des commandes
} Filesystem;

// Structure pour associer un nom de fichier à un inode
typedef struct {
    char name[50];      // Nom du fichier
    int inode_index;    // Index de l'inode associé
} file_entry;

// Variables globales
superblock sb; 
char block_data[NUM_BLOCKS][BLOCK_SIZE]; // Tableau pour stocker les données des fichiers
file_entry file_table[NUM_INODES];       // Table des noms de fichiers

char current_own[NAME_SIZE];     // Utilisateur actuel
char current_group[GROUP_SIZE];  // Utilisateur actuel
char permissions[PERM_SIZE];     // Permissions par défaut
int sudo = 0;                    // Indicateur pour le mode super utilisateur
//pthread_mutex_t fs_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex pour la synchronisation


void save_filesystem(Filesystem *fs);
int delete_file(Filesystem *fs, const char *filename);
int calculate_directory_size_recursive(Filesystem *fs, const char *dirpath);
int create_directory(Filesystem *fs, const char *dirname, const char *destname);
int delete_directory(Filesystem *fs, const char *dirname);
int user_add_group(Filesystem *fs, const char *groupname);
Inode* get_inode_by_name(Filesystem *fs, const char *filename);
char* extract_path(const char* full_path);
char *retirer_suffixe(char *str);
char* last_element(const char* full_path);
int reset_user_workspace(Filesystem *fs, const char *username);
int create_directory_group(Filesystem *fs, const char *dirname);
int create_directory_home(Filesystem *fs, const char *dirname, const char *destname);

// Fonction pour créer un groupe dans ./user/groups s'il n'existe pas
int create_group_directory(Filesystem *fs, const char *groupname) {
    memset(current_group, '\0', sizeof(current_group));
    char group_path[MAX_FILENAME * 2];
    snprintf(group_path, sizeof(group_path), "./users/groups/%s", groupname);

    // Vérifier si le répertoire du groupe existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, group_path) == 0 && fs->inodes[i].is_directory) {
            printf("Le répertoire du groupe '%s' existe déjà.\n", groupname);
            return 0; // Le répertoire existe déjà
        }
    }

    // Sauvegarder le répertoire courant
    char current_dir_backup[MAX_PATH];
    strcpy(current_dir_backup, fs->current_directory);

    // Créer le répertoire du groupe
    strcpy(fs->current_directory, "./users/groups");
    create_directory_group(fs, groupname);

    // Restaurer le répertoire courant
    strcpy(fs->current_directory, current_dir_backup);
    user_add_group(fs,groupname);

    printf("Répertoire du groupe '%s' créé dans ./users/groups\n", groupname);
    return 1; // Le répertoire a été créé avec succès
}

// Fonction pour créer un groupe
int user_add_group(Filesystem *fs, const char *groupname) {
    if (groupname == NULL || strlen(groupname) == 0) {
        printf("Erreur : nom de groupe invalide.\n");
        return 0;
    }

    if (strcmp(current_own, groupname) == 1) {
        // Créer le répertoire du groupe s'il n'existe pas
        create_group_directory(fs, groupname);
    }

    // Trouver l'utilisateur actuel dans la table des groupes
    int user_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Erreur : utilisateur '%s' introuvable.\n", current_own);
        return 0;
    }

    // Vérifier si le groupe existe déjà pour l'utilisateur
    for (int j = 0; j < fs->group[user_index].taille; j++) {
        if (strcmp(fs->group[user_index].group[j].data, groupname) == 0) {
            printf("Le groupe '%s' existe déjà pour l'utilisateur '%s'.\n", groupname, current_own);
            if (strlen(current_group) == 0) {
                strncpy(current_group, groupname, strlen(groupname));
                printf("Le groupe actuel est'%s'.\n", groupname);
            }
            return 0; // Le groupe existe déjà
        }
    }

    // Ajouter le groupe si l'utilisateur a de la place
    if (fs->group[user_index].taille < GROUP_SIZE) {
        strncpy(fs->group[user_index].group[fs->group[user_index].taille].data, groupname, GROUP_SIZE);
        fs->group[user_index].taille++;
        printf("Groupe '%s' ajouté à l'utilisateur '%s'.\n", groupname, current_own);
        save_filesystem(fs);
        strncpy(current_group, groupname, strlen(groupname));
        if (strlen(current_group) == 0) {
            strncpy(current_group, groupname, strlen(groupname));
            return 1; // Le groupe a été ajouté avec succès
        }
    } else {
        printf("Erreur : l'utilisateur '%s' a déjà atteint le nombre maximal de groupes.\n", current_own);
        return 0; // Nombre maximal de groupes atteint
    }
    return 0;
}

// Fonction pour quitter un groupe avec gestion du propriétaire
int leave_group(Filesystem *fs, const char *groupname) {
    // Vérifications de base
    if (strlen(current_own) == 0) {
        printf("Erreur : Aucun utilisateur connecté.\n");
        return 0;
    }

    if (groupname == NULL || strlen(groupname) == 0) {
        printf("Erreur : Nom de groupe invalide.\n");
        return 0;
    }

    // Vérifier si c'est le groupe principal
    if (strcmp(groupname, current_own) == 0) {
        printf("Erreur : Vous ne pouvez pas quitter votre groupe principal '%s'.\n", current_own);
        return 0;
    }

    // Trouver le groupe dans le système de fichiers
    char group_path[MAX_PATH];
    snprintf(group_path, sizeof(group_path), "./users/groups/%s", groupname);
    
    Inode* group_inode = NULL;
    int is_owner = 0;
    
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, group_path) == 0 && fs->inodes[i].is_directory) {
            group_inode = &fs->inodes[i];
            is_owner = (strcmp(group_inode->owner, current_own) == 0);
            break;
        }
    }

    if (group_inode == NULL) {
        printf("Erreur : Groupe '%s' introuvable.\n", groupname);
        return 0;
    }

    // Trouver l'utilisateur courant dans la table des groupes
    int user_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Erreur : Utilisateur introuvable.\n");
        return 0;
    }

    // Vérifier l'appartenance au groupe
    int group_index = -1;
    for (int j = 0; j < fs->group[user_index].taille; j++) {
        if (strcmp(fs->group[user_index].group[j].data, groupname) == 0) {
            group_index = j;
            break;
        }
    }

    if (group_index == -1) {
        printf("Vous ne faites pas partie du groupe '%s'.\n", groupname);
        return 0;
    }

    // Cas particulier : propriétaire du groupe
    if (is_owner) {
        // Compter les membres restants
        int member_count = 0;
        char new_owner[MAX_FILENAME] = "";
        
        for (int i = 0; i < NUM_USER; i++) {
            if (fs->group[i].user[0] != '\0') {
                for (int j = 0; j < fs->group[i].taille; j++) {
                    if (strcmp(fs->group[i].group[j].data, groupname) == 0) {
                        member_count++;
                        if (strcmp(fs->group[i].user, current_own) != 0 && new_owner[0] == '\0') {
                            strncpy(new_owner, fs->group[i].user, MAX_FILENAME);
                        }
                        break;
                    }
                }
            }
        }

        if (member_count <= 1) {
            printf("Vous êtes le dernier membre du groupe '%s'.\n", groupname);
            printf("Options disponibles :\n");
            printf("1. Supprimer complètement le groupe (commande: sudo delgroup %s)\n", groupname);
            printf("2. Annuler et rester dans le groupe\n");
            return 0;
        } else {
            printf("Vous êtes le propriétaire du groupe '%s'.\n", groupname);
            printf("Il reste %d autres membres dans ce groupe.\n", member_count - 1);
            
            if (new_owner[0] != '\0') {
                printf("Voulez-vous transférer la propriété à '%s'? (o/n) ", new_owner);
                
                char response[2];
                fgets(response, sizeof(response), stdin);
                
                if (response[0] == 'o' || response[0] == 'O') {
                    // Transfert de propriété
                    strncpy(group_inode->owner, new_owner, MAX_FILENAME);
                    printf("Propriété du groupe '%s' transférée à '%s'.\n", groupname, new_owner);
                    save_filesystem(fs);
                    return 1;
                } else {
                    printf("Annulation. Vous restez propriétaire du groupe.\n");
                    return 0;
                }
            }
        }
    }

    // Retirer le groupe de la liste de l'utilisateur
    for (int k = group_index; k < fs->group[user_index].taille - 1; k++) {
        strcpy(fs->group[user_index].group[k].data, fs->group[user_index].group[k+1].data);
    }
    fs->group[user_index].taille--;

    // Mise à jour du groupe actuel si nécessaire
    if (strcmp(current_group, groupname) == 0) {
        for (int j = 0; j < fs->group[user_index].taille; j++) {
            if (strcmp(fs->group[user_index].group[j].data, current_own) != 0) {
                strcpy(current_group, fs->group[user_index].group[j].data);
                printf("Groupe actuel changé pour '%s'.\n", current_group);
                break;
            }
        }
    }

    printf("Vous avez quitté le groupe '%s'.\n", groupname);
    save_filesystem(fs);
    return 1; // Succès
}

// Fonction pour initialiser le superbloc
void init_superblock() {
    sb.next_inode_number = 1;  // Commence à 1 (0 peut être réservé)
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
    FILE *trace_file = fopen(TRACE_FILE, "a");
    if (!file && trace_file) {
        printf("Fichier système non trouvé, initialisation...\n");
        fs->inode_count = 0;
        fs->user_count = 0;
        strcpy(fs->current_directory, "\0");  // Chaîne vide
        create_directory_home(fs, ".",NULL);
        strcpy(fs->current_directory, ".");
        init_superblock(); // Initialiser le superbloc
        create_directory_home(fs, "users",NULL); // Créer le répertoire des utilisateurs
        strcpy(fs->current_directory, "./users");
        create_directory_home(fs, "groups",NULL); // Créer le répertoire des groupes
        create_directory_home(fs, "home",NULL); // Créer le répertoire home
        create_directory_home(fs, "local",NULL); // Créer le répertoire local
        strcpy(fs->current_directory, "./users/home");
        save_filesystem(fs);
        printf("Système de fichiers initialisé : %s\n", fs->current_directory);
    } else {
        fread(fs, sizeof(Filesystem), 1, file);
        fread(&sb, sizeof(superblock), 1, file);  // Charger le superbloc
        fread(block_data, sizeof(block_data), 1, file);  // Charger les blocs de données
        fclose(file);
    }
}

// Fonction pour afficher le nombre de blocs libres
int print_free_blocks() {
    int free_blocks = 0;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (sb.free_blocks[i] == 1) {
            free_blocks++;
        }
    }
    printf("Blocs libres disponibles : %d/%d\n", free_blocks, NUM_BLOCKS);
    return free_blocks;
}

// Fonction pour lister tous les liens symboliques pointant vers un fichier
int list_symbolic_links(Filesystem *fs, const char *target_path) {
    printf("Liens symboliques pointant vers '%s':\n", target_path);
    int found = 0;

    for (int i = 0; i < fs->inode_count; i++) {
        for (int j = 0; j < NUM_LIEN_MAX; j++) {
            if (strlen(fs->inodes[i].lien.symbolicLink[j].data) > 0 &&
                strcmp(fs->inodes[i].lien.symbolicLink[j].data, target_path) == 0) {
                printf("- %s\n", fs->inodes[i].name);
                found = 1;
            }
        }
    }

    if (!found) {
        printf("Aucun lien symbolique trouvé.\n");
        return 0;
    }
    return 1;
}

// Fonction pour lister tous les hardlinks d'un fichier
int list_hard_links(Filesystem *fs, const char *target_path) {
    // Trouver l'inode original
    Inode *target_inode = NULL;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, target_path) == 0 && !fs->inodes[i].is_directory && fs->inodes[i].is_link) {
            target_inode = &fs->inodes[i];
            break;
        }
    }

    if (target_inode == NULL) {
        printf("Fichier cible '%s' introuvable.\n", target_path);
        return 0;
    }

    printf("Hardlinks du fichier '%s':\n", target_path);
    printf("- %s (original)\n", target_path); // Le fichier original

    // Parcourir tous les inodes pour trouver les hardlinks
    for (int i = 0; i < fs->inode_count; i++) {
        if (fs->inodes[i].block_count > 0 && 
            fs->inodes[i].block_indices[0] == target_inode->block_indices[0] &&
            strcmp(fs->inodes[i].name, target_path) != 0) {
            printf("- %s\n", fs->inodes[i].name);
            return 1; // Au moins un hardlink trouvé
        }
    }
    return 0; // Aucun hardlink trouvé
}

// Fonction pour créer un répertoire
int create_directory_home(Filesystem *fs, const char *dirname, const char *destname) {
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return 0;
    }
    strcpy(permissions, "drwxrwxrwx");

    // Construire le chemin complet
    char path[MAX_FILENAME * 2];
    if (destname == NULL) {
        if (strcmp(fs->current_directory, "\0") == 0) {
            snprintf(path, sizeof(path), "%s", dirname);
        }     
        else if (strcmp(fs->current_directory, ".") == 0) {
            snprintf(path, sizeof(path), "./%s", dirname);
        }  
        else {
            snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);
        }
    }  
    else {
        snprintf(path, sizeof(path), "%s/%s/%s", fs->current_directory, destname, dirname);
    }

    strcpy(fs->inodes[fs->inode_count].name, path);
    fs->inodes[fs->inode_count].is_directory = 1;
    fs->inodes[fs->inode_count].is_group = 0;
    fs->inodes[fs->inode_count].is_file = 0;
    fs->inodes[fs->inode_count].is_link = 0;
    fs->inodes[fs->inode_count].size = 0;
    fs->inodes[fs->inode_count].inode_number = sb.next_inode_number++;

    // Initialisation des métadonnées
    time_t now = time(NULL); // Récupère l'heure actuelle
    fs->inodes[fs->inode_count].creation_time = now;
    fs->inodes[fs->inode_count].modification_time = now;
    fs->inodes[fs->inode_count].num_liens = 0;

    strncpy(fs->inodes[fs->inode_count].owner, current_own, strlen(current_own));
    strncpy(fs->inodes[fs->inode_count].permissions, permissions, 10);
    strncpy(fs->inodes[fs->inode_count].group, current_group, strlen(current_group));

    
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        for (int j = 0; j < MAX_FILENAME; j++) {
            fs->inodes[fs->inode_count].lien.hardLink[i].data[j] = '\0'; // Initialiser les liens physiques a vide
        }     
    }
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        for (int j = 0; j < MAX_FILENAME; j++) {
            fs->inodes[fs->inode_count].lien.symbolicLink[i].data[j] = '\0'; // Initialiser les liens physiques a vide
        }     
    }

    fs->inode_count++;

    save_filesystem(fs);
    //printf("Répertoire '%s' créé.\n", path);
    //pthread_mutex_unlock(&fs_mutex); // Déverrouiller avant de retourner
    return 1;
}

// Fonction pour créer un répertoire
int create_directory(Filesystem *fs, const char *dirname, const char *destname) {
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return 0;
    }
    if (strcmp(fs->current_directory,GROUP_FILE) == 0) {
        printf("Utilise la commande crtgroup <nom>\n");
        return 0;
    }
    strcpy(permissions, "drw-------");
    
    char perm = 'w';

    // Construire le chemin complet
    char path[MAX_FILENAME * 2];
    if (destname == NULL) {
        if (strcmp(fs->current_directory, "\0") == 0) {
            snprintf(path, sizeof(path), "%s", dirname);
        }     
        else if (strcmp(fs->current_directory, ".") == 0) {
            snprintf(path, sizeof(path), "./%s", dirname);
        }  
        else {    
            // Vérifie si un répertoire existe déjà 
            for (int i = 0; i < fs->inode_count; i++) {
                if (strcmp(fs->inodes[i].name, fs->current_directory) == 0 && fs->inodes[i].is_directory) {
                    if ((fs->inodes[i].permissions[2] == perm && strcmp(fs->inodes[i].owner, current_own)== 0) || (fs->inodes[i].permissions[8] == perm)) {
                        snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);
                    } 
                    else { 
                        printf("Accès refusé : %s est un répertoire privé!\n", fs->current_directory);
                        return 0;
                    }
                }
            }
        }
    }  
    else { 
        // Vérifie si un répertoire existe déjà 
        char path_rep[MAX_FILENAME * 3];
        char dot_path[MAX_FILENAME];
        if (strcmp(fs->current_directory, "./") == 0) {
            snprintf(path_rep, sizeof(path_rep), "./%s", destname);
            snprintf(dot_path, sizeof(dot_path), ".");
        } 
        else { 
            snprintf(path_rep, sizeof(path_rep), "%s/%s", fs->current_directory, destname);
            snprintf(dot_path, sizeof(dot_path), "%s", fs->current_directory);
        } 
        for (int i = 0; i < fs->inode_count; i++) {
            if (strcmp(fs->inodes[i].name, path_rep) == 0 && fs->inodes[i].is_directory) {
                if ((fs->inodes[i].permissions[2] == perm && strcmp(fs->inodes[i].owner, current_own) == 0) || (fs->inodes[i].permissions[8] == perm)) {
                        snprintf(path, sizeof(path), "%s/%s/%s",dot_path, destname, dirname);
                } 
                else { 
                    printf("Accès refusé : %s est un répertoire privé!\n", path_rep);
                    return 0;
                }                
            }
        }
    }

    // Vérification si le répertoire existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, path) == 0) {
            printf("Erreur : le répertoire '%s' existe déjà.\n", path);
            return 0;
        }
    }

    strcpy(fs->inodes[fs->inode_count].name, path);
    fs->inodes[fs->inode_count].is_directory = 1;
    fs->inodes[fs->inode_count].is_group = 0;
    fs->inodes[fs->inode_count].is_file = 0;
    fs->inodes[fs->inode_count].is_link = 0;
    fs->inodes[fs->inode_count].size = 0;
    fs->inodes[fs->inode_count].inode_number = sb.next_inode_number++;
    fs->inodes[fs->inode_count].parent_inode_number = fs->inodes[fs->inode_count].inode_number;

    // Initialisation des métadonnées
    time_t now = time(NULL); // Récupère l'heure actuelle
    fs->inodes[fs->inode_count].creation_time = now;
    fs->inodes[fs->inode_count].modification_time = now;
    fs->inodes[fs->inode_count].num_liens = 0;
    strncpy(fs->inodes[fs->inode_count].owner, current_own, strlen(current_own));
    strncpy(fs->inodes[fs->inode_count].permissions, permissions, 10);
    strncpy(fs->inodes[fs->inode_count].group, current_group, strlen(current_group));

    
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        for (int j = 0; j < MAX_FILENAME; j++) {
            fs->inodes[fs->inode_count].lien.hardLink[i].data[j] = '\0'; // Initialiser les liens physiques a vide
        }     
    }
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        for (int j = 0; j < MAX_FILENAME; j++) {
            fs->inodes[fs->inode_count].lien.symbolicLink[i].data[j] = '\0'; // Initialiser les liens physiques a vide
        }     
    }

    fs->inode_count++;

    save_filesystem(fs);
    //printf("Répertoire '%s' créé.\n", path);
    //pthread_mutex_unlock(&fs_mutex); // Déverrouiller avant de retourner
    return 1;
}

// Fonction pour créer un répertoire de groupe
int create_directory_group(Filesystem *fs, const char *dirname) {
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return 0;
    }
    strcpy(permissions, "drw-------");

    // Construire le chemin complet
    char path[MAX_FILENAME * 2];
    if (strcmp(fs->current_directory, "\0") == 0) {
        snprintf(path, sizeof(path), "%s", dirname);
    }     
    if (strcmp(fs->current_directory, ".") == 0) {
        snprintf(path, sizeof(path), "./%s", dirname);
    } else {
        snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);
    }

    strcpy(fs->inodes[fs->inode_count].name, path);
    fs->inodes[fs->inode_count].is_directory = 1;
    fs->inodes[fs->inode_count].is_group = 1;
    fs->inodes[fs->inode_count].is_file = 0;
    fs->inodes[fs->inode_count].is_link = 0;
    fs->inodes[fs->inode_count].size = 0;
    fs->inodes[fs->inode_count].inode_number = sb.next_inode_number++;
    fs->inodes[fs->inode_count].parent_inode_number = fs->inodes[fs->inode_count].inode_number;

    // Initialisation des métadonnées
    time_t now = time(NULL); // Récupère l'heure actuelle
    fs->inodes[fs->inode_count].creation_time = now;
    fs->inodes[fs->inode_count].modification_time = now;
    fs->inodes[fs->inode_count].num_liens = 0;

    strncpy(fs->inodes[fs->inode_count].owner, current_own, strlen(current_own));
    strncpy(fs->inodes[fs->inode_count].permissions, permissions, 10);
    strncpy(fs->inodes[fs->inode_count].group, current_group, strlen(current_group));

    
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        for (int j = 0; j < MAX_FILENAME; j++) {
            fs->inodes[fs->inode_count].lien.hardLink[i].data[j] = '\0'; // Initialiser les liens physiques a vide
        }     
    }
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        for (int j = 0; j < MAX_FILENAME; j++) {
            fs->inodes[fs->inode_count].lien.symbolicLink[i].data[j] = '\0'; // Initialiser les liens physiques a vide
        }     
    }

    fs->inode_count++;

    save_filesystem(fs);
    return 1;
    //printf("Répertoire '%s' créé.\n", path);
    //pthread_mutex_unlock(&fs_mutex); // Déverrouiller avant de retourner
}

// Fonction pour supprimer un groupe
int delete_group(Filesystem *fs, const char *groupname) {
    // Vérification que le nom de groupe est valide
    if (groupname == NULL || strlen(groupname) == 0) {
        printf("Erreur : Nom de groupe invalide.\n");
        return 0;
    }

    // Vérification des permissions (sudo requis)
    if (!sudo) {
        printf("Erreur : Cette opération nécessite les privilèges sudo.\n");
        printf("Utilisez 'sudo delgroup %s'\n", groupname);
        return 0;
    }

    // Chemin du répertoire du groupe
    char group_path[MAX_PATH];
    snprintf(group_path, sizeof(group_path), "./users/groups/%s", groupname);

    // Vérifier si le groupe existe
    int group_found = 0;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, group_path) == 0 && fs->inodes[i].is_directory) {
            group_found = 1;
            break;
        }
    }

    if (!group_found) {
        printf("Erreur : Le groupe '%s' n'existe pas.\n", groupname);
        return 0;
    }

    // Sauvegarder le répertoire courant
    char current_dir_backup[MAX_PATH];
    strcpy(current_dir_backup, fs->current_directory);

    // 1. Supprimer le répertoire du groupe
    strcpy(fs->current_directory, "./users/groups");
    delete_directory(fs, groupname);

    // 2. Retirer le groupe de tous les utilisateurs
    for (int i = 0; i < NUM_USER; i++) {
        if (strlen(fs->group[i].user) > 0) { // Si l'utilisateur existe
            for (int j = 0; j < fs->group[i].taille; j++) {
                if (strcmp(fs->group[i].group[j].data, groupname) == 0) {
                    // Décaler les groupes restants
                    for (int k = j; k < fs->group[i].taille - 1; k++) {
                        strcpy(fs->group[i].group[k].data, fs->group[i].group[k+1].data);
                    }
                    fs->group[i].taille--;
                    
                    // Mettre à jour le groupe actuel si nécessaire
                    if (strcmp(current_group, groupname) == 0 && 
                        strcmp(fs->group[i].user, current_own) == 0) {
                        strcpy(current_group, "");
                    }
                    break;
                }
            }
        }
    }

    // 3. Supprimer tous les fichiers du groupe (optionnel)
    // Cette partie peut être commentée si vous voulez garder les fichiers
    for (int i = 0; i < fs->inode_count; ) {
        if (strcmp(fs->inodes[i].group, groupname) == 0) {
            char file_path[MAX_PATH];
            strcpy(file_path, fs->inodes[i].name);
            
            if (fs->inodes[i].is_directory) {
                delete_directory(fs, file_path);
            } else {
                delete_file(fs, file_path);
            }
            // Ne pas incrémenter i car la suppression réduit fs->inode_count
        } else {
            i++;
        }
    }

    // Restaurer le répertoire courant
    strcpy(fs->current_directory, current_dir_backup);

    printf("Groupe '%s' supprimé avec succès.\n", groupname);
    save_filesystem(fs);
    return 1;
}

// Fonction pour supprimer un répertoire
int delete_directory(Filesystem *fs, const char *dirname) {
    
   if (strcmp(fs->current_directory,"./users/groups/") == 0) {
       printf("Utilise la commande delgroup <nom>\n");
       return 0;
    }
    char path[MAX_FILENAME * 2];
    //snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);

    if (strncmp(dirname, "./users/home/", strlen("./users/home/")) == 0) {  
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
            printf("Répertoire '%s' supprimé.\n", last_element(path));
            return 1; // Répertoire supprimé avec succès
        }
    }
    printf("Répertoire '%s' introuvable !\n", path);
    return 0; // Répertoire introuvable
}

// Fonction pour vérifier si un utilisateur appartient à un groupe
int is_user_in_group(Filesystem *fs, const char *username, const char *groupname) {
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            for (int j = 0; j < fs->group[i].taille; j++) {
                if (strcmp(fs->group[i].group[j].data, groupname) == 0) {
                    return 1; // L'utilisateur est dans le groupe
                }
            }
            break;
        }
    }
    return 0; // L'utilisateur n'est pas dans le groupe
}

// Fonction pour changer de répertoire
int change_directory(Filesystem *fs, const char *dirname) {
    // Vérification des entrées
    if (dirname == NULL || strlen(dirname) == 0) {
        printf("Erreur: nom de répertoire invalide.\n");
        return 0;
    }

    // Cas spécial pour la navigation parent
    if (strcmp(dirname, "..") == 0) {
        // Si déjà à la racine
        if (strcmp(fs->current_directory, ".") == 0 || 
            strcmp(fs->current_directory, "./") == 0) {
            printf("Vous êtes déjà à la racine.\n");
            return 0;
        }

        // Trouver le dernier '/'
        char *last_slash = strrchr(fs->current_directory, '/');
        if (last_slash == NULL) {
            // Cas improbable où il n'y a pas de slash
            strcpy(fs->current_directory, ".");
        } else {
            // Cas normal - tronquer au dernier slash
            if (last_slash == fs->current_directory) {
                // Cas "/something" -> "/"
                *(last_slash + 1) = '\0';
            } else {
                // Cas "/path/to/something" -> "/path/to"
                *last_slash = '\0';
                
                // Cas spécial pour "./something" -> "."
                if (strcmp(fs->current_directory, ".") == 0) {
                    strcpy(fs->current_directory, "./");
                }
            }
        }
        printf("Déplacé dans '%s'.\n", fs->current_directory);
        return 1;
    }

    // Vérification des permissions pour les répertoires spéciaux
    char perm = 'r';
    Inode *inod = get_inode_by_name(fs, dirname);
    
    // Accès au répertoire home
    if (strcmp(fs->current_directory, "./users/home") == 0) {
        if (inod != NULL && inod->permissions[7] != perm) {
            if (strcmp(inod->owner, current_own) != 0) {
                printf("Accès refusé : %s est un répertoire privé!\n", dirname);
                return 0;
            }
        }
    }
    
    // Accès aux groupes
    if (strcmp(fs->current_directory, "./users/groups") == 0) {
        if (inod != NULL && strcmp(dirname, "..") != 0 &&  !is_user_in_group(fs, current_own, dirname) && 
            inod->permissions[6] != perm) {
            printf("Accès refusé : %s est un groupe privé ou vous n'en faites pas partie!\n", dirname);
            return 0;
        }
    }

    // Construction du nouveau chemin
    char path[MAX_FILENAME * 2];
    if (strcmp(fs->current_directory, "./") == 0 || strcmp(fs->current_directory, ".") == 0) {
        // Cas spécial pour la racine
        snprintf(path, sizeof(path), "./%s", dirname);
    } else {
        snprintf(path, sizeof(path), "%s/%s", fs->current_directory, dirname);
    }

    // Recherche du répertoire
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, path) == 0 && fs->inodes[i].is_directory) {
            strcpy(fs->current_directory, path);
            printf("Déplacé dans '%s'.\n", fs->current_directory);
            return 1; // Changement de répertoire réussi
        }
    }
    printf("Répertoire '%s' introuvable !\n", dirname);
    return 0; // Répertoire introuvable
}

// Fonction pour créer un fichier
int create_file(Filesystem *fs, const char *filename, size_t size, const char *owner) {
    strcpy(permissions, "-rw-r--r--"); // Permissions par défaut 

    char full_path[MAX_FILENAME * 3];
    char path[MAX_FILENAME * 2];        
    if (strcmp(current_group, current_own) == 0) {
        snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);
        snprintf(path, sizeof(path), "%s", fs->current_directory);
    }
    else {
        snprintf(full_path, sizeof(full_path), "%s/%s/%s", GROUP_FILE,current_group, filename);
        snprintf(path, sizeof(path), "%s/%s", GROUP_FILE,current_group);
    }

    // Vérifie si un fichier existe déjà dans le répertoire courant
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0) {
            printf("Le fichier existe déjà !\n");
            return 0;
        }
    }

    // Vérifier si le groupe actuel est vide
    if (strlen(current_group) == 0) {
        printf("Erreur : aucun groupe n'est défini pour l'utilisateur actuel.\n");
        return 0;
    }

    // Crée un nouvel inode pour le fichier
    strcpy(fs->inodes[fs->inode_count].name, full_path);
    fs->inodes[fs->inode_count].size = size;
    fs->inodes[fs->inode_count].is_directory = 0; // Ce n'est pas un répertoire
    fs->inodes[fs->inode_count].is_group = 0;
    fs->inodes[fs->inode_count].is_file = 1;
    fs->inodes[fs->inode_count].is_link = 0;
    fs->inodes[fs->inode_count].inode_number = sb.next_inode_number++;
    fs->inodes[fs->inode_count].parent_inode_number = fs->inodes[fs->inode_count].inode_number; // Parent inode number

    // Initialisation des métadonnées
    time_t now = time(NULL); // Récupère l'heure actuelle
    fs->inodes[fs->inode_count].creation_time = now;
    fs->inodes[fs->inode_count].modification_time = now;
    fs->inodes[fs->inode_count].num_liens = 0;
    fs->inodes[fs->inode_count].is_directory = 0;
    fs->inodes[fs->inode_count].is_group = 0; 
    strncpy(fs->inodes[fs->inode_count].owner, owner, strlen(owner));
    strncpy(fs->inodes[fs->inode_count].permissions, permissions, 10);
    strncpy(fs->inodes[fs->inode_count].group, current_group, strlen(current_group));
    
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        for (int j = 0; j < MAX_FILENAME; j++) {
            fs->inodes[fs->inode_count].lien.hardLink[i].data[j] = '\0'; // Initialiser les liens physiques a vide
        }     
    }
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        for (int j = 0; j < MAX_FILENAME; j++) {
            fs->inodes[fs->inode_count].lien.symbolicLink[i].data[j] = '\0'; // Initialiser les liens physiques a vide
        }     
    }

    fs->inode_count++;

    // Vérification si le répertoire existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, path) == 0) {
            fs->inodes[i].size = calculate_directory_size_recursive(fs,fs->inodes[i].name); 
        }
    }
    
    save_filesystem(fs);
    printf("Fichier '%s' créé (%zu octets).\n", full_path, size);
    return 1;
}

// Fonction pour lister le contenu du répertoire courant
int list_directory(Filesystem *fs) {
    printf("Contenu de '%s':\n", fs->current_directory);
    int found = 0;
    size_t current_dir_len = strlen(fs->current_directory);

    for (int i = 0; i < fs->inode_count; i++) {
        // Check if the item is directly in current directory
        if (strncmp(fs->inodes[i].name, fs->current_directory, current_dir_len) == 0) {
            const char *remaining_path = fs->inodes[i].name + current_dir_len;
            
            // Skip leading slash if present
            if (*remaining_path == '/') remaining_path++;
            
            // If there are no more slashes, it's directly in this directory
            if (strlen(remaining_path) > 0 && strchr(remaining_path, '/') == NULL) {
                if (fs->inodes[i].is_directory) {
                    printf("[DIRC] %s/\n", remaining_path);
                } 
                if (fs->inodes[i].is_link) {
                    printf("[LINK] %s\n", remaining_path);
                } 
                if (fs->inodes[i].is_file) {
                    printf("[FILE] %s (%d octets)\n", remaining_path, fs->inodes[i].size);
                }
                found = 1;
            }
        }
    }

    if (!found) { 
        printf("Le répertoire est vide.\n"); 
        return 0; 
    } 
    return 1;
}

// Fonction pour afficher les métadonnées d'un fichier
int show_file_metadata(Filesystem *fs, const char *filename) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);

    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0) {
            printf("Métadonnées de '%s':\n", full_path);
            printf("  Propriétaire: %s\n", fs->inodes[i].owner);
            printf("  Groupe: %s\n", fs->inodes[i].group);
            printf("  Permissions: %s\n", fs->inodes[i].permissions);
            printf("  Numéro d'inode: %lu\n", (unsigned long)fs->inodes[i].inode_number);
            printf("  Nombre de liens: %d\n", fs->inodes[i].num_liens);

            char creation_time[100];
            char modification_time[100];

            // Formater les dates pour un affichage lisible
            strftime(creation_time, sizeof(creation_time), "%Y-%m-%d %H:%M:%S", localtime(&fs->inodes[i].creation_time));
            strftime(modification_time, sizeof(modification_time), "%Y-%m-%d %H:%M:%S", localtime(&fs->inodes[i].modification_time));

            printf("  Date de création: %s\n", creation_time);
            printf("  Date de modification: %s\n", modification_time);
            printf("  Taille: %d octets\n", fs->inodes[i].size);
            return 1;
        }
    }
    printf("Fichier '%s' introuvable.\n", full_path);
    return 0;
}

// Fonction pour afficher les métadonnées d'un fichier
int show_directory_metadata(Filesystem *fs, const char *namerep) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, namerep);

    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0 && fs->inodes[i].is_directory == 1) {
            printf("  Métadonnées de '%s':\n", full_path);
            printf("  Propriétaire: %s\n", fs->inodes[i].owner);
            printf("  Groupe: %s\n", fs->inodes[i].group);
            printf("  Permissions: %s\n", fs->inodes[i].permissions);
            printf("  Nombre de liens: %lu\n", (unsigned long)fs->inodes[i].num_liens);

            char creation_time[100];
            char modification_time[100];

            // Formater les dates pour un affichage lisible
            strftime(creation_time, sizeof(creation_time), "%Y-%m-%d %H:%M:%S", localtime(&fs->inodes[i].creation_time));
            strftime(modification_time, sizeof(modification_time), "%Y-%m-%d %H:%M:%S", localtime(&fs->inodes[i].modification_time));

            printf("  Date de création: %s\n", creation_time);
            printf("  Date de modification: %s\n", modification_time);
            // mettre à jour la taille du répertoire
            fs->inodes[i].size = calculate_directory_size_recursive(fs,fs->inodes[i].name); 
            printf("  Taille: %d octets\n", fs->inodes[i].size);
            return 1;
        }
    }
    printf("Répertoire '%s' introuvable.\n", full_path);
    return 0;
}

// Fonction pour modifier les permissions d'un fichier
int chmod_file(Filesystem *fs, const char *filename, const char *target, const char *new_permissions) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);

    // Parcourir tous les inodes pour trouver le fichier correspondant
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0) {
            if (strcmp(current_own, fs->inodes[i].owner) == 0) {
                // Gérer les permissions en fonction de la cible
                if (strcmp(target, "-Owner") == 0) {
                    if (strlen(new_permissions) == 3) {
                        strncpy(fs->inodes[i].permissions + 1, new_permissions, 3);  // Mettre à jour les permissions du propriétaire
                        printf("Permissions de '%s' pour le propriétaire mises à jour en '%s'.\n", filename, new_permissions);
                    } else {
                        printf("Les permissions du propriétaire doivent être exactement 3 caractères (rwx).\n");
                        
                    }
                } else if (strcmp(target, "-Group") == 0) {
                    if (strlen(new_permissions) == 3) {
                        strncpy(fs->inodes[i].permissions + 4, new_permissions, 3);  // Mettre à jour les permissions du groupe
                        printf("Permissions de '%s' pour le groupe mises à jour en '%s'.\n", filename, new_permissions);
                    } else {
                        printf("Les permissions du groupe doivent être exactement 3 caractères (rwx).\n");
                    }
                } else if (strcmp(target, "-Others") == 0) {
                    if (strlen(new_permissions) == 3) {
                        strncpy(fs->inodes[i].permissions + 7, new_permissions, 3);  // Mettre à jour les permissions des autres
                        printf("Permissions de '%s' pour les autres mises à jour en '%s'.\n", filename, new_permissions);
                    } else {
                        printf("Les permissions des autres doivent être exactement 3 caractères (rwx).\n");
                    }
                } else {
                    printf("Option '%s' inconnue !\n", target);
                    return 0;
                }

                // Mettre à jour la date de modification
                fs->inodes[i].modification_time = time(NULL);
                save_filesystem(fs);
                return 1;
            }
            printf("Vous n'êtes pas pripriétaire de ce fichier!\n");
            return 0;
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Fichier '%s' introuvable !\n", filename);
    return 0;
}

// Fonction pour modifier les permissions d'un fichier
int chmod_dir(Filesystem *fs, const char *repertoire_name, const char *target, const char *new_permissions) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, repertoire_name);

    // Parcourir tous les inodes pour trouver le répertoire correspondant
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0) {
            if (strcmp(current_own, fs->inodes[i].owner) == 0 || is_user_in_group(fs, current_own, repertoire_name)) {
                // Gérer les permissions en fonction de la cible
                if (strcmp(target, "-Owner") == 0) {
                    if (strlen(new_permissions) == 3) {
                        strncpy(fs->inodes[i].permissions + 1, new_permissions, 3);  // Mettre à jour les permissions du propriétaire
                        printf("Permissions de '%s' pour le propriétaire mises à jour en '%s'.\n", repertoire_name, new_permissions);
                    } else {
                        printf("Les permissions du propriétaire doivent être exactement 3 caractères (rwx).\n");
                    }
                } else if (strcmp(target, "-Group") == 0) {
                    if (strlen(new_permissions) == 3) {
                        strncpy(fs->inodes[i].permissions + 4, new_permissions, 3);  // Mettre à jour les permissions du groupe
                        printf("Permissions de '%s' pour le groupe mises à jour en '%s'.\n", repertoire_name, new_permissions);
                    } else {
                        printf("Les permissions du groupe doivent être exactement 3 caractères (rwx).\n");
                    }
                } else if (strcmp(target, "-Others") == 0) {
                    if (strlen(new_permissions) == 3) {
                        strncpy(fs->inodes[i].permissions + 7, new_permissions, 3);  // Mettre à jour les permissions des autres
                        printf("Permissions de '%s' pour les autres mises à jour en '%s'.\n", repertoire_name, new_permissions);
                    } else {
                        printf("Les permissions des autres doivent être exactement 3 caractères (rwx).\n");
                    }
                } else {
                    printf("Option '%s' inconnue !\n", target);
                    return 0;
                }

                // Mettre à jour la date de modification
                fs->inodes[i].modification_time = time(NULL);
                save_filesystem(fs);
                return 1;
            }
            printf("Vous n'êtes pas pripriétaire de ce répertoire!\n");
            return 0;
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Répertoire '%s' introuvable !\n", repertoire_name);
    return 0;
}

// Fonction pour allouer un bloc de données
int allocate_block() {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (sb.free_blocks[i] == 1) { // 1 = libre
            sb.free_blocks[i] = 0; // Marquer le bloc comme occupé
            return i; // Retourner l'index du bloc alloué
        }
    }
    return 0; // Aucun bloc libre disponible
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

// Fonction récursive pour calculer la taille d'un répertoire (fichiers + sous-répertoires)
int calculate_directory_size_recursive(Filesystem *fs, const char *dirpath) {
    int total_size = 0;
    size_t dirpath_len = strlen(dirpath);

    for (int i = 0; i < fs->inode_count; i++) {
        // Vérifier si le fichier/répertoire est dans ce répertoire ou ses sous-répertoires
        if (strncmp(fs->inodes[i].name, dirpath, dirpath_len) == 0) {
            // Pour les fichiers directs dans ce répertoire
            if (!fs->inodes[i].is_directory && strcmp(fs->inodes[i].name, dirpath) != 0 && strchr(fs->inodes[i].name + dirpath_len + 1, '/') == NULL) {
                total_size += fs->inodes[i].size;
            }
            // Pour les sous-répertoires (appel récursif)
            else if (fs->inodes[i].is_directory &&  strcmp(fs->inodes[i].name, dirpath) != 0) {
                // Vérifier que c'est un sous-répertoire direct
                char *subdir = fs->inodes[i].name + dirpath_len;
                if (*subdir == '/' && strchr(subdir + 1, '/') == NULL) {
                    total_size += calculate_directory_size_recursive(fs, fs->inodes[i].name);
                }
            }
        }
    }
    return total_size;
}

// Fonction pour écrire du contenu dans un fichier
int write_to_file(Filesystem *fs, const char *filename, const char *content) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);
    char perm = 'w';

    // Rechercher le fichier dans les inodes
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0 && fs->inodes[i].is_file) {
            // Vérifier si l'utilisateur a les permissions d'écriture
            if (strcmp(fs->inodes[i].owner, current_own) == 0 && fs->inodes[i].permissions[2] != perm) {
                printf("Permission refusée : L'utilisateur %s n'a pas les droits nécessaires.\n", current_own);
                return 0;
            }
        
            if (strcmp(fs->inodes[i].owner, current_own) != 0 && strcmp(fs->inodes[i].group, current_group) == 0 && fs->inodes[i].permissions[5] != perm) {
                printf("Permission refusée : Le groupe %s ne possède pas les droits nécessaires.\n", current_group);
                return 0;
            }
        
            if (strcmp(fs->inodes[i].owner, current_own) != 0 && strcmp(fs->inodes[i].group, current_group) != 0 && fs->inodes[i].permissions[8] != perm) {
                printf("Permission refusée : Ni l'utilisateur %s ni le groupe %s ne possèdent les droits nécessaires.\n", current_own, current_group);
                return 0;
            }

            // Calculer le nombre de blocs nécessaires
            size_t content_size = strlen(content);
            int blocks_needed = (content_size / BLOCK_SIZE) + 1;

            // Vérifier s'il y a suffisamment de blocs libres
            if (count_free_blocks() < blocks_needed) {
                printf("Espace insuffisant : %d blocs nécessaires, %d blocs libres.\n", blocks_needed, count_free_blocks());
                return 0;
            }

            // Allouer les blocs et écrire le contenu
            for (int j = 0; j < blocks_needed; j++) {
                int block_index = allocate_block();
                if (block_index == -1) {
                    printf("Erreur d'allocation de bloc.\n");
                    return 0;
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
            
            // Vérification si le répertoire existe déjà
            for (int i = 0; i < fs->inode_count; i++) {
                if (strcmp(fs->inodes[i].name, fs->current_directory) == 0) {
                    fs->inodes[i].size = calculate_directory_size_recursive(fs,fs->inodes[i].name); 
                }
            }

            // Sauvegarder le système de fichiers
            save_filesystem(fs);
            printf("Contenu écrit dans le fichier '%s'.\n", filename);
            return 1; // Écriture réussie
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Fichier '%s' introuvable ou est un répertoire.\n", filename);
    return 0;
}

// Fonction pour lire le contenu d'un fichier ||||||||
int read_file(Filesystem *fs, const char *filename) {
    char full_path[MAX_FILENAME * 2];
    snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);
    char perm = 'r';

    // Rechercher le fichier dans les inodes
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0 && fs->inodes[i].is_file) {
            // Vérifier si l'utilisateur a les permissions de lecture
            if (strcmp(fs->inodes[i].owner, current_own) == 0 && fs->inodes[i].permissions[1] != perm) {
                printf("Permission refusée : L'utilisateur %s n'a pas les droits de lecture.\n", current_own);
                return 0;
            }
        
            if (strcmp(fs->inodes[i].owner, current_own) != 0 && strcmp(fs->inodes[i].group, current_group) == 0 && fs->inodes[i].permissions[4] != perm) {
                printf("Permission refusée : Le groupe %s ne possède pas les droits de lecture.\n", current_group);
                return 0;
            }
        
            if (strcmp(fs->inodes[i].owner, current_own) != 0 && strcmp(fs->inodes[i].group, current_group) != 0 && fs->inodes[i].permissions[7] != perm) {
                printf("Permission refusée : Ni l'utilisateur %s ni le groupe %s ne possèdent les droits de lecture.\n", current_own, current_group);
                return 0;
            }

            // Afficher le contenu du fichier
            printf("Contenu du fichier '%s':\n", filename);
            if (fs->inodes[i].block_count == 0) {
                printf("(Le fichier est vide)\n");
            } else {
                for (int j = 0; j < fs->inodes[i].block_count; j++) {
                    int block_index = fs->inodes[i].block_indices[j];
                    printf("%s", block_data[block_index]);
                }
            }
            printf("\n");
            return 1; // Lecture réussie
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Fichier '%s' introuvable ou est un répertoire.\n", filename);
    return 0;
}

// Fonction pour supprimer un fichier
int delete_file(Filesystem *fs, const char *filename) {
    char full_path[MAX_FILENAME * 2];

    if (strncmp(filename, "/home/", strlen("/home/")) == 0) {  
        snprintf(full_path, sizeof(full_path), "%s", filename);
    } else {
        snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, filename);
    }

    // Rechercher le fichier dans les inodes
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path) == 0 && fs->inodes[i].is_file) {
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
            return 1; // Fichier supprimé avec succès
        }
    }

    // Si le fichier n'est pas trouvé
    printf("Fichier '%s' introuvable ou est un répertoire.\n", filename);
    return 0;
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
int copy_file(Filesystem *fs, const char *file_name, const char *link_name, const char *rep_name) {
    char full_path_source[MAX_FILENAME * 2];
    char full_file_path[MAX_FILENAME * 2];
    char dest_directory[MAX_FILENAME * 2];
    char prevent_path[MAX_FILENAME * 2];
    strncpy(prevent_path, fs->current_directory, MAX_FILENAME);

    // Construire le chemin complet du fichier source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, file_name);

    if (rep_name != NULL) {
        // Vérifier si le rep_name est un chemin complet ou un répertoire relatif
        if (strchr(rep_name, '/') != NULL) {
            char exists_path[MAX_FILENAME * 2 - 2];
            // C'est un répertoire relatif au répertoire courant
            snprintf(exists_path, sizeof(exists_path), "%s/%s", fs->current_directory, rep_name);
            // C'est un chemin complet
            if (!directory_exists(fs, exists_path)) {
                printf("Le répertoire '%s' n'existe pas.\n", rep_name);
                return 0;
            }

            snprintf(full_file_path, sizeof(full_file_path), "%s/%s", exists_path, link_name);
        } else if (strcmp(rep_name, "..") == 0) {
            snprintf(full_file_path, sizeof(full_file_path), "%s/%s", retirer_suffixe(fs->current_directory), link_name); // Chemin relatif au répertoire parent
        } else {
            // C'est un répertoire relatif au répertoire courant
            snprintf(dest_directory, sizeof(dest_directory), "%s/%s", fs->current_directory, rep_name);
            if (!directory_exists(fs, dest_directory)) {
                printf("Le répertoire '%s' n'existe pas dans le répertoire courant.\n", rep_name);
                return 0;
            }
            snprintf(full_file_path, sizeof(full_file_path), "%s/%s/%s", fs->current_directory, rep_name, link_name);
        }
    } else {
        snprintf(full_file_path, sizeof(full_file_path), "%s/%s", fs->current_directory, link_name);
    }
    

    // Rechercher le fichier source dans les inodes
    Inode *source_inode = NULL;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && fs->inodes[i].is_file) {
            source_inode = &fs->inodes[i];
            break;
        }
    }

    if (!source_inode) {
        printf("Fichier source '%s' introuvable ou est un répertoire.\n", file_name);
        return 0;
    }

    // Vérifier si le fichier de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_file_path) == 0) {
            printf("Le fichier de destination '%s' existe déjà.\n", link_name);
            return 0;
        }
    }

    // Créer un nouvel inode pour le fichier de destination
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return 0;
    }

    // Copier les métadonnées du fichier source
    Inode *dest_inode = &fs->inodes[fs->inode_count];
    strcpy(dest_inode->name, full_file_path);
    dest_inode->is_directory = 0;
    dest_inode->is_file = 1;
    dest_inode->size = source_inode->size;
    dest_inode->inode_number = sb.next_inode_number++;
    dest_inode->parent_inode_number = dest_inode->inode_number; // Parent inode number
    dest_inode->creation_time = time(NULL);
    dest_inode->modification_time = time(NULL);
    strncpy(dest_inode->owner, source_inode->owner, MAX_FILENAME);
    strncpy(dest_inode->permissions, source_inode->permissions, 10);
    strncpy(dest_inode->group, source_inode->group, GROUP_SIZE);

    // Allouer des blocs pour le fichier de destination
    dest_inode->block_count = 0;
    for (int i = 0; i < source_inode->block_count; i++) {
        int block_index = allocate_block();
        if (block_index == -1) {
            printf("Erreur d'allocation de bloc pour la copie.\n");
            return 0;
        }
        dest_inode->block_indices[dest_inode->block_count++] = block_index;

        // Copier le contenu du bloc source vers le bloc de destination
        strncpy(block_data[block_index], block_data[source_inode->block_indices[i]], BLOCK_SIZE);
    }

    // Incrémenter le nombre d'inodes
    fs->inode_count++;

    // Sauvegarder le système de fichiers
    save_filesystem(fs);
    printf("Fichier '%s' copié vers '%s'.\n", file_name, full_file_path);
    strncpy(fs->current_directory, prevent_path, MAX_FILENAME);
    return 1;
}

// Fonction pour déplacer un fichier
int move_file(Filesystem *fs, const char *filename, const char *rep_name) {
    char full_path_source[MAX_FILENAME * 2];
    char full_file_path[MAX_FILENAME * 2];
    char dest_directory[MAX_FILENAME * 2];
    char prevent_path[MAX_FILENAME * 2];
    strncpy(prevent_path, fs->current_directory, MAX_FILENAME);

    // Construire le chemin complet du fichier source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, filename);

    // Vérifier si le rep_name est un chemin complet ou un répertoire relatif
    if (strchr(rep_name, '/') != NULL) {
        char exists_path[MAX_FILENAME * 2-2];
        // C'est un répertoire relatif au répertoire courant
        snprintf(exists_path, sizeof(exists_path), "%s/%s", fs->current_directory, rep_name);
        // C'est un chemin complet
        if (!directory_exists(fs, exists_path)) {
            printf("Le répertoire '%s' n'existe pas.\n", rep_name);
            return 0;
        }
        snprintf(full_file_path, sizeof(full_file_path), "%s/%s", exists_path, filename);
    } else if (strcmp(rep_name, "..") == 0) {
        snprintf(full_file_path, sizeof(full_file_path), "%s/%s", retirer_suffixe(fs->current_directory), filename); // Chemin relatif au répertoire parent
    } else {
        // C'est un répertoire relatif au répertoire courant
        snprintf(dest_directory, sizeof(dest_directory), "%s/%s", fs->current_directory, rep_name);
        if (!directory_exists(fs, dest_directory)) {
            printf("Le répertoire '%s' n'existe pas dans le répertoire courant.\n", rep_name);
            return 0;
        }
        snprintf(full_file_path, sizeof(full_file_path), "%s/%s/%s", fs->current_directory, rep_name, filename);
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
        return 0;
    }

    // Vérifier si le fichier de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_file_path) == 0) {
            printf("Le fichier de destination '%s' existe déjà.\n", filename);
            return 0;
        }
    }

    // Créer un nouvel inode pour le fichier de destination
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return 0;
    }

    // Copier les métadonnées du fichier source
    Inode *dest_inode = &fs->inodes[fs->inode_count];
    strcpy(dest_inode->name, full_file_path);
    dest_inode->is_directory = 0;
    dest_inode->is_file = 1;
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
    printf("Fichier '%s' déplacé vers '%s'.\n", filename, full_file_path);
    strncpy(fs->current_directory, prevent_path, MAX_FILENAME);
    return 1;
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

// Fonction pour extraire le chemin relatif
char* last_element(const char* full_path) {
    char* dernier = strrchr(full_path,'/');
    return (dernier !=NULL)? dernier +1 :  (char*)full_path;
}

// Fonction pour retirer le suffixe d'une chaîne de caractères
char *retirer_suffixe(char *str) {
    // Recherche du dernier '/' dans la chaîne
    char *pos = strrchr(str, '/');
    if (pos != NULL) {
        // On termine la chaîne à la position du '/' pour supprimer le suffixe
        *pos = '\0';
    }
    return str;
}

// Fonction pour copier un répertoire et son contenu
int copy_repertoire(Filesystem *fs, const char *source_dir, const char *dest_name, const char *dest_parent) {
    char full_source_path[MAX_FILENAME * 2];
    char full_dest_path[MAX_FILENAME * 2];
    char temp_current_dir[MAX_PATH-1];

    // Construire les chemins complets
    if (source_dir[0] == '/') {
        snprintf(full_source_path, sizeof(full_source_path), "%s", source_dir); // Chemin absolu
    } else {
        snprintf(full_source_path, sizeof(full_source_path), "%s/%s", fs->current_directory, source_dir); // Chemin relatif
    }

    if (dest_parent != NULL) {
        if (dest_parent[0] == '/') {
            snprintf(full_dest_path, sizeof(full_dest_path), "%s/%s", dest_parent, dest_name); // Chemin absolu
        } else if (strcmp(dest_parent, "..") == 0) {
            snprintf(full_dest_path, sizeof(full_dest_path), "%s/%s", retirer_suffixe(fs->current_directory), dest_name); // Chemin relatif au répertoire parent
        } else {
            snprintf(full_dest_path, sizeof(full_dest_path), "%s/%s/%s", fs->current_directory, dest_parent, dest_name); // Chemin relatif au sous répertoire courant
        }
    } else {
        snprintf(full_dest_path, sizeof(full_dest_path), "%s/%s", fs->current_directory, dest_name); // Chemin relatif au répertoire courant
    }

    // Vérifier si le répertoire source existe
    Inode *src_inode = NULL;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_source_path) == 0 && fs->inodes[i].is_directory) {
            src_inode = &fs->inodes[i];
            break;
        }
    }

    if (!src_inode) {
        printf("Erreur: répertoire source '%s' introuvable.\n", full_source_path);
        return 0;
    }

    // Vérifier si la destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_dest_path) == 0) {
            printf("Erreur: le répertoire de destination '%s' existe déjà.\n", full_dest_path);
            return 0;
        }
    }

    // Sauvegarder le répertoire courant
    strcpy(temp_current_dir, fs->current_directory);

    // Créer le répertoire de destination
    if (dest_parent != NULL) {
        if (dest_parent[0] == '/') {
            strcpy(fs->current_directory, dest_parent);
        } else {
            snprintf(fs->current_directory, sizeof(fs->current_directory), "%s/%s", temp_current_dir, dest_parent);
        }
    }
    
    create_directory(fs, dest_name, NULL);
    strcpy(fs->current_directory, temp_current_dir); // Restaurer le répertoire courant

    // Copier récursivement le contenu
    for (int i = 0; i < fs->inode_count; i++) {
        // Vérifier si l'élément est dans le répertoire source
        if (strstr(fs->inodes[i].name, full_source_path) == fs->inodes[i].name && 
            strlen(fs->inodes[i].name) > strlen(full_source_path)) {
            
            // Extraire le chemin relatif
            const char *relative_path = fs->inodes[i].name + strlen(full_source_path) + 1;
            char new_path[MAX_FILENAME * 3];
            snprintf(new_path, sizeof(new_path), "%s/%s", full_dest_path, relative_path);

            if (fs->inodes[i].is_directory) {
                // Créer le sous-répertoire
                char parent_path[MAX_FILENAME * 2];
                char *last_slash = strrchr(new_path, '/');
                if (last_slash) {
                    strncpy(parent_path, new_path, last_slash - new_path);
                    parent_path[last_slash - new_path] = '\0';
                    char *dir_name = last_slash + 1;
                    strcpy(fs->current_directory, parent_path);
                    create_directory(fs, dir_name, NULL);
                }
            } else {
                // Copier le fichier avec le contenu - VERSION CORRIGÉE
                char current_path[MAX_FILENAME * 2];
                strcpy(current_path, fs->current_directory);
                
                // Déterminer le répertoire parent du nouveau fichier
                char *last_slash = strrchr(new_path, '/');
                if (last_slash) {
                    *last_slash = '\0';
                    strcpy(fs->current_directory, new_path);
                    *last_slash = '/';
                }
                
                // Créer le fichier vide
                create_file(fs, last_element(new_path), fs->inodes[i].size, fs->inodes[i].owner);

                
                // Copier le contenu
                if (fs->inodes[i].block_count > 0) {
                    char content[BLOCK_SIZE * BLOCK_SIZE] = {0};
                    for (int j = 0; j < fs->inodes[i].block_count; j++) {
                        int block_index = fs->inodes[i].block_indices[j];
                        strcat(content, block_data[block_index]);
                    }
                    write_to_file(fs, last_element(new_path), content);
                }
                
                // Restaurer le répertoire courant
                strcpy(fs->current_directory, current_path);
            }
        }
    }

    // Restaurer le répertoire courant
    strcpy(fs->current_directory, temp_current_dir);

    save_filesystem(fs);
    printf("Répertoire '%s' copié vers '%s' avec son contenu.\n", full_source_path, full_dest_path);
    return 1;
}

// Fonction pour déplacer un repertoire
int move_directory(Filesystem *fs, const char *repertoirename, const char *rep_name) {
    // Chemins complets pour le répertoire source et de destination
    char full_path_source[MAX_FILENAME * 2];
    char full_file_path[MAX_FILENAME * 2];
    char dest_directory[MAX_FILENAME * 2];
    char prevent_path[MAX_FILENAME * 2];
    strncpy(prevent_path, fs->current_directory, MAX_FILENAME);

    // Construire le chemin complet du répertoire source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, repertoirename);

    // Vérifier si le répertoire source existe
    Inode *source_inode = NULL;
    //int source_index = -1;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && fs->inodes[i].is_directory) {
            source_inode = &fs->inodes[i];
            //source_index = i;
            break;
        }
    }

    if (!source_inode) {
        printf("Erreur : le répertoire source '%s' n'existe pas ou n'est pas un répertoire.\n", full_path_source);
        return 0;
    }

    // Vérifier si le rep_name est un chemin complet ou un répertoire relatif
    if (strchr(rep_name, '/') != NULL) {            
        char exists_path[MAX_FILENAME * 2 - 2];
        // C'est un répertoire relatif au répertoire courant
        snprintf(exists_path, sizeof(exists_path), "%s/%s", fs->current_directory, rep_name);
        // C'est un chemin complet
        if (!directory_exists(fs, exists_path)) {
            printf("Erreur : le répertoire de destination '%s' n'existe pas.\n", rep_name);
            return 0;
        }
        snprintf(full_file_path, sizeof(full_file_path), "%s/%s", exists_path, repertoirename);
    } else if (strcmp(rep_name, "..") == 0) {
        snprintf(full_file_path, sizeof(full_file_path), "%s/%s", retirer_suffixe(fs->current_directory), repertoirename); // Chemin relatif au répertoire parent
    } else {
        // C'est un répertoire relatif au répertoire courant
        snprintf(dest_directory, sizeof(dest_directory), "%s/%s", fs->current_directory, rep_name);
        if (!directory_exists(fs, dest_directory)) {
            printf("Erreur : le répertoire de destination '%s' n'existe pas dans le répertoire courant.\n", rep_name);
            return 0;
        }
        snprintf(full_file_path, sizeof(full_file_path), "%s/%s/%s", fs->current_directory, rep_name, repertoirename);
    }

    // Vérifier si le répertoire de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_file_path) == 0) {
            printf("Erreur : le répertoire de destination '%s' existe déjà.\n", full_file_path);
            return 0;
        }
    }

    // Mettre à jour le chemin du répertoire source
    strcpy(source_inode->name, full_file_path);

    // Mettre à jour les chemins des fichiers et sous-répertoires dans le répertoire déplacé
    for (int i = 0; i < fs->inode_count; i++) {
        if (strstr(fs->inodes[i].name, full_path_source) == fs->inodes[i].name) {
            // Construire le nouveau chemin en remplaçant le chemin source par le chemin de destination
            char new_path[MAX_FILENAME * 4];
            snprintf(new_path, sizeof(new_path), "%s%s", full_file_path, fs->inodes[i].name + strlen(full_path_source));
            strcpy(fs->inodes[i].name, new_path);
        }
    }

    // Sauvegarder le système de fichiers
    save_filesystem(fs);
    printf("Répertoire '%s' déplacé vers '%s'.\n", full_path_source, full_file_path);
    strncpy( fs->current_directory, prevent_path,MAX_FILENAME);
    return 1;
}

// Fonction rénommer un fichier
int rename_file(Filesystem *fs, const char *file_name, const char *link_name) {
    char full_path_source[MAX_FILENAME * 2];
    char full_file_path[MAX_FILENAME * 2];

    // Construire le chemin complet du fichier source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, file_name);
    // Construire le chemin complet du fichier source
    snprintf(full_file_path, sizeof(full_file_path), "%s/%s", fs->current_directory, link_name);

    // Rechercher le fichier source dans les inodes
    Inode *source_inode = NULL;
    int index_inode = -1;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && fs->inodes[i].is_file) {
            source_inode = &fs->inodes[i];
            index_inode = i;
            break;
        }
    }

    if (!source_inode) {
        printf("Fichier source '%s' introuvable ou est un répertoire.\n", file_name);
        return 0;
    }

    // Vérifier si le fichier de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_file_path) == 0 && fs->inodes[i].is_file) {
            printf("Le fichier de destination '%s' existe déjà.\n", link_name);
            return 0;
        }
    }
    strcpy(fs->inodes[index_inode].name, full_file_path);
    // Sauvegarder le système de fichiers
    save_filesystem(fs);
    printf("Fichier '%s' renommé en '%s'.\n", file_name, link_name);
    return 1;
}

// Fonction rénommer un répertoire
int rename_directory(Filesystem *fs, const char *repnamedepart, const char *repnamefinal) {
    char full_path_source[MAX_FILENAME * 2];
    char full_file_path[MAX_FILENAME * 2];

    // Construire le chemin complet du fichier source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, repnamedepart);
    // Construire le chemin complet du fichier source
    snprintf(full_file_path, sizeof(full_file_path), "%s/%s", fs->current_directory, repnamefinal);

    // Rechercher le fichier source dans les inodes
    Inode *source_inode = NULL;
    int index_inode = -1;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && fs->inodes[i].is_directory) {
            source_inode = &fs->inodes[i];
            index_inode = i;
            break;
        }
    }

    if (!source_inode) {
        printf("Répertoire source '%s' introuvable.\n", repnamedepart);
        return 0;
    }

    // Vérifier si le fichier de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_file_path) == 0 && fs->inodes[i].is_directory) {
            printf("Le répertoire de destination '%s' existe déjà.\n", repnamefinal);
            return 0;
        }
    }
    strcpy(fs->inodes[index_inode].name, full_file_path);
    // Sauvegarder le système de fichiers
    save_filesystem(fs);
    printf("Répertoire '%s' renommé en '%s'.\n", repnamedepart, repnamefinal);
    return 1;
}

// Fonction pour effacer l'écran
void clear_screen() {
    #ifdef _WIN32
        system("cls"); // Pour Windows
    #else
        system("clear"); // Pour Unix/Linux
    #endif
}

// Fonction pour lister le contenu du répertoire avec leur métadonnées 
int list_all_directory(Filesystem *fs) {
    printf("Contenu de '%s':\n", fs->current_directory);

    int found = 0;
    int file_count = 0;
    int dir_count = 0;
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
            char modification_time[100];
            strftime(modification_time, sizeof(modification_time), "%Y-%m-%d %H:%M:%S", localtime(&fs->inodes[i].modification_time));
            if (fs->inodes[i].is_directory) {
                fs->inodes[i].size = calculate_directory_size_recursive(fs,fs->inodes[i].name); 
                printf("%s  %i  %s  %s  %d  %s  %lu  %s  \n", fs->inodes[i].permissions, fs->inodes[fs->inode_count].num_liens, fs->inodes[i].owner, fs->inodes[i].group, fs->inodes[i].size, modification_time, (unsigned long)fs->inodes[i].inode_number,  last_element(fs->inodes[i].name));
            } else {
                printf("%s  %i  %s  %s  %d  %s  %lu  %s  \n",fs->inodes[i].permissions, fs->inodes[fs->inode_count].num_liens, fs->inodes[i].owner, fs->inodes[i].group, fs->inodes[i].size, modification_time, (unsigned long)fs->inodes[i].inode_number, last_element(fs->inodes[i].name));
            }
            found = 1;
            // Compter les fichiers et répertoires
            if (fs->inodes[i].is_file) {
                file_count++;
            } else {
                dir_count++;
            }
        }
    }

    if (!found) {
        printf("Le répertoire est vide.\n");
        return 0;
    } else {
        printf("\nTotal : %d fichier(s), %d répertoire(s)\n", file_count, dir_count);
        return 1;
    }
    return 0;
}

// Fonction pour afficher l'utilisateur actuel
int list_user_groups(Filesystem *fs) {
    printf("Groupes de l'utilisateur '%s':\n", current_own);
    
    // Trouver l'utilisateur dans la table des groupes
    int user_found = 0;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            user_found = 1;
            if (fs->group[i].taille == 0) {
                printf("Aucun groupe disponible.\n");
                return 0;
            } else {
                for (int j = 0; j < fs->group[i].taille; j++) {
                    printf("- %s", fs->group[i].group[j].data);
                    if (strcmp(fs->group[i].group[j].data, current_group) == 0) {
                        printf(" (groupe actuel)");
                    }
                    printf("\n");
                }
            }
            break;
        }
    }
    
    if (!user_found) {
        printf("Erreur: utilisateur introuvable dans la table des groupes.\n");
        return 0;
    }
    return 1;
}

// Fonction pour changer de groupe
int change_group(Filesystem *fs, const char *groupname) {
    // Vérifier si le groupe est vide
    if (groupname == NULL || strlen(groupname) == 0) {
        printf("Erreur : nom de groupe invalide.\n");
        return 0;
    }

    // Trouver l'utilisateur actuel dans la table des groupes
    int user_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Erreur : utilisateur '%s' introuvable.\n", current_own);
        return 0;
    }

    // Vérifier si le groupe existe pour cet utilisateur
    int group_found = 0;
    for (int j = 0; j < fs->group[user_index].taille; j++) {
        if (strcmp(fs->group[user_index].group[j].data, groupname) == 0) {
            group_found = 1;
            break;
        }
    }

    if (!group_found) {
        printf("Erreur : le groupe '%s' n'existe pas pour l'utilisateur '%s'.\n", groupname, current_own);
        return 0;
    }
    
    memset(current_group, '\0', sizeof(current_group));
    // Changer le groupe actuel
    strncpy(current_group, groupname, GROUP_SIZE);
    printf("Groupe actuel changé pour '%s'.\n", groupname);
    save_filesystem(fs);
    return 1;
}

// Fonction pour afficher le groupe actuel
int show_current_group() {
    if (strlen(current_group) == 0) {
        printf("Aucun groupe n'est actuellement sélectionné.\n");
        return 0;
    } else {
        printf("Groupe actuel: %s\n", current_group);
        return 1;
    }
}

// Fonction pour supprimer uniquement son propre compte (même en sudo)
int delete_user_account(Filesystem *fs, const char *username) {
    // Vérifier si l'utilisateur actuel correspond au compte à supprimer
    if (strcmp(current_own, username) != 0) {
        printf("Erreur : Vous ne pouvez supprimer que votre propre compte.\n");
        return 0;
    }

    // Vérifier si l'utilisateur existe
    int user_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Erreur : l'utilisateur '%s' n'existe pas.\n", username);
        return 0;
    }

    // Supprimer le répertoire personnel
    snprintf(fs->current_directory, sizeof("./users/home"), "./users/home");
    delete_directory(fs, username);
    // Vérifier si l'utilisateur existe déjà dans la table des groupes
    int user_exists = 0;
    int inode = 0;

    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            user_exists = 1; // L'utilisateur existe
            inode = i; 
            break;
        }
    }
    if (user_exists) {
        strcpy(fs->group[inode].user, "\0");
    }



    strcpy(current_own, "\0");
    strcpy(current_group, "\0");
    // Déconnecter l'utilisateur
    printf("Compte '%s' supprimé. Déconnexion...\n", username);

    save_filesystem(fs);
    return 1;
}

// Fonction pour réinitialise le répertoire de travail d'un utilisateur (supprime tout sauf son dossier home)
int reset_user_workspace(Filesystem *fs, const char *username) {
    // Vérifier si l'utilisateur existe
    int user_exists = 0;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            user_exists = 1;
            break;
        }
    }

    if (!user_exists) {
        printf("Erreur : l'utilisateur '%s' n'existe pas.\n", username);
        return 0;
    }

    // Chemin du dossier home à préserver
    char user_home_path[MAX_PATH];
    snprintf(user_home_path, sizeof(user_home_path), "./users/home/%s", username);

    // Parcourir tous les fichiers/dossiers de l'utilisateur
    for (int i = 0; i < fs->inode_count; ) {
        // Si le fichier/dossier appartient à l'utilisateur ET n'est pas son dossier home
        if (strcmp(fs->inodes[i].owner, username) == 0) {
            strncpy(fs->current_directory, user_home_path, sizeof(user_home_path));
            // Supprimer le fichier ou le répertoire
            if (fs->inodes[i].is_directory) {
                delete_directory(fs, last_element(fs->inodes[i].name));
            } else {
                delete_file(fs, last_element(fs->inodes[i].name));
            }
            // Ne pas incrémenter `i` car la suppression réduit `fs->inode_count`
        } else {
            i++; // Passer au suivant
        }
    }

    printf("Répertoire de travail de '%s' réinitialisé (dossier home conservé).\n", username);
    save_filesystem(fs);
    return 1;
}

// Fonction pour afficher le mot de passe de l'utilisateur actuel
int show_password(Filesystem *fs) {
    // Trouver l'utilisateur dans la table des groupes
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            printf("Mot de passe de %s: %s\n", current_own, fs->group[i].password);
            return 1;
        }
    }
    printf("Utilisateur non trouvé.\n");
    return 0;
}

// Fonction pour modifier le mot de passe de l'utilisateur actuel
int change_password(Filesystem *fs) {
    char current_password[MAX_PASSWORD];
    char new_password[MAX_PASSWORD];
    char confirm_password[MAX_PASSWORD];

    // Demander le mot de passe actuel
    printf("Entrez votre mot de passe actuel: ");
    fgets(current_password, MAX_PASSWORD, stdin);
    current_password[strcspn(current_password, "\n")] = '\0';

    // Trouver l'utilisateur dans la table des groupes
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            // Vérifier le mot de passe actuel
            if (strcmp(fs->group[i].password, current_password) != 0) {
                printf("Mot de passe incorrect.\n");
                return 0;
            }

            // Demander le nouveau mot de passe
            printf("Entrez votre nouveau mot de passe: ");
            fgets(new_password, MAX_PASSWORD, stdin);
            new_password[strcspn(new_password, "\n")] = '\0';

            // Confirmer le nouveau mot de passe
            printf("Confirmez votre nouveau mot de passe: ");
            fgets(confirm_password, MAX_PASSWORD, stdin);
            confirm_password[strcspn(confirm_password, "\n")] = '\0';

            // Vérifier que les nouveaux mots de passe correspondent
            if (strcmp(new_password, confirm_password) != 0) {
                printf("Les mots de passe ne correspondent pas.\n");
                return 0;
            }

            // Vérifier que le nouveau mot de passe est différent de l'ancien
            if (strcmp(new_password, current_password) == 0) {
                printf("Le nouveau mot de passe doit être différent de l'actuel.\n");
                return 0;
            }

            // Changer le mot de passe
            strncpy(fs->group[i].password, new_password, MAX_PASSWORD);
            fs->group[i].password[MAX_PASSWORD - 1] = '\0';
            save_filesystem(fs);
            printf("Mot de passe modifié avec succès.\n");
            return 1;
        }
    }
    printf("Utilisateur non trouvé.\n");
    return 0;
}

// Fonction pour vérifier le mot de passe
int verify_password(Filesystem *fs, const char *password) {
    // Vérifier si le mot de passe est NULL ou vide
    if (password == NULL || strlen(password) == 0) {
        return 0; // Mot de passe invalide
    }
    // Parcourir la table des utilisateurs pour trouver l'utilisateur actuel
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            // Comparer le mot de passe fourni avec celui stocké
            if (strcmp(fs->group[i].password, password) == 0) {
                return 1; // Les mots de passe correspondent
            } else {
                return 0; // Les mots de passe ne correspondent pas
            }
        }
    }

    // Si l'utilisateur n'est pas trouvé
    return 0;
}

// Fonction pour vérifier le mot de passe sudo
int verify_sudo_password(Filesystem* fs, const char* current_own) {
    printf("[sudo] Mot de passe pour %s: ", current_own);
    char password[MAX_PASSWORD];
    fgets(password, MAX_PASSWORD, stdin);
    password[strcspn(password, "\n")] = '\0';
    
    if (!verify_password(fs, password)) {
        printf("[sudo] Mot de passe incorrect\n");
        return 0;
    }
    sudo = 1;
    return 1;
}

// Fonction pour vérifier si l'utilisateur est un administrateur
int is_user_admin(Filesystem *fs, const char *username) {
    // Parcourir la table des utilisateurs
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            // L'utilisateur a été trouvé, retourne le flag is_admin
            return fs->group[i].is_admin;
        }
    }
    // Si l'utilisateur n'est pas trouvé, affiche un message d'erreur
    printf("Erreur : utilisateur '%s' introuvable.\n", username);
    return 0;
}

// Fonction pour vérifier si l'utilisateur est un super administrateur
int is_user_superadmin(Filesystem *fs, const char *username) {

    // Vérifier si l'utilisateur actuel est root et que le mot de passe est correct
    int root_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (fs->group[i].is_root == 1 && strcmp(fs->group[i].user, username) == 0) {
            root_index = i;
            break;
        }
    }

    if (root_index == -1) {
        printf("Erreur : L'utilisateur actuel n'est pas root.\n");
        return 0;
    }
    
    printf("[root] Mot de passe pour %s: ", username);
    char password[MAX_PASSWORD];
    fgets(password, MAX_PASSWORD, stdin);
    password[strcspn(password, "\n")] = '\0';

    

    // Vérifier le mot de passe de root
    if (strcmp(fs->group[root_index].root_pwd, password) != 0) {
        printf("Erreur : Mot de passe incorrect.\n");
        return 0;
    }
    
    // Parcourir la table des utilisateurs
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            // L'utilisateur a été trouvé, retourne le flag is_root
            return fs->group[i].is_root;
        }
    }
    // Si l'utilisateur n'est pas trouvé, affiche un message d'erreur
    printf("Erreur : utilisateur '%s' introuvable.\n", username);
    return 0;
}

// Fonction pour ajouter un utilisateur à un groupe existant
int add_user_to_group(Filesystem *fs, const char *username, const char *groupname) {
    // Vérifications de base
    if (username == NULL || groupname == NULL || strlen(username) == 0 || strlen(groupname) == 0) {
        printf("Erreur : Nom d'utilisateur ou de groupe invalide.\n");
        return 0;
    }

    // Vérifier que l'utilisateur courant a les droits (propriétaire ou sudo)
    int is_owner_or_sudo = 0;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            for (int j = 0; j < fs->group[i].taille; j++) {
                if (strcmp(fs->group[i].group[j].data, groupname) == 0) {
                    is_owner_or_sudo = 1;
                    break;
                }
            }
            break;
        }
    }

    if (!is_owner_or_sudo && !sudo) {
        printf("Erreur : Vous n'avez pas les droits pour modifier ce groupe.\n");
        printf("Utilisez 'sudo add %s %s' si nécessaire.\n", username, groupname);
        return 0;
    }

    // Vérifier que le groupe existe
    char group_path[MAX_PATH];
    snprintf(group_path, sizeof(group_path), "./users/groups/%s", groupname);
    
    int group_exists = 0;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, group_path) == 0 && fs->inodes[i].is_directory) {
            group_exists = 1;
            break;
        }
    }

    if (!group_exists) {
        printf("Erreur : Le groupe '%s' n'existe pas.\n", groupname);
        return 0;
    }

    // Trouver l'utilisateur à ajouter
    int user_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Erreur : L'utilisateur '%s' n'existe pas.\n", username);
        return 0;
    }

    // Vérifier si l'utilisateur est déjà dans le groupe
    for (int j = 0; j < fs->group[user_index].taille; j++) {
        if (strcmp(fs->group[user_index].group[j].data, groupname) == 0) {
            printf("L'utilisateur '%s' fait déjà partie du groupe '%s'.\n", username, groupname);
            return 0;
        }
    }

    // Ajouter le groupe à l'utilisateur
    if (fs->group[user_index].taille < GROUP_SIZE) {
        strncpy(fs->group[user_index].group[fs->group[user_index].taille].data, groupname, MAX_FILENAME);
        fs->group[user_index].taille++;
        
        printf("Utilisateur '%s' ajouté au groupe '%s'.\n", username, groupname);
        save_filesystem(fs);
        return 1;
    } else {
        printf("Erreur : L'utilisateur '%s' a atteint le nombre maximal de groupes (%d).\n",  username, GROUP_SIZE);
        return 0;
    }
    return 0; // Si aucune des conditions n'est remplie, retourner 0
}

// Fonction pour afficher les membres d'un groupe
int list_group_members(Filesystem *fs, const char *groupname) {
    // Vérifications de base
    if (groupname == NULL || strlen(groupname) == 0) {
        printf("Erreur : Nom de groupe invalide.\n");
        return 0;
    }

    // Vérifier que le groupe existe
    char group_path[MAX_PATH];
    snprintf(group_path, sizeof(group_path), "./users/groups/%s", groupname);
    
    int group_exists = 0;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, group_path) != 0 && strcmp(current_own, groupname) == 0) {
            group_exists = 1;
            break;
        }
        else if (strcmp(fs->inodes[i].name, group_path) == 0 && fs->inodes[i].is_directory) {
            group_exists = 1;
            break;
        }
    }

    if (!group_exists) {
        printf("Erreur : Le groupe '%s' n'existe pas.\n", groupname);
        return 0;
    }

    // Parcourir tous les utilisateurs pour trouver ceux dans le groupe
    printf("Membres du groupe '%s':\n", groupname);
    int member_count = 0;
    
    for (int i = 0; i < NUM_USER; i++) {
        if (fs->group[i].user[0] != '\0') { // Si l'utilisateur existe
            for (int j = 0; j < fs->group[i].taille; j++) {
                if (strcmp(fs->group[i].group[j].data, groupname) == 0) {
                    printf("- %s", fs->group[i].user);
                    if (strcmp(fs->group[i].user, current_own) == 0) {
                        printf(" (vous)");
                    }
                    printf("\n");
                    member_count++;
                    break;
                }
            }
        }
    }

    if (member_count == 0) {
        printf("Aucun membre dans ce groupe.\n");
        return 0;
    } else {
        printf("Total: %d membre(s)\n", member_count);
        return 1;
    }
    return 0; // Si aucune des conditions n'est remplie, retourner 0
}

// Fonction pour afficher les informations d'un utilisateur
int show_user_info(Filesystem *fs,const char *utilisateur) {
    if (utilisateur == NULL) {
        printf("Erreur : utilisateur non trouvé.\n");
        return 0;
    }
    // Vérifier si l'utilisateur actuel est root et que le mot de passe est correct
    User_Group *utilisa_teur = NULL;
    int is_root = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, utilisateur) == 0) {
            utilisa_teur = &fs->group[i];
            break;
        }
    }
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            is_root = 1;
            break;
        }else {
            is_root = 0;
        }
    }
    
    if (utilisa_teur == NULL) {
        printf("Erreur : utilisateur '%s' non trouvé.\n", utilisateur);
        return 0;
    }

    if (is_root || utilisa_teur->user == current_own) {   
        printf("----- Informations sur l'utilisateur -----\n");
        printf("Nom d'utilisateur : %s\n", utilisa_teur->user);   
        printf("Est root          : %s\n", utilisa_teur->is_root ? "Oui" : "Non");
        printf("Est admin         : %s\n", utilisa_teur->is_admin ? "Oui" : "Non");
        printf("Groupes associés  : ");
        if (utilisa_teur->taille > 0) {
            for (int i = 0; i < utilisa_teur->taille; i++) {
                printf("%s ", utilisa_teur->group[i].data);
            }
            printf("\n");
        } else {
            printf("Aucun\n");
        }
    } 
    if (strcmp(utilisa_teur->user, current_own)== 0) {
        printf("Mot de passe      : %s\n", utilisa_teur->password);
        printf("Mot de passe root : %s\n", utilisa_teur->root_pwd);
    } else {
        printf("------------------------------------------\n");
    }
    printf("------------------------------------------\n");
    return 1;
}

// Fonction pour retirer un utilisateur d'un groupe
int remove_user_from_group(Filesystem *fs, const char *username, const char *groupname) {
    // Vérifications de base
    if (username == NULL || groupname == NULL || strlen(username) == 0 || strlen(groupname) == 0) {
        printf("Erreur : Nom d'utilisateur ou de groupe invalide.\n");
        return 0;
    }

    // Vérifier que l'utilisateur courant a les droits (sudo ou propriétaire du groupe)
    int is_owner_or_sudo = 0;
    char group_path[MAX_PATH];
    snprintf(group_path, sizeof(group_path), "./users/groups/%s", groupname);

    // Vérifier si l'utilisateur est propriétaire du groupe
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, group_path) == 0 &&   fs->inodes[i].is_directory &&  strcmp(fs->inodes[i].owner, current_own) == 0) {
            is_owner_or_sudo = 1;
            break;
        }
    }

    if (!is_owner_or_sudo && !sudo) {
        printf("Erreur : Vous n'avez pas les droits pour modifier ce groupe.\n");
        printf("Utilisez 'sudo remove %s %s' si nécessaire.\n", username, groupname);
        return 0;
    }

    // Vérifier que le groupe existe
    int group_exists = 0;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, group_path) == 0 && fs->inodes[i].is_directory) {
            group_exists = 1;
            break;
        }
    }

    if (!group_exists) {
        printf("Erreur : Le groupe '%s' n'existe pas.\n", groupname);
        return 0;
    }

    // Trouver l'utilisateur à retirer
    int user_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Erreur : L'utilisateur '%s' n'existe pas.\n", username);
        return 0;
    }

    // Vérifier si l'utilisateur est dans le groupe
    int group_index = -1;
    for (int j = 0; j < fs->group[user_index].taille; j++) {
        if (strcmp(fs->group[user_index].group[j].data, groupname) == 0) {
            group_index = j;
            break;
        }
    }

    if (group_index == -1) {
        printf("L'utilisateur '%s' ne fait pas partie du groupe '%s'.\n", username, groupname);
        return 0;
    }

    // Cas spécial : on ne peut pas se retirer soi-même du groupe si c'est notre seul groupe
    if (strcmp(username, current_own) == 0 && fs->group[user_index].taille <= 1) {
        printf("Erreur : Vous ne pouvez pas quitter votre dernier groupe.\n");
        return 0;
    }

    // Retirer le groupe de la liste de l'utilisateur
    for (int k = group_index; k < fs->group[user_index].taille - 1; k++) {
        strcpy(fs->group[user_index].group[k].data, fs->group[user_index].group[k+1].data);
    }
    fs->group[user_index].taille--;

    // Si c'était le groupe actuel, le réinitialiser
    if (strcmp(current_group, groupname) == 0 && 
        strcmp(username, current_own) == 0) {
        strcpy(current_group, "");

    }

    printf("Utilisateur '%s' retiré du groupe '%s'.\n", username, groupname);
    save_filesystem(fs);
    return 1;
}

// Fonction pour promouvoir un utilisateur au rôle d'admin
int promote_to_admin(Filesystem *fs, const char *username) {
    // Vérifier si l'utilisateur actuel est root et que le mot de passe est correct
    int root_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (fs->group[i].is_root == 1 && strcmp(fs->group[i].user, current_own) == 0) {
            root_index = i;
            break;
        }
    }

    if (root_index == -1) {
        printf("Erreur : L'utilisateur actuel n'est pas root.\n");
        return 0;
    }

    // Chercher l'utilisateur spécifié dans le système de fichiers
    int user_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Erreur : L'utilisateur '%s' n'existe pas.\n", username);
        return 0;
    }

    // Promouvoir l'utilisateur en admin
    if (fs->group[user_index].is_admin == 1) {
        printf("L'utilisateur '%s' est déjà un administrateur.\n", username);
        return 0;
    } else {
        fs->group[user_index].is_admin = 1;
        printf("L'utilisateur '%s' a été promu au rôle d'administrateur.\n", username);
        save_filesystem(fs);
        return 1;
    }
    return 0; // Si aucune des conditions n'est remplie, retourner 0
}

// Fonction pour retirer le rôle d'admin d'un utilisateur
int demote_from_admin(Filesystem *fs, const char *username) {
    // Vérifier si l'utilisateur actuel est root
    int root_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (fs->group[i].is_root == 1 && strcmp(fs->group[i].user, current_own) == 0) {
            root_index = i;
            break;
        }
    }

    if (root_index == -1) {
        printf("Erreur : L'utilisateur actuel n'est pas root.\n");
        return 0;
    }

    // Chercher l'utilisateur spécifié dans le système de fichiers
    int user_index = -1;
    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, username) == 0) {
            user_index = i;
            break;
        }
    }

    if (user_index == -1) {
        printf("Erreur : L'utilisateur '%s' n'existe pas.\n", username);
        return 0;
    }

    // Vérifier si l'utilisateur est bien administrateur
    if (fs->group[user_index].is_admin == 0) {
        printf("L'utilisateur '%s' n'est pas un administrateur.\n", username);
        return 0;
    } else {
        // Retirer le rôle d'administrateur
        fs->group[user_index].is_admin = 0;
        printf("L'utilisateur '%s' a été retiré de la fonction d'administrateur.\n", username);
        save_filesystem(fs);
        return 1;
    }
    return 0; // Si aucune des conditions n'est remplie, retourner 0
}

// Fonction pour enregistrer une trace d'exécution dans le fichier trace_execution.txt
void save_trace_execution(Filesystem *fs, const char *current_own, const char *current_group,const char *command, char success) {
    FILE *trace_file = fopen(TRACE_FILE, "a"); // Ouvrir le fichier en mode "append"
    
    if (trace_file == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier de trace d'exécution.\n");
        return;
    }

    // Obtenir l'heure actuelle
    time_t current_time = time(NULL);
    struct tm *time_info = localtime(&current_time);

    // Formater la date et l'heure (ex: 2025-04-03 14:45:20)
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    // Déterminer le rôle de l'utilisateur (root/admin)
    char role[10];
    if (fs->group[0].is_root && strcmp(fs->group[0].user, current_own) == 0) {
        strcpy(role, "root");
    } else if (fs->group[0].is_admin && strcmp(fs->group[0].user, current_own) == 0) {
        strcpy(role, "admin");
    } else {
        strcpy(role, "user");
    }

    // Enregistrer la trace d'exécution dans le fichier de log
    fprintf(trace_file, "Date: [%s], Utilisateur: [%s], Groupe: [%s], Role: [%s], Succes: [%c], Commande: [%s]\n", time_str, current_own,current_group, role, success, command);

    // Fermer le fichier
    fclose(trace_file);
}

// Fonction pour lire le fichier de trace et filtrer selon les droits d'accès de l'utilisateur
int  read_trace_by_user(Filesystem *fs, const char *current_own) {
    FILE *trace_file = fopen(TRACE_FILE, "r"); // Ouvrir le fichier en mode lecture

    if (trace_file == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier de trace d'exécution pour lecture.\n");
        return 0;
    }

    char line[256]; // Variable pour stocker chaque ligne du fichier

    // Vérifier si l'utilisateur est un superadmin
    int is_superadmin = is_user_superadmin(fs, current_own);
    int show_all = 0;

    // Si l'utilisateur est un superadmin, on lui demande son choix
    if (is_superadmin) {
        char choix;
        printf("Voulez-vous afficher toutes les lignes ? [O/N] : ");
        scanf(" %c", &choix);
        // On vide le buffer d'entrée pour éviter d'éventuels problèmes
        while(getchar() != '\n');
        if (choix == 'O' || choix == 'o') {
            show_all = 1;
        } else if (choix == 'N' || choix == 'n') {
            show_all = 0;
        } else {
            printf("Choix invalide. Vous devez entrer 'O' ou 'N'.\n");
            fclose(trace_file);
        }
    }

    // Lire chaque ligne du fichier
    while (fgets(line, sizeof(line), trace_file)) {
        // Si l'utilisateur est un superadmin, afficher toutes les lignes
        if (is_superadmin && show_all) {
            printf("%s", line);
        } else {
            // Si l'utilisateur n'est pas un superadmin, vérifier s'il correspond à la ligne
            char utilisateur[50];
            if (sscanf(line, "Date: [%*[^]]], Utilisateur: [%49[^]]]", utilisateur) == 1) {
                // Si le nom de l'utilisateur dans la ligne correspond à current_own, afficher la ligne
                if (strcmp(utilisateur, current_own) == 0) {
                    printf("%s", line);
                }
            }
        }
    }

    // Fermer le fichier
    fclose(trace_file);
    return 1;
}

//=============================================================================
//=============================================================================

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
    //printf("Fichier ou répertoire '%s' introuvable.\n", filename);
    return NULL;
}

// Vérifie si un lien symbolique pointe vers un fichier spécifique
int is_symbolic_link_for(Filesystem *fs, const char *file_base, const char *symb_link) {
    // vérifier si le lien symbolique existe
    Inode *inod_lien = get_inode_by_name(fs, symb_link);
    if (inod_lien == NULL) {
        printf("Erreur : Lien symbolique '%s' introuvable.\n", symb_link);
        return 0;
    }
    // Vérifier si le fichier de base existe
    Inode *inod_file = get_inode_by_name(fs, file_base);
    if (inod_file == NULL) {
        printf("Erreur : Fichier de base '%s' introuvable.\n", file_base);
        return 0;
    }

    for (int j = 0; j < NUM_LIEN_MAX; j++) {
        if (strcmp(inod_file->lien.symbolicLink[j].data, inod_lien->name) == 0) {
            return 1; // Le lien symbolique pointe bien vers le fichier
        }
    }
    return 0; // Lien non trouvé ou ne pointe pas vers le fichier
}

// Vérifie si un fichier est un hardlink d'un autre fichier
int is_hard_link_for(Filesystem *fs, const char *file_base, const char *hard_link) {
    // vérifier si le lien symbolique existe
    Inode *inod_lien = get_inode_by_name(fs, hard_link);
    if (inod_lien == NULL) {
        printf("Erreur : Lien matériel '%s' introuvable.\n", hard_link);
        return 0;
    }
    // Vérifier si le fichier de base existe
    Inode *inod_file = get_inode_by_name(fs, file_base);
    if (inod_file == NULL) {
        printf("Erreur : Fichier de base '%s' introuvable.\n", file_base);
        return 0;
    }

    for (int j = 0; j < NUM_LIEN_MAX; j++) {
        if (strcmp(inod_file->lien.hardLink[j].data, inod_lien->name) == 0) {
            return 1; // Le lien symbolique pointe bien vers le fichier
        }
    }
    return 0; // Lien non trouvé ou ne pointe pas vers le fichier
}

// Fonction pour trouver le fichier de base d'un lien symbolique
char* get_symbolic_link_target(Filesystem *fs, const char *symb_link) {
    // Construire le chemin complet du lien symbolique
    char full_link_path[MAX_FILENAME * 2];
    snprintf(full_link_path, sizeof(full_link_path), "%s/%s", fs->current_directory, symb_link);

    // Parcourir tous les inodes pour trouver le lien symbolique
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_link_path) == 0 && fs->inodes[i].is_link) {
            // Ceci est le lien symbolique - trouver le fichier qui le référence
            for (int j = 0; j < fs->inode_count; j++) {
                // Vérifier tous les fichiers qui pourraient pointer vers ce lien
                if (fs->inodes[j].is_file) {
                    for (int k = 0; k < NUM_LIEN_MAX; k++) {
                        if (strcmp(fs->inodes[j].lien.symbolicLink[k].data, full_link_path) == 0) {
                            return fs->inodes[j].name; // Retourner le nom du fichier original
                        }
                    }
                }
            }
        }
    }
    
    printf("Erreur : Lien symbolique '%s' introuvable ou ne pointe vers aucun fichier.\n", symb_link);
    return NULL;
}

// Fonction pour trouver le fichier original d'un hardlink
char* get_hardlink_original(Filesystem *fs, const char *hard_link) {
    // Construire le chemin complet du hardlink
    char full_link_path[MAX_FILENAME * 2];
    snprintf(full_link_path, sizeof(full_link_path), "%s/%s", fs->current_directory, hard_link);

    // Vérifier si le hardlink existe
    Inode *link_inode = NULL;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_link_path) == 0) {
            link_inode = &fs->inodes[i];
            break;
        }
    }

    if (link_inode == NULL) {
        printf("Erreur : Hardlink '%s' introuvable.\n", hard_link);
        return NULL;
    }

    // Trouver l'original en cherchant un fichier avec le même numéro d'inode
    for (int i = 0; i < fs->inode_count; i++) {
        if (fs->inodes[i].inode_number == link_inode->inode_number && 
            strcmp(fs->inodes[i].name, full_link_path) != 0) {
            // On a trouvé un fichier avec le même inode_number mais un nom différent
            return fs->inodes[i].name;
        }
    }

    // Si on arrive ici, c'est que le hardlink est peut-être l'original lui-même
    // ou qu'il n'y a pas d'autre fichier avec le même inode_number
    printf("Avertissement : Aucun fichier original trouvé pour le hardlink '%s'.\n", hard_link);
    return NULL;
}

//=============================================================================
//=============================================================================

// Fonction pour créer un lien symbolique
int create_symbolic_link(Filesystem *fs, const char *file_name, const char *link_name,  const char *rep_name) {
    // Vérifier si on a atteint le nombre maximal de fichiers
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return 0;
    }

    // Construire les chemins complets
    char full_file_path[MAX_FILENAME * 2];
    char full_link_path[MAX_FILENAME * 2];
    char dest_directory[MAX_FILENAME * 2];


    char prevent_path[MAX_FILENAME * 2];
    strncpy(prevent_path, fs->current_directory, MAX_FILENAME);

    // Construire le chemin complet du fichier source
    snprintf(full_file_path, sizeof(full_file_path), "%s/%s", fs->current_directory, file_name);

    if (rep_name != NULL) {
        // Vérifier si le rep_name est un chemin complet ou un répertoire relatif
        if (strchr(rep_name, '/') != NULL) {
            char exists_path[MAX_FILENAME * 2 - 2];
            // C'est un répertoire relatif au répertoire courant
            snprintf(exists_path, sizeof(exists_path), "%s/%s", fs->current_directory, rep_name);
            // C'est un chemin complet
            if (!directory_exists(fs, exists_path)) {
                printf("Le répertoire '%s' n'existe pas.\n", rep_name);
                return 0;
            }

            snprintf(full_link_path, sizeof(full_link_path), "%s/%s", exists_path, link_name);
        } else if (strcmp(rep_name, "..") == 0) {
            snprintf(full_link_path, sizeof(full_link_path), "%s/%s", retirer_suffixe(fs->current_directory), link_name); // Chemin relatif au répertoire parent
        } else {
            // C'est un répertoire relatif au répertoire courant
            snprintf(dest_directory, sizeof(dest_directory), "%s/%s", fs->current_directory, rep_name);
            if (!directory_exists(fs, dest_directory)) {
                printf("Le répertoire '%s' n'existe pas dans le répertoire courant.\n", rep_name);
                return 0;
            }
            snprintf(full_link_path, sizeof(full_link_path), "%s/%s/%s", fs->current_directory, rep_name, link_name);
        }
    } else {
        snprintf(full_link_path, sizeof(full_link_path), "%s/%s", fs->current_directory, link_name);
    }

    // Vérifier si le fichier source existe
    Inode *source_inode = NULL;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_file_path) == 0) {
            source_inode = &fs->inodes[i];
            break;
        }
    }

    if (source_inode == NULL) {
        printf("Erreur : le fichier source '%s' n'existe pas.\n", full_file_path);
        return 0;
    }

    // Vérifier si le lien symbolique existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_link_path) == 0) {
            printf("Erreur : un fichier ou lien existe déjà avec le nom '%s'.\n", link_name);
            return 0;
        }
    }

    // Vérifier les permissions sur le fichier source
    if (strcmp(source_inode->owner, current_own) != 0 && 
        strcmp(source_inode->group, current_group) != 0 && 
        source_inode->permissions[7] != 'r') {  // Permission 'others read'
        printf("Erreur : permissions insuffisantes sur le fichier source '%s'.\n", file_name);
        return 0;
    }

    // Créer le nouvel inode pour le lien symbolique
    Inode *link_inode = &fs->inodes[fs->inode_count];
    
    strcpy(link_inode->name, full_link_path);
    link_inode->is_directory = 0;
    link_inode->is_file = 0;
    link_inode->is_link = 1;
    link_inode->size = source_inode->size; // Taille du fichier source
    link_inode->block_count = source_inode->block_count; // Nombre de blocs du fichier source
    link_inode->inode_number = source_inode->inode_number; // Numéro d'inode du fichier source
    link_inode->parent_inode_number = link_inode->inode_number; // Numéro d'inode du parent
    
    // Métadonnées
    time_t now = time(NULL);
    link_inode->creation_time = now;
    link_inode->modification_time = now;
    link_inode->num_liens = 0; // Initialiser le nombre de liens à 0

    
    strncpy(link_inode->owner, source_inode->owner, NAME_SIZE);
    strncpy(link_inode->group, source_inode->group, GROUP_SIZE);
    strcpy(link_inode->permissions, "lrwx------"); // Permissions par défaut pour les liens
    
    // Initialiser les liens
    for (int i = 0; i < NUM_LIEN_MAX; i++) {
        memset(link_inode->lien.hardLink[i].data, 0, MAX_FILENAME);
        memset(link_inode->lien.symbolicLink[i].data, 0, MAX_FILENAME);
    }

    // Ajouter le lien symbolique au fichier source
    int added = 0;
    for (int i = 0; i < NUM_LIEN_MAX && !added; i++) {
        if (strlen(source_inode->lien.symbolicLink[i].data) == 0) {
            strncpy(source_inode->lien.symbolicLink[i].data, full_link_path, MAX_FILENAME);
            added = 1;
            source_inode->num_liens++;
        }
    }

    if (!added) {
        printf("Erreur : nombre maximum de liens symboliques atteint pour ce fichier.\n");
        return 0;
    }

    fs->inode_count++;
    source_inode->num_liens++; // Incrémenter le nombre de blocs du fichier source
    save_filesystem(fs);
    strncpy( fs->current_directory,prevent_path, MAX_FILENAME);
    
    printf("Lien symbolique '%s' créé vers '%s'.\n", full_link_path, full_file_path);
    return 1;
}

//Fonction pour lire un lien symbolique
int read_symbolic_link(Filesystem *fs, const char *link_name) {
    // Vérifier si le lien symbolique existe
    Inode *link_inode = get_inode_by_name(fs, link_name);
    if (link_inode == NULL || !link_inode->is_link) {
        printf("Erreur : Lien symbolique '%s' introuvable.\n", link_name);
        return 0;
    }

    char *file_inode = get_symbolic_link_target(fs, link_name);
    if (file_inode == NULL) {
        printf("Erreur : Fichier pointé par le lien symbolique introuvable.\n");
        return 0;
    }

    int inode_index = link_inode->inode_number;
    // Vérifier si le lien symbolique pointe vers un fichier
    if (link_inode->is_link) {
        // Parcourir tous les inodes pour trouver le fichier pointé par le lien
        for (int i = 0; i < fs->inode_count; i++) {
            if (fs->inodes[i].is_file && fs->inodes[i].inode_number == inode_index) {
                read_file(fs,last_element(fs->inodes[i].name)); // Appeler la fonction de lecture de fichier
                return 1;
            }
        }
    } else {
        printf("Erreur : '%s' n'est pas un lien symbolique.\n", link_name);
        return 0;
    }
    return 0; // Si aucune des conditions n'est remplie, retourner 0
}

// Fonction pour supprimer un lien symbolique
int delete_symbolic_link(Filesystem *fs, const char *link_name) {
    // Vérifier si le lien symbolique existe
    Inode *link_inode = get_inode_by_name(fs, link_name);
    if (link_inode == NULL || !link_inode->is_link) {
        printf("Erreur : Lien symbolique '%s' introuvable.\n", link_name);
        return 0;
    }

    // Supprimer le lien symbolique de l'inode source
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, link_inode->name) == 0) {
            for (int j = 0; j < NUM_LIEN_MAX; j++) {
                if (strcmp(fs->inodes[i].lien.symbolicLink[j].data, link_name) == 0) {
                    memset(fs->inodes[i].lien.symbolicLink[j].data, 0, MAX_FILENAME);
                    fs->inodes[i].num_liens--;
                    break;
                }
            }
            break;
        }
    }

    // Supprimer l'inode du lien symbolique
    memset(link_inode, 0, sizeof(Inode)); // Réinitialiser l'inode du lien symbolique
    fs->inode_count--;

    save_filesystem(fs);
    printf("Lien symbolique '%s' supprimé.\n", link_name);
    return 1;
}

// Fonction pour écrire dépuis un lien symbolique
int write_symbolic_link(Filesystem *fs, const char *link_name, const char *data) {
    // Vérifier si le lien symbolique existe
    Inode *link_inode = get_inode_by_name(fs, link_name);
    if (link_inode == NULL || !link_inode->is_link) {
        printf("Erreur : Lien symbolique '%s' introuvable.\n", link_name);
        return 0;
    }

    // Vérifier si le lien symbolique pointe vers un fichier
    if (link_inode->is_link) {
        // Parcourir tous les inodes pour trouver le fichier pointé par le lien
        for (int i = 0; i < fs->inode_count; i++) {
            if (fs->inodes[i].inode_number == link_inode->inode_number) {
                write_to_file(fs, last_element(fs->inodes[i].name), data); // Appeler la fonction d'écriture de fichier
                return 1;
            }
        }
    } else {
        printf("Erreur : '%s' n'est pas un lien symbolique.\n", link_name);
        return 0;
    }
    return 0; // Si aucune des conditions n'est remplie, retourner 0
}

// Fonction pour déplacer un lien symbolique
int move_symbolic_link(Filesystem *fs, const char *linkname, const char *rep_name) {
    char full_path_source[MAX_FILENAME * 2];
    char full_link_path[MAX_FILENAME * 2];
    char dest_directory[MAX_FILENAME * 2];
    char prevent_path[MAX_FILENAME * 2];
    strncpy(prevent_path, fs->current_directory, MAX_FILENAME);

    // Construire le chemin complet du lien source
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, linkname);

    // Vérifier si le rep_name est un chemin complet ou un répertoire relatif
    if (strchr(rep_name, '/') != NULL) {
        char exists_path[MAX_FILENAME * 2-2];
        snprintf(exists_path, sizeof(exists_path), "%s/%s", fs->current_directory, rep_name);
        if (!directory_exists(fs, exists_path)) {
            printf("Le répertoire '%s' n'existe pas.\n", rep_name);
            return 0;
        }
        snprintf(full_link_path, sizeof(full_link_path), "%s/%s", exists_path, linkname);
    } else if (strcmp(rep_name, "..") == 0) {
        snprintf(full_link_path, sizeof(full_link_path), "%s/%s", retirer_suffixe(fs->current_directory), linkname);
    } else {
        snprintf(dest_directory, sizeof(dest_directory), "%s/%s", fs->current_directory, rep_name);
        if (!directory_exists(fs, dest_directory)) {
            printf("Le répertoire '%s' n'existe pas dans le répertoire courant.\n", rep_name);
            return 0;
        }
        snprintf(full_link_path, sizeof(full_link_path), "%s/%s/%s", fs->current_directory, rep_name, linkname);
    }

    // Rechercher le lien symbolique source
    Inode *source_inode = NULL;
    int source_index = -1;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && fs->inodes[i].is_link) {
            source_inode = &fs->inodes[i];
            source_index = i;
            break;
        }
    }

    if (!source_inode) {
        printf("Lien symbolique '%s' introuvable ou n'est pas un lien.\n", linkname);
        return 0;
    }

    // Vérifier si la destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_link_path) == 0) {
            printf("Un fichier/lien existe déjà à '%s'.\n", full_link_path);
            return 0;
        }
    }

    
    // Copier les métadonnées du fichier source
    Inode *dest_inode = &fs->inodes[fs->inode_count];
    strcpy(dest_inode->name, full_link_path);
    dest_inode->is_directory = 0;
    dest_inode->is_file = 0;
    dest_inode->is_link = 1;
    dest_inode->inode_number = source_inode->inode_number; // Numéro d'inode du fichier source
    dest_inode->size = source_inode->size;
    dest_inode->creation_time = source_inode->creation_time;
    dest_inode->modification_time = time(NULL); // Mettre à jour la date de modification
    strncpy(dest_inode->owner, source_inode->owner, MAX_FILENAME);
    strncpy(dest_inode->permissions, source_inode->permissions, 10);
    strncpy(dest_inode->group, source_inode->group, GROUP_SIZE);
    // Incrémenter le nombre d'inodes
    fs->inode_count++;

    // Supprimer le fichier source
    for (int i = source_index; i < fs->inode_count - 1; i++) {
        fs->inodes[i] = fs->inodes[i + 1];
    }
    fs->inode_count--;

    save_filesystem(fs);
    printf("Lien symbolique '%s' déplacé vers '%s'.\n", linkname, full_link_path);
    strncpy(fs->current_directory, prevent_path, MAX_FILENAME);
    return 1;
}

// Fonction pour créer un lien matériel*****************************************************************
int create_hard_link(Filesystem *fs, const char *existing_file, const char *new_link) {
    char full_path_source[MAX_FILENAME * 2];
    char full_file_path[MAX_FILENAME * 2];

    // Construire les chemins complets
    snprintf(full_path_source, sizeof(full_path_source), "%s/%s", fs->current_directory, existing_file);
    snprintf(full_file_path, sizeof(full_file_path), "%s/%s", fs->current_directory, new_link);

    // Vérifier si le fichier source existe
    Inode *source_inode = NULL;
    //int source_index = -1;
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_path_source) == 0 && !fs->inodes[i].is_directory) {
            source_inode = &fs->inodes[i];
            //source_index = i;
            break;
        }
    }

    if (!source_inode) {
        printf("Fichier source '%s' introuvable ou est un répertoire.\n", existing_file);
        return 0;
    }

    // Vérifier si le lien de destination existe déjà
    for (int i = 0; i < fs->inode_count; i++) {
        if (strcmp(fs->inodes[i].name, full_file_path) == 0) {
            printf("Le fichier de destination '%s' existe déjà.\n", new_link);
            return 0;
        }
    }

    // Vérifier si le nombre maximal de liens est atteint
    if (source_inode->num_liens >= NUM_LIEN_MAX) {
        printf("Nombre maximal de liens atteint pour le fichier '%s'.\n", existing_file);
        return 0;
    }

    // Ajouter l'entrée du lien matériel dans la structure du fichier source
    strncpy(source_inode->lien.hardLink[source_inode->num_liens].data, full_file_path, MAX_FILENAME);
    source_inode->num_liens++;

    // Créer une nouvelle entrée d'inode pour le lien
    if (fs->inode_count >= MAX_FILES) {
        printf("Nombre maximum de fichiers atteint !\n");
        return 0;
    }

    // Copier toutes les métadonnées du fichier source
    Inode *dest_inode = &fs->inodes[fs->inode_count];
    strcpy(dest_inode->name, full_file_path);
    dest_inode->is_directory = 0;
    dest_inode->size = source_inode->size;
    dest_inode->creation_time = time(NULL); // Le lien a sa propre date de création
    dest_inode->modification_time = source_inode->modification_time;
    strncpy(dest_inode->owner, source_inode->owner, MAX_FILENAME);
    strncpy(dest_inode->permissions, source_inode->permissions, PERM_SIZE);
    strncpy(dest_inode->group, source_inode->group, GROUP_SIZE);
    dest_inode->num_liens = source_inode->num_liens;

    // Copier les références aux mêmes blocs de données (crucial pour les liens matériels)
    dest_inode->block_count = source_inode->block_count;
    for (int i = 0; i < source_inode->block_count; i++) {
        dest_inode->block_indices[i] = source_inode->block_indices[i];
    }

    // Copier les liens existants
    for (int i = 0; i < source_inode->num_liens; i++) {
        strncpy(dest_inode->lien.hardLink[i].data, source_inode->lien.hardLink[i].data, MAX_FILENAME);
    }

    // Ajouter le nouveau lien à tous les autres liens existants
    for (int i = 0; i < fs->inode_count; i++) {
        for (int j = 0; j < fs->inodes[i].num_liens; j++) {
            if (strcmp(fs->inodes[i].lien.hardLink[j].data, full_path_source) == 0) {
                // Ajouter le nouveau lien à cet inode également
                strncpy(fs->inodes[i].lien.hardLink[fs->inodes[i].num_liens].data, full_file_path, MAX_FILENAME);
                fs->inodes[i].num_liens++;
                break;
            }
        }
    }

    // Incrémenter le nombre d'inodes
    fs->inode_count++;

    // Sauvegarder le système de fichiers
    save_filesystem(fs);
    printf("Lien matériel '%s' créé pour le fichier '%s'.\n", new_link, existing_file);
    return 1;
}

// Fonction pour afficher l'aide
void help() {
    printf("\n=== Aide du système de fichiers ===\n\n");
    printf("Commandes de base :\n");
    printf("  help........................................Affiche cette aide\n");
    printf("  exit........................................Quitte le shell\n");
    printf("  clear.......................................Efface l'écran\n");
    printf("  whoami......................................Affiche l'utilisateur actuel\n");
    printf("  pwd.........................................Affiche le répertoire courant\n\n");

    printf("Gestion des répertoires :\n");
    printf("  mkdir <nom>.................................Crée un répertoire\n");
    printf("  rmdir <nom>.................................Supprime un répertoire\n");
    printf("  ls..........................................Liste le contenu du répertoire\n");
    printf("  lsl.........................................Liste avec métadonnées détaillées\n");
    printf("  statd <nom>.................................Affiche les métadonnées d'un répertoire\n");
    printf("  cd <nom>....................................Change de répertoire\n");
    printf("    cd .. ------------------------------------Remonte d'un niveau\n");
    printf("    cd rep------------------------------------Va dans le répertoire 'rep'\n");
    printf("    cd rep/sousrep/etc------------------------Chemin relatif\n\n");
    printf("  cpdir <src> <dest> [répertoire].............Copie un répertoire\n");
    printf("    cpdir <src> <dest>------------------------Copie un répertoire dans le répertoire actuel\n");
    printf("    cpdir <src> <dest> .. --------------------Copie un répertoire dans le répertoire parent\n");
    printf("    cpdir <src> <dest> rep--------------------Copie un répertoire dans le répertoire 'rep'\n");
    printf("    cpdir <src> <dest> rep/sousrep/etc--------Chemin relatif\n\n");
    printf("  mvdir <src> <répertoire>....................Déplace un répertoire\n");
    printf("    mvdir <src> .. ---------------------------Déplace un répertoire dans le répertoire parent\n");
    printf("    mvdir <src> rep---------------------------Déplace un répertoire dans le répertoire 'rep'\n");
    printf("    mvdir <src> rep/sousrep/etc---------------Chemin relatif\n\n");

    printf("Gestion des fichiers :\n");
    printf("  touch <nom>.................................Crée un fichier vide\n");
    printf("  statf <nom>.................................Affiche les métadonnées d'un fichier\n");
    printf("  write <nom> <cont>..........................Écrit dans un fichier\n");
    printf("  cat <nom>...................................Affiche le contenu d'un fichier\n");
    printf("  rm <nom>....................................Supprime un fichier\n");
    printf("  cp <src> <dest> [répertoire]................Copie un fichier\n");
    printf("    cp <src> <dest>---------------------------Copie un fichier dans un répertoire actuel\n");
    printf("    cp <src> <dest> .. -----------------------Copie un fichier dans un répertoire parent\n");
    printf("    cp <src> <dest> rep-----------------------Copie un fichier dans un répertoire 'rep'\n");
    printf("    cp <src> <dest> rep/sousrep/etc-----------Chemin relatif\n\n");
    printf("  mv <src> <répertoire>.......................Déplace/renomme un fichier\n");
    printf("    mv <src> .. ------------------------------Déplace un fichier dans le répertoire parent\n");
    printf("    mv <src> rep------------------------------Déplace un fichier dans le répertoire 'rep'\n");
    printf("    mv <src> rep/sousrep/etc------------------Chemin relatif\n\n");

    printf("Gestion des liens :\n");
    printf("  lns <cible> <lien>..........................crée un lien symbolic\n");
    printf("  writes <lien> <cont>........................Écrit dans un lien symbolique\n");
    printf("  reads <lien>................................Lit un lien symbolique\n");
    printf("  rms <lien>..................................Supprime un lien symbolique\n");
    printf("  mvs <lien> <rep>............................Déplace un lien symbolique\n");
    printf("  lssymlinks <fic>............................Liste les liens symboliques pointant vers le fichier\n\n");

    printf("  lnh <src> <dest>............................rée un lien matériel\n");
    printf("  writeh <lien> <cont>........................Écrit dans un lien matériel\n");
    printf("  readh <lien>................................Lit un lien matériel\n");
    printf("  rmh <lien>..................................Supprime un lien matériel\n");
    printf("  lshardlinks <fic>...........................Liste les liens matériels pointant vers le fichier\n\n");

    printf("Permissions :\n");
    printf("  chmodf <fichier> <cible> <perm>.............Modifie permissions fichier\n");
    printf("  chmodd <rep> <cible> <perm>.................Modifie permissions répertoire\n");
    printf("  -----(cibles: -Owner, -Group, -Others)\n");
    printf("  -----(perm: combinaison de rwx, ex: rw-)\n\n");

    printf("Gestion des groupes :\n");
    printf("  lsgroups....................................Liste les groupes de l'utilisateur\n");
    printf("  chgroup <nom>...............................Change le groupe actuel\n");
    printf("  curgroup....................................Affiche le groupe actuel\n");
    printf("  crtgroup <nom>..............................Crée un nouveau groupe\n");
    printf("  leavegroup <nom>............................Quitter un groupe\n");
    printf("  lsmembers <nom>.............................Liste les membres d'un groupe\n");
    printf("  sudo delgroup <nom>.........................Supprime un groupe (admin)\n");
    printf("  sudo add <nom> <pers>.......................Ajoute un utilisateur au groupe (admin)\n");
    printf("  sudo remove <pers> <nom>....................Retire un utilisateur du groupe (admin)\n\n");

    printf("Commandes administrateur (sudo) :\n");
    printf("  sudo passwd.................................Affiche le mot de passe (admin)\n");
    printf("  sudo chgpasswd..............................Change le mot de passe (admin)\n");
    printf("  sudo trace..................................Affiche la trace d'exécution (admin)\n");
    printf("  sudo deluser <nom>..........................Supprime un compte utilisateur (admin)\n");
    printf("  sudo resetuser <nom>........................Réinitialise un répertoire utilisateur (admin)\n\n");
    printf("  sudo addadmin <nom>.........................Ajoute un utilisateur en admin (superadmin)\n");
    printf("  sudo deladmin <nom>.........................Retire un utilisateur en admin (superadmin)\n\n");

    printf("Commandes utilisateur (sudo) :\n");
    printf("  sudo infuser <nom>.........................Affiche les informations de l'utilisateur (admin)\n\n");

    printf("Système :\n");
    printf("  free........................................Affiche les blocs libres\n\n");

    printf("Note : Les commandes admin nécessitent le mot de passe sudo\n");
}

// Fonction principale du shell
void shell(Filesystem *fs, char *current_own) {
    char command[100];
    char success;  // Variable pour déterminer le succès de la commande

    printf("\nBienvenue dans le système de fichiers %s!\n", current_own);
    //printf("Système de fichiers initialisé : %s\n", fs->current_directory);

    while (current_own) {
        printf("\n%s> ", fs->current_directory);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Supprimer le saut de ligne

        if (strncmp(command, "sudo passwd", 11) == 0) {
            if (verify_sudo_password(fs, current_own)) {
                if (show_password(fs)) {
                    sudo = 0; // Réinitialiser le mode sudo            
                    success = 'o'; // Si le mot de passe est affiché avec succès
                } else {
                    printf("Erreur : Impossible d'afficher le mot de passe.\n");
                    success = 'n'; // Si le mot de passe n'est pas affiché
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo infuser", 12) == 0) {
            if (verify_sudo_password(fs, current_own)) {
                if (show_user_info(fs, command + 13)) {
                    sudo = 0; // Réinitialiser le mode sudo
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    printf("Erreur : Impossible d'afficher les informations de l'utilisateur.\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        }
        else if (strncmp(command, "sudo trace", 10) == 0) {
            if (verify_sudo_password(fs, current_own)) {
                if (read_trace_by_user(fs, current_own)) {
                    sudo = 0; // Réinitialiser le mode sudo
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    printf("Erreur : Impossible de changer le mot de passe.\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo chgpasswd", 14) == 0) {
            if (verify_sudo_password(fs, current_own)) {
                if (change_password(fs)) {
                    sudo = 0; // Réinitialiser le mode sudo
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    printf("Erreur : Impossible de changer le mot de passe.\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo deluser", 12) == 0) {
            if (verify_sudo_password(fs, current_own)) {
                if (delete_user_account(fs, command + 13)) {
                    printf("Compte utilisateur supprimé avec succès.\n");
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    printf("Erreur : Impossible de supprimer le compte utilisateur.\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
                sudo = 0; // Réinitialiser le mode sudo
                break;
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo resetuser", 14) == 0) {
            if (verify_sudo_password(fs, current_own)) {
                if (reset_user_workspace(fs, command + 15)) {
                    printf("Répertoire utilisateur réinitialisé avec succès.\n");
                    sudo = 0; // Réinitialiser le mode sudo
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    printf("Erreur : Impossible de réinitialiser le répertoire utilisateur.\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo delgroup", 13) == 0) {
            if (verify_sudo_password(fs, current_own) && is_user_admin(fs, current_own)) {
                if (delete_group(fs, command + 14)) {
                    printf("Groupe supprimé avec succès.\n");
                    success = 'o'; // Si la création du répertoire réussit
                    sudo = 0; // Réinitialiser le mode sudo
                } else {
                    printf("Erreur : Impossible de supprimer le groupe.\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo add", 8) == 0) {
            if (verify_sudo_password(fs, current_own) && is_user_admin(fs, current_own)) {
                char username[MAX_FILENAME];
                char groupname[MAX_FILENAME];
                if (sscanf(command + 9, "%s %s", username, groupname) == 2) {
                    if (add_user_to_group(fs, username, groupname)) {
                        printf("Utilisateur '%s' ajouté au groupe '%s'.\n", username, groupname);
                        success = 'o'; // Si la création du répertoire réussit
                        sudo = 0;
                    } else {
                        printf("Erreur : Impossible d'ajouter l'utilisateur au groupe.\n");
                        success = 'n'; // Si la création du répertoire échoue
                    }
                } else {
                    printf("Usage: sudo add <username> <groupname>\n");
                    success = 'n'; // Si la création du répertoire échoue    
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo addadmin", 13) == 0) {
            if (verify_sudo_password(fs, current_own) && is_user_superadmin(fs, current_own)) {
                char username[MAX_FILENAME];
                if (sscanf(command + 14, "%s", username) == 1) {
                    if (promote_to_admin(fs, username)) {
                        printf("Utilisateur '%s' promu administrateur.\n", username);
                        success = 'o'; // Si la création du répertoire réussit
                        sudo = 0;
                    } else {
                        printf("Erreur : Impossible de promouvoir l'utilisateur.\n");
                        success = 'n'; // Si la création du répertoire échoue
                    }
                } else {
                    printf("Usage: sudo addadmin <username>\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo deladmin", 13) == 0) {
            if (verify_sudo_password(fs, current_own) && is_user_superadmin(fs, current_own)) {
                char username[MAX_FILENAME];
                if (sscanf(command + 14, "%s", username) == 1) {
                    if (demote_from_admin(fs, username)) {
                        printf("Utilisateur '%s' retiré du rôle d'administrateur.\n", username);
                        success = 'o'; // Si la création du répertoire réussit
                        sudo = 0;
                    } else {
                        printf("Erreur : Impossible de retirer l'utilisateur.\n");
                        success = 'n'; // Si la création du répertoire échoue
                    }
                } else {
                    printf("Usage: sudo deladmin <username>\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "sudo remove", 11) == 0) {
            if (verify_sudo_password(fs, current_own) && is_user_admin(fs, current_own)) {
                char username[MAX_FILENAME];
                char groupname[MAX_FILENAME];
                if (sscanf(command + 12, "%s %s", username, groupname) == 2) {
                    if (remove_user_from_group(fs, username, groupname)) {
                        printf("Utilisateur '%s' retiré du groupe '%s'.\n", username, groupname);
                        success = 'o'; // Si la création du répertoire réussit
                    } else {
                        printf("Erreur : Impossible de retirer l'utilisateur du groupe.\n");
                        success = 'n'; // Si la création du répertoire échoue
                    }
                } else {
                    printf("Usage: sudo remove <username> <groupname>\n");
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                success = 'n'; // Le mot de passe sudo est incorrect
            }
        } else if (strncmp(command, "lssymlinks", 10) == 0) {
            if (strlen(command) > 11) {
                char full_path[MAX_PATH+1];
                snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, command + 11);
                if (list_symbolic_links(fs, full_path)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire réussit
                }
            } else {
                printf("Usage: lssymlinks <fichier_cible>\n");
                success = 'n'; // Si la création du répertoire échoue
            }
        } else if (strncmp(command, "lshardlinks", 11) == 0) {
            if (strlen(command) > 12) {
                char full_path[MAX_PATH+1];
                snprintf(full_path, sizeof(full_path), "%s/%s", fs->current_directory, command + 12);
                if (list_hard_links(fs, full_path)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire réussit
                }
            } else {
                printf("Usage: lshardlinks <fichier_cible>\n");
                success = 'n'; // Si la création du répertoire échoue
            }
        } else if (strncmp(command, "exit", 4) == 0) {
            printf("Arrêt du système de fichiers.\n");
            strcpy(fs->current_directory, "/home");
            success = 'o';  // Succès de l'exécution de la commande exit
            break;
        } else if (strncmp(command, "help", 4) == 0) {
            help();
            success = 'o';  // Succès de l'exécution de la commande exit
        } else if (strncmp(command, "delgroup", 8) == 0 || strncmp(command, "passwd", 6) == 0 || strncmp(command, "chgpasswd", 9) == 0 || strncmp(command, "deluser", 7) == 0  || strncmp(command, "resetuser", 9) == 0) {
            printf("Erreur : Cette commande fonctionne uniquement avec sudo\n");
            success = 'n';  // Échec de l'exécution de la commande
        } else if (strncmp(command, "pwd", 3) == 0) {
            printf("%s\n", fs->current_directory);
            success = 'o';  // Succès de l'exécution de la commande pwd
        } else if (strncmp(command, "mkdir", 5) == 0) {
            char createdirname[MAX_DIRECTORY];
            char finaldirename[MAX_DIRECTORY];

            // Initialiser les variables à des chaînes vides pour éviter les erreurs
            createdirname[0] = '\0';
            finaldirename[0] = '\0';
            int count = sscanf(command + 6, "%s %s", createdirname, finaldirename);
            // Vérifier si le répertoire a été fourni ou non
            if (count < 2 || strlen(finaldirename) == 0) {
                if (create_directory(fs, createdirname, NULL)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                if (create_directory(fs, createdirname, finaldirename)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            }
        } else if (strncmp(command, "rmdir", 5) == 0) {
            if (delete_directory(fs, command + 6)) {
                success = 'o'; // Si la suppression du répertoire réussit
            } else {
                success = 'n'; // Si la suppression du répertoire échoue
            }
        } else if (strncmp(command, "cpdir", 5) == 0) {
            char dirnamedepart[MAX_DIRECTORY];
            char direnamefinal[MAX_DIRECTORY];
            char repertoire[MAX_DIRECTORY];

            // Initialiser les variables à des chaînes vides pour éviter les erreurs
            dirnamedepart[0] = '\0';
            direnamefinal[0] = '\0';
            repertoire[0] = '\0';
        
            int count = sscanf(command + 6, "%s %s %s", dirnamedepart, direnamefinal, repertoire);
        
            // Vérifier si toutes les valeurs ont été correctement lues
            if (count < 2) { 
                printf("Erreur : commande incorrecte. Format attendu : cpdir <source> <destination> [répertoire]\n");
                return;
            }
        
            // Vérifier si le répertoire a été fourni ou non
            if (count < 3 || strlen(repertoire) == 0) {
                if (copy_repertoire(fs, dirnamedepart, direnamefinal, NULL)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                if (copy_repertoire(fs, dirnamedepart, direnamefinal, repertoire)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            }
        } else if (strncmp(command, "mvdir", 5) == 0) {
            char repertoirename[MAX_DIRECTORY];
            char rep_name[MAX_DIRECTORY];
            sscanf(command + 6, "%s %s", repertoirename, rep_name);
            if (move_directory(fs, repertoirename, rep_name)) {
                success = 'o'; // Si le déplacement du répertoire réussit
            } else {
                success = 'n'; // Si le déplacement du répertoire échoue
            }
        }  else if (strncmp(command, "mvs", 3) == 0) {
            char symbolik_name[MAX_DIRECTORY];
            char rep_name[MAX_DIRECTORY];
            sscanf(command + 4, "%s %s", symbolik_name, rep_name);
            if (move_symbolic_link(fs, symbolik_name, rep_name)) {
                success = 'o'; // Si le déplacement du répertoire réussit
            } else {
                success = 'n'; // Si le déplacement du répertoire échoue
            }
        } else if (strncmp(command, "cd", 2) == 0) {
            if (change_directory(fs, command + 3)) {
                success = 'o'; // Si le changement de répertoire réussit
            } else {
                success = 'n'; // Si le changement de répertoire échoue
            }
        } else if (strncmp(command, "lsgroups", 8) == 0) {
            if (list_user_groups(fs)) {
                success = 'o'; // Si la création du répertoire réussit
            } else {
                success = 'n'; // Si la création du répertoire échoue
            }
        } else if (strncmp(command, "lsmembers", 9) == 0) {
            if (strlen(command) > 10) {
                if (list_group_members(fs, command + 10)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                printf("Usage: lsmembers <groupname>\n");
                printf("Alternative: lsmembers (affiche le groupe courant)\n");
                
                // Afficher les membres du groupe courant si aucun groupe spécifié
                if (strlen(current_group) > 0) {
                    printf("\nMembres du groupe courant '%s':\n", current_group);
                    if (list_group_members(fs, current_group)) {
                        success = 'o'; // Si la création du répertoire réussit
                    } else {
                        success = 'n'; // Si la création du répertoire échoue
                    }
                }
            }
        } else if (strncmp(command, "lsl", 3) == 0) {
            if (list_all_directory(fs)) {
                success = 'o'; // Si la création du répertoire réussit
            } else {
                success = 'n'; // Si la création du répertoire échoue
            }
        } else if (strncmp(command, "ls", 2) == 0) {
            if (list_directory(fs)) {
                success = 'o'; // Si la création du répertoire réussit
            } else {
                success = 'n'; // Si la création du répertoire échoue
            }
        }  else if (strncmp(command, "lns", 3) == 0) {
            char file_name[MAX_FILENAME];
            char link_name[MAX_FILENAME];
            char repertoire[MAX_DIRECTORY];

            // Initialiser les variables à des chaînes vides pour éviter les erreurs
            file_name[0] = '\0';
            link_name[0] = '\0';
            repertoire[0] = '\0';
        
            int count = sscanf(command + 3, "%s %s %s", file_name, link_name, repertoire);
        
            // Vérifier si toutes les valeurs ont été correctement lues
            if (count < 2) { 
                printf("Erreur : commande incorrecte. Format attendu : lns <source> <destination> [répertoire]\n");
                return;
            }
        
            // Vérifier si le répertoire a été fourni ou non
            if (count < 3 || strlen(repertoire) == 0) {
                if (create_symbolic_link(fs, file_name, link_name, NULL)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                if (create_symbolic_link(fs, file_name, link_name, repertoire)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            }
        }  else if (strncmp(command, "writes", 6) == 0) {
            char linkname[MAX_FILENAME];
            char content[MAX_CONTENT * 2];
            sscanf(command + 7, "%s %[^\n]", linkname, content);
            if (write_symbolic_link(fs, linkname, content)) {
                success = 'o'; // Si l'écriture dans le fichier réussit
            } else {
                success = 'n'; // Si l'écriture dans le fichier échoue
            }
        } else if (strncmp(command, "touch", 5) == 0) {
            char filename[MAX_FILENAME];
            int size = FILE_SIZE; // Taille par défaut
            sscanf(command + 6, "%s", filename);
            if (create_file(fs, filename, size, current_own)) {
                success = 'o'; // Si la création du fichier réussit
            } else {
                success = 'n'; // Si la création du fichier échoue
            }
        } else if (strncmp(command, "statf", 5) == 0) {
            if (show_file_metadata(fs, command + 6)) {
                success = 'o'; // Si l'affichage des métadonnées réussit
            } else {
                success = 'n'; // Si l'affichage des métadonnées échoue
            }
        } else if (strncmp(command, "statd", 5) == 0) {
            if (show_directory_metadata(fs, command + 6)) {
                success = 'o'; // Si l'affichage des métadonnées réussit
            } else {
                success = 'n'; // Si l'affichage des métadonnées échoue
            }
        } else if (strncmp(command, "chmodf", 6) == 0) {
            char filename[MAX_FILENAME];
            char target[10];
            char new_permissions[4];
            sscanf(command + 7, "%s %s %s", filename, target, new_permissions);
            if (chmod_file(fs, filename, target, new_permissions)) {
                success = 'o'; // Si la modification des permissions réussit
            } else {
                success = 'n'; // Si la modification des permissions échoue
            }
        } else if (strncmp(command, "chmodd", 6) == 0) {
            char dirname[MAX_FILENAME];
            char target[10];
            char new_permissions[4];
            sscanf(command + 7, "%s %s %s", dirname, target, new_permissions);
            if (chmod_dir(fs, dirname, target, new_permissions)) {
                success = 'o'; // Si la modification des permissions réussit
            } else {
                success = 'n'; // Si la modification des permissions échoue
            }
        } else if (strncmp(command, "write", 5) == 0) {
            char filename[MAX_FILENAME];
            char content[MAX_CONTENT * 2];
            sscanf(command + 6, "%s %[^\n]", filename, content);
            if (write_to_file(fs, filename, content)) {
                success = 'o'; // Si l'écriture dans le fichier réussit
            } else {
                success = 'n'; // Si l'écriture dans le fichier échoue
            }
        } else if (strncmp(command, "cat", 3) == 0) {
            if (read_file(fs, command + 4)) {
                success = 'o'; // Si la lecture du fichier réussit
            } else {
                success = 'n'; // Si la lecture du fichier échoue
            }
        }  else if (strncmp(command, "reads", 5) == 0) {
            if (read_symbolic_link(fs, command + 6)) {
                success = 'o'; // Si la lecture du fichier réussit
            } else {
                success = 'n'; // Si la lecture du fichier échoue
            }
        }  else if (strncmp(command, "rms", 3) == 0) {
            if (delete_symbolic_link(fs, command + 4)) {
                success = 'o'; // Si la suppression du fichier réussit
            } else {
                success = 'n'; // Si la suppression du fichier échoue
            }
        } else if (strncmp(command, "rm", 2) == 0) {
            if (delete_file(fs, command + 3)) {
                success = 'o'; // Si la suppression du fichier réussit
            } else {
                success = 'n'; // Si la suppression du fichier échoue
            }
        } else if (strncmp(command, "cp", 2) == 0) {
            char file_name[MAX_FILENAME];
            char link_name[MAX_FILENAME];
            char repertoire[MAX_DIRECTORY];

            // Initialiser les variables à des chaînes vides pour éviter les erreurs
            file_name[0] = '\0';
            link_name[0] = '\0';
            repertoire[0] = '\0';
        
            int count = sscanf(command + 3, "%s %s %s", file_name, link_name, repertoire);
        
            // Vérifier si toutes les valeurs ont été correctement lues
            if (count < 2) { 
                printf("Erreur : commande incorrecte. Format attendu : cp <source> <destination> [répertoire]\n");
                return;
            }
        
            // Vérifier si le répertoire a été fourni ou non
            if (count < 3 || strlen(repertoire) == 0) {
                if (copy_file(fs, file_name, link_name, NULL)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            } else {
                if (copy_file(fs, file_name, link_name, repertoire)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            }
        } else if (strncmp(command, "mv", 2) == 0) {
            char filename[MAX_FILENAME];
            char rep_name[MAX_DIRECTORY];
            sscanf(command + 3, "%s %s", filename, rep_name);
            if (move_file(fs, filename, rep_name)) {
                success = 'o'; // Si le déplacement du fichier réussit
            } else {
                success = 'n'; // Si le déplacement du fichier échoue
            }
        } else if (strncmp(command, "free", 4) == 0) {
            print_free_blocks();
            success = 'o'; // Si l'affichage des blocs libres réussit
        } else if (strncmp(command, "leavegroup", 10) == 0) {
            if (strlen(command) > 11) {
                if (leave_group(fs, command + 11)) {
                    success = 'o'; // Si la création du répertoire réussit
                } else {
                    success = 'n'; // Si la création du répertoire échoue
                }
            } 
            
        } else if (strncmp(command, "clear", 5) == 0) {
            clear_screen(); 
            success = 'o'; // Si l'effacement de l'écran réussit
        } else if (strncmp(command, "whoami", 6) == 0) {
            printf("Utilisateur actuel : %s\n", current_own);
            success = 'o'; // Si l'affichage de l'utilisateur actuel réussit 
        } else if (strncmp(command, "mvf", 3) == 0) {
            char file_name[MAX_FILENAME];
            char link_name[MAX_FILENAME];
            sscanf(command + 4, "%s %s", file_name, link_name);
            if (rename_file(fs,file_name,link_name)) {
                success = 'o'; // Si le déplacement du fichier réussit
            } else {
                success = 'n'; // Si le déplacement du fichier échoue
            }
        } else if (strncmp(command, "mvd", 3) == 0) {
            char repnamedepart[MAX_DIRECTORY];
            char repnamefinal[MAX_DIRECTORY];
            sscanf(command + 4, "%s %s", repnamedepart, repnamefinal);
            if (rename_directory(fs,repnamedepart,repnamefinal)) {
                success = 'o'; // Si le déplacement du répertoire réussit
            } else {
                success = 'n'; // Si le déplacement du répertoire échoue
            }
        } else if (strncmp(command, "chgroup", 7) == 0) {
            if (change_group(fs, command + 8)) {
                success = 'o'; // Si le changement de groupe réussit
            } else {
                success = 'n'; // Si le changement de groupe échoue
            }
        } else if (strncmp(command, "curgroup", 12) == 0) {
            if (show_current_group()) {
                success = 'o'; // Si l'affichage du groupe courant réussit
            } else {
                success = 'n'; // Si l'affichage du groupe courant échoue
            }
        } else if (strncmp(command, "crtgroup", 8) == 0) {          
            if (create_group_directory(fs, command + 9)) {
                success = 'o'; // Si la création du répertoire réussit
            } else {
                success = 'n'; // Si la création du répertoire échoue
            }
        } else if (strncmp(command, "lnm", 3) == 0) {
            char source_file[MAX_FILENAME];
            char link_name[MAX_FILENAME];
            sscanf(command + 4, "%s %s", source_file, link_name);
            if (create_hard_link(fs, source_file, link_name)) {
                success = 'o'; // Si la création du lien matériel réussit
            } else {
                success = 'n'; // Si la création du lien matériel échoue
            }
        }  else {
            printf("Commande inconnue !\n");
            success = 'n'; // Échec de l'exécution de la commande
        }
        save_trace_execution(fs, current_own, current_group, command, success);
    }
}

// Fonction pour initialiser le système de fichiers
void init_main(Filesystem *fs) {
    printf("\nEntrez votre nom: ");

    // Vérifier si l'utilisateur existe déjà dans la table des groupes
    int user_exists = 0;
    int good = 0; // Pour vérifier si l'utilisateur a été trouvé ou créé
    int user_count = 0; // Compteur d'utilisateurs
    char command[10]; // Pour stocker la commande à exécuter
    char success; // Pour déterminer le succès de l'exécution de la commande

    // Utiliser fgets pour lire l'entrée
    if (fgets(current_own, NAME_SIZE, stdin) != NULL) {
        // Supprimer le saut de ligne (\n) à la fin de la chaîne
        current_own[strcspn(current_own, "\n")] = '\0';

        // Vérifier si la chaîne est vide
        if (strlen(current_own) == 0) {
            printf("Erreur : le nom d'utilisateur ne peut pas être vide.\n");
            success = 'n'; // Échec de l'exécution de la commande
            exit(1);
        }
    } else {
        printf("Erreur lors de la lecture du nom.\n");
        success = 'n'; // Échec de l'exécution de la commande
        exit(1);
    }

    for (int i = 0; i < NUM_USER; i++) {
        if (strcmp(fs->group[i].user, current_own) == 0) {
            user_exists = 1; // L'utilisateur existe
            break;
        }
    }

    // Si l'utilisateur n'existe pas, créer une nouvelle entrée
    if (!user_exists) {
        int found_free_slot = 0;
        for (int i = 0; i < NUM_USER; i++) {
            if (fs->group[i].user[0] == '\0') { // Si l'emplacement est libre

                // Demander un mot de passe simple (visible à l'écran)
                char password[MAX_PASSWORD];
                printf("Entrez votre mot de passe pour votre compte : ");
                fgets(password, MAX_PASSWORD, stdin);
                password[strcspn(password, "\n")] = '\0'; // Supprimer le saut de ligne
                
                // Vérification minimale
                if (strlen(password) == 0) {
                    printf("Erreur : le mot de passe ne peut pas être vide.\n");
                    exit(1);
                }

                strncpy(fs->group[i].user, current_own, NAME_SIZE);
                fs->group[i].user[NAME_SIZE - 1] = '\0'; // Garantir la terminaison de la chaîne
                strncpy(fs->group[i].password, password, MAX_PASSWORD);
                fs->group[i].password[MAX_PASSWORD - 1] = '\0';
                fs->group[i].taille = 0; // Initialiser le nombre de groupes à 0
                if (fs->user_count == 0) {
                    fs->group[i].is_admin = 1; // Initialiser le statut admin à 0
                    fs->group[i].is_root = 1; // Initialiser le statut super utilisateur à 1
                                    
                    char root_pwd[MAX_PASSWORD];
                    printf("Entrez votre mot de passe de super admin : ");
                    fgets(root_pwd, MAX_PASSWORD, stdin);
                    root_pwd[strcspn(root_pwd, "\n")] = '\0'; // Supprimer le saut de ligne
                    
                    // Vérification minimale
                    if (strlen(root_pwd) == 0) {
                        printf("Erreur : le mot de passe ne peut pas être vide.\n");
                        exit(1);
                    }
                    strncpy(fs->group[i].root_pwd, root_pwd, MAX_PASSWORD);
                    fs->group[i].root_pwd[MAX_PASSWORD - 1] = '\0';
                } else {
                    fs->group[i].is_admin = 0; // Initialiser le statut admin à 0
                    fs->group[i].is_root = 0; // Initialiser le statut super utilisateur à 0
                }
                fs->user_count++; // Incrémenter le nombre de groupes
                user_add_group(fs, current_own); // Ajouter l'utilisateur au groupe par défaut
                found_free_slot = 1;
                strncpy(fs->current_directory, "./users/home", MAX_FILENAME);
                create_directory(fs, current_own, NULL); // Crée ./users/home/<username>
                printf("Nouvel utilisateur '%s' créé.\n", current_own); 
                good = 1;   
                user_count = 1; // Réinitialiser le compteur d'utilisateurs    
                save_filesystem(fs); // Sauvegarder le système de fichiers
                success = 'o'; // Si la création du répertoire réussit
                break;
            }
        }
        if (!found_free_slot) {
            printf("Erreur : impossible de créer un nouvel utilisateur, nombre maximal d'utilisateurs atteint.\n");
            exit(1); // Quitter si aucun emplacement libre n'est trouvé
        }
    } else {
        // Demander un mot de passe simple (visible à l'écran)
        char password[MAX_PASSWORD];
        printf("Entrez votre mot de passe pour votre compte : ");
        fgets(password, MAX_PASSWORD, stdin);
        password[strcspn(password, "\n")] = '\0'; // Supprimer le saut de ligne
        
        // Vérification minimale
        if (strlen(password) == 0) {
            printf("Erreur : le mot de passe ne peut pas être vide.\n");
            exit(1);
        }
        for (int i = 0; i < NUM_USER; i++) {
            if (strcmp(fs->group[i].user, current_own) == 0) {
                if (strcmp(fs->group[i].password, password) != 0) {
                    printf("Erreur : mot de passe incorrect.\n");
                    exit(1); // Quitter si le mot de passe est incorrect
                }
                else {
        
                    printf("Utilisateur '%s' trouvé.\n", current_own);
                    good = 1;   
                    user_count = 2; // Réinitialiser le compteur d'utilisateurs   
                    success = 'o'; // Si la création du répertoire réussit  
                    break; // Sortir de la boucle une fois l'utilisateur trouvé
                
                }
            }
        }
    }

    if (good) {
        // Réinitialiser le chemin courant avant de créer le répertoire
        strncpy(fs->current_directory, "./users/home", MAX_FILENAME);
        // Mettre à jour le chemin courant
        snprintf(fs->current_directory, MAX_FILENAME*2, "./users/home/%s", current_own);
        strncpy(current_group, current_own, sizeof(current_group));  
        if (user_count == 1) {
            strncpy(command, "new user", sizeof(command)); 
        } 
        if (user_count == 2){
            strncpy(command, "old user", sizeof(command)); 
        }
        save_trace_execution(fs, current_own,current_group, command,success); // Enregistrer l'exécution de la commande
    }
}

// Fonction principale
int main() {
    setlocale(LC_ALL, "en_US.UTF-8"); // Pour gérer les caractères spéciaux
    Filesystem fs;
    init_filesystem(&fs); // Initialiser le système de fichiers  
    init_main(&fs); // Initialiser le système
    shell(&fs, current_own); // Lancer le shell
    return 0;
}
