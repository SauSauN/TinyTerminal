#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

// Définir les tailles maximales pour les noms, les chemins et les entrées utilisateur
#define MAX_NOM 50
#define MAX_INPUT 100
#define MAX_CHEMIN 1000  
#define MORE_CHEMIN 100
#define FS_SIZE 10 * 1024 * 1024  // 10 Mo
#define PARTITION "projet.bin"

// Structure pour représenter un utilisateur
typedef struct {
    int id;                  // Identifiant unique de l'utilisateur
    char nom[MAX_NOM];       // Nom de l'utilisateur
} Utilisateur;

// Variables globales
int idUtilisateurConnecte = -1;  // ID de l'utilisateur connecté (-1 si aucun utilisateur n'est connecté)
char ident[MAX_NOM] = "";        // Nom de l'utilisateur connecté
char cheminActuel[MAX_CHEMIN] = "";  // Chemin actuel dans le système de fichiers virtuel
char cheminPrincipal[MAX_CHEMIN] = "";  // Chemin actuel dans le système de fichiers virtuel
const char *identite = NULL;

// Fonction pour vérifier si un fichier existe
int fichierExiste(const char *nomFichier) {
    FILE *fichier = fopen(nomFichier, "rb");  // Ouvrir le fichier en mode lecture binaire
    if (fichier) {
        fclose(fichier);  // Fermer le fichier s'il existe
        return 1;         // Retourner 1 si le fichier existe
    }
    return 0;  // Retourner 0 si le fichier n'existe pas
}

// Fonction pour vérifier si un fichier existe dans le fichier bin
int fileExistance(const char *nomFichier, const char *nomFile, char *nomdossier){
    FILE *fichier = fopen(nomFichier, "rb");  
    if (!fichier) { 
        return 0; // Le fichier principal n'existe même pas encore
    }
    char cheminDestination[MAX_CHEMIN];

    // Vérification si nomdossier est NULL
    if (nomdossier == NULL) {
        snprintf(cheminDestination, sizeof(cheminDestination), "%s%s", cheminActuel, nomFile);
    } else {
        snprintf(cheminDestination, sizeof(cheminDestination), "%s%s", nomdossier, nomFile);
    }
    
    char ligne[MAX_CHEMIN];
    int exist = 0;  // Variable pour indiquer si la chaîne est trouvée

    // Lecture du fichier ligne par ligne
    while (fgets(ligne, sizeof(ligne), fichier)) {
        // Suppression du caractère de nouvelle ligne '\n'
        ligne[strcspn(ligne, "\n")] = '\0'; 

        // Recherche de la balise dans la ligne
        if (strstr(ligne, cheminDestination) != NULL) {
            exist = 1; // Indique que la balise a été trouvée
            break;     // Arrêt de la lecture du fichier
        }
    }

    fclose(fichier); // Fermeture du fichier après la lecture

    return exist;  // Retourne 1 si la chaîne est trouvée, sinon 0
}

// Fonction pour vérifier si un fichier existe dans le fichier bin
int repertoireExistance(const char *nomFichier, char *nomdossier){
    FILE *fichier = fopen(nomFichier, "rb");  
    if (!fichier) { 
        return 0; // Le fichier principal n'existe même pas encore
    }
    
    char ligne[MAX_CHEMIN];
    int exist = 0;  // Variable pour indiquer si la chaîne est trouvée

    // Lecture du fichier ligne par ligne
    while (fgets(ligne, sizeof(ligne), fichier)) {
        // Suppression du caractère de nouvelle ligne '\n'
        ligne[strcspn(ligne, "\n")] = '\0'; 

        // Recherche de la balise dans la ligne
        if (strstr(ligne, nomdossier) != NULL) {
            exist = 1; // Indique que la balise a été trouvée
            break;     // Arrêt de la lecture du fichier
        }
    }

    fclose(fichier); // Fermeture du fichier après la lecture

    return exist;  // Retourne 1 si la chaîne est trouvée, sinon 0
}

// Fonction pour initialiser le disque virtuel
void initialiserDisqueVirtuel(const char *nomFichier) {
    FILE *fichier = fopen(nomFichier, "wb");  // Ouvrir le fichier en mode écriture binaire
    if (!fichier) {
        perror("Erreur de création du disque virtuel");  // Afficher une erreur si le fichier ne peut pas être créé
        exit(1);  // Quitter le programme en cas d'erreur
    }
    // Écrire la structure de base du disque virtuel
    fprintf(fichier, "Répertoire: File/\n");
    fprintf(fichier, "Répertoire: User/\n");
    fclose(fichier);  // Fermer le fichier
    printf("📂 Disque virtuel '%s' initialisé.\n", nomFichier);  // Confirmer l'initialisation
}

// Fonction pour générer un ID unique pour un nouvel utilisateur
int genererID(const char *nomFichier) {
    FILE *fichier = fopen(nomFichier, "rb");  // Ouvrir le fichier en mode lecture binaire
    if (!fichier) return 10;  // Retourner 10 si le fichier ne peut pas être ouvert

    int id, maxID = 9;  // Initialiser maxID à 9
    char ligne[MAX_CHEMIN];

    // Lire le fichier ligne par ligne pour trouver le plus grand ID existant
    while (fgets(ligne, sizeof(ligne), fichier)) {
        if (sscanf(ligne, "File/Uslist.txt: %d", &id) == 1) {
            if (id > maxID) maxID = id;  // Mettre à jour maxID si un ID plus grand est trouvé
        }
    }
    fclose(fichier);  // Fermer le fichier
    return maxID + 1;  // Retourner le prochain ID disponible
}

// Fonction pour vérifier si un utilisateur existe déjà
int utilisateurExiste(const char *nomFichier, const char *nomUtilisateur) {
    FILE *fichier = fopen(nomFichier, "rb");  // Ouvrir le fichier en mode lecture binaire
    if (!fichier) return 0;  // Retourner 0 si le fichier ne peut pas être ouvert

    char ligne[MAX_CHEMIN], nom[MAX_NOM];
    int id;
    
    // Lire le fichier ligne par ligne pour vérifier si l'utilisateur existe
    while (fgets(ligne, sizeof(ligne), fichier)) {
        if (sscanf(ligne, "File/Uslist.txt: %d %s", &id, nom) == 2) {
            if (strcmp(nom, nomUtilisateur) == 0) {  // Comparer le nom de l'utilisateur
                fclose(fichier);  // Fermer le fichier
                return 1;  // Retourner 1 si l'utilisateur existe
            }
        }
    }
    fclose(fichier);  // Fermer le fichier
    return 0;  // Retourner 0 si l'utilisateur n'existe pas
}

// Fonction pour ajouter un nouvel utilisateur
void ajouterUtilisateur(const char *nomFichier, const char *nomUtilisateur) {
    if (utilisateurExiste(nomFichier, nomUtilisateur)) {  // Vérifier si l'utilisateur existe déjà
        printf("⚠️ Utilisateur '%s' existe déjà.\n", nomUtilisateur);
        return;
    }

    int id = genererID(nomFichier);  // Générer un nouvel ID pour l'utilisateur
    FILE *fichier = fopen(nomFichier, "ab");  // Ouvrir le fichier en mode ajout binaire
    if (!fichier) {
        perror("Erreur d'écriture");  // Afficher une erreur si le fichier ne peut pas être ouvert
        return;
    }

    // Ajouter l'utilisateur au fichier
    fprintf(fichier, "File/Uslist.txt: %d %s\n", id, nomUtilisateur);
    fprintf(fichier, "User/id%d_%s/\n", id, nomUtilisateur);
    fclose(fichier);  // Fermer le fichier
    printf("✅ Utilisateur '%s' créé avec ID %d.\n", nomUtilisateur, id);  // Confirmer la création
}

// Fonction pour afficher le contenu du disque virtuel
void afficherDisqueVirtuel(const char *nomFichier) {
    FILE *fichier = fopen(nomFichier, "rb");  // Ouvrir le fichier en mode lecture binaire
    if (!fichier) {
        perror("Erreur de lecture");  // Afficher une erreur si le fichier ne peut pas être ouvert
        return;
    }

    char ligne[MAX_CHEMIN];
    printf("\n📂 Contenu du disque virtuel :\n");
    // Lire et afficher chaque ligne du fichier
    while (fgets(ligne, sizeof(ligne), fichier)) {
        printf("%s", ligne);
    }
    fclose(fichier);  // Fermer le fichier
}

// Fonction pour afficher la liste des utilisateurs
void afficherUslist(const char *nomFichier) {
    FILE *fichier = fopen(nomFichier, "rb");  // Ouvrir le fichier en mode lecture binaire
    if (!fichier) return;

    char ligne[MAX_CHEMIN];
    printf("\n📄 Liste des utilisateurs :\n");

    // Lire le fichier ligne par ligne et afficher les utilisateurs
    while (fgets(ligne, sizeof(ligne), fichier)) {
        if (strncmp(ligne, "File/Uslist.txt:", 16) == 0) {
            printf("%s", ligne + 17);  // Afficher la ligne sans le préfixe
        }
    }
    fclose(fichier);  // Fermer le fichier
}

// Fonction pour connecter un utilisateur
int connectionCompte(const char *nomFichier, const char *identifiant) {
    FILE *fichier = fopen(nomFichier, "rb");
    if (!fichier) return 0;

    char ligne[MAX_CHEMIN], nom[MAX_NOM];
    int id, trouve = 0;
    
    while (fgets(ligne, sizeof(ligne), fichier)) {
        if (sscanf(ligne, "File/Uslist.txt: %d %s", &id, nom) == 2) {
            if (strcmp(nom, identifiant) == 0) {
                trouve = 1;
                idUtilisateurConnecte = id;
                strcpy(ident, identifiant);
                // Initialiser le chemin actuel
                snprintf(cheminActuel, sizeof(cheminActuel), "User/id%d_%s/", id, identifiant);
                // Copie correcte du cheminActuel vers cheminPrincipal
                strncpy(cheminPrincipal, cheminActuel, sizeof(cheminPrincipal));
                cheminPrincipal[sizeof(cheminPrincipal) - 1] = '\0';  // Sécuriser la fin de la chaîne
                break;
            }
        }
    }
    fclose(fichier);

    if (trouve) {
        printf("✅ Connexion réussie : %s\n", identifiant);
        identite = identifiant;
        return 1;
    } else {
        printf("⚠️ Utilisateur non trouvé.\n");
        return 0;
    }
}

// Fonction pour afficher le chemin actuel
void afficherCheminActuel() {
    if (idUtilisateurConnecte != -1)
        printf("📂 Vous êtes dans ce dossier : %s\n",cheminActuel);  // Afficher le chemin actuel
    else
        printf("⚠️ Aucun utilisateur connecté.\n");  // Afficher un message d'erreur
}

// Fonction pour créer un répertoire utilisateur
void creerRepertoireUtilisateur(const char *nomFichier, const char *nomRepertoire) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");  // Afficher un message d'erreur
        return;
    }

    char chemin[MAX_CHEMIN+MORE_CHEMIN];
    // Construire le chemin du nouveau répertoire
    //snprintf(chemin, sizeof(chemin), "User/id%d_%s/%s/\n", idUtilisateurConnecte, ident, nomRepertoire);
    snprintf(chemin, sizeof(chemin), "%s%s/\n", cheminActuel, nomRepertoire);

    FILE *fichier = fopen(nomFichier, "ab");  // Ouvrir le fichier en mode ajout binaire
    if (!fichier) {
        perror("Erreur d'écriture");  // Afficher une erreur si le fichier ne peut pas être ouvert
        return;
    }
    fprintf(fichier, "%s", chemin);  // Écrire le chemin dans le fichier
    fclose(fichier);  // Fermer le fichier
    printf("📁 Répertoire '%s' créé avec succès !\n", nomRepertoire);  // Confirmer la création
}

// Fonction pour enlever le caractère après le dernier slash dans une chaîne
void remove_char_after_last_slash(char *str) {
    // Trouver la position du dernier slash
    char *last_slash = strrchr(str, '/');
    if (last_slash != NULL && *(last_slash + 1) != '\0') {
        // Remplacer le caractère après le dernier slash par '\0'
        *(last_slash + 1) = '\0';
    }
}

// Fonction pour lister les répertoires et fichiers d'un utilisateur
void listerRepertoiresFichiers(const char *nomFichier) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");  // Afficher un message d'erreur
        return;
    }

    FILE *fichier = fopen(nomFichier, "rb");  // Ouvrir le fichier en mode lecture binaire
    if (!fichier) {
        perror("Erreur de lecture");  // Afficher une erreur si le fichier ne peut pas être ouvert
        return;
    }

    char ligne[MAX_CHEMIN];
    int trouve = 0;

    printf("\n📂 Contenu du répertoire '%s' :\n", cheminActuel);

    // Parcourir le fichier ligne par ligne
    while (fgets(ligne, sizeof(ligne), fichier)) {
        // Supprimer le saut de ligne
        ligne[strcspn(ligne, "\n")] = '\0';

        // Vérifier si la ligne commence par le chemin actuel
        if (strncmp(ligne, cheminActuel, strlen(cheminActuel)) == 0) {
            // Extraire le nom du répertoire ou fichier
            char *nom = ligne + strlen(cheminActuel);

            // Ignorer les lignes vides ou les chemins incorrects
            if (strlen(nom) > 0) {
                trouve = 1;

                // Vérifier si c'est un répertoire (se termine par '/')
                if (nom[strlen(nom) - 1] == '/') {
                    // Vérifier qu'il n'y a pas d'autres slashes après le premier
                    char *slash = strchr(nom, '/');
                    if (slash != NULL && slash == nom + strlen(nom) - 1) {
                        // Afficher uniquement les répertoires de premier niveau
                        printf("📁 %s\n", nom);
                        trouve = 1;
                    }
                } else {
                    if (strchr(nom, '/') == NULL && strstr(nom, ".meta") == NULL) {
                        printf("📄 %s\n", nom);  // Afficher le fichier
                    } 
                }
            }
        }
    }

    fclose(fichier);  // Fermer le fichier

    if (!trouve) {
        printf("📁 Aucun fichier ou dossier trouvé.\n");  // Afficher un message si aucun contenu n'est trouvé
    }
}

// Fonction pour chnager de répertoire 
void changerRepertoire(const char *nomDossier) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");
        return;
    }

    char nouveauChemin[MAX_CHEMIN];

    // Cas 1 : Remonter d'un niveau (cd ..)
    if (strcmp(nomDossier, "..") == 0) {
        char *dernierSlash = strrchr(cheminActuel, '/');
        if (dernierSlash != NULL && dernierSlash != cheminActuel) {
            *dernierSlash = '\0'; // Tronquer le chemin au dernier '/'
            remove_char_after_last_slash(cheminActuel);
            printf("Vous êtes maintenant dans : %s\n", cheminActuel); // Affiche le nouveau chemin
        } else {
            printf("⚠️ Impossible de remonter plus haut.\n");
        }
    }
    // Cas 2 : Descendre dans un répertoire (cd <nom_du_répertoire>)
    else {
        // Vérifier si le chemin dépassera la taille du tampon
        size_t longueurChemin = strlen(cheminActuel) + strlen(nomDossier) + 2; // +2 pour '/' et '\0'
        if (longueurChemin >= MAX_CHEMIN) {
            printf("⚠️ Chemin trop long.\n");
            return;
        }

        // Construire le nouveau chemin
        int n = snprintf(nouveauChemin, sizeof(nouveauChemin), "%s%s/", cheminActuel, nomDossier);

        // Vérifier si snprintf a tronqué la chaîne
        if (n < 0 || n >= sizeof(nouveauChemin)) {
            printf("⚠️ Erreur de construction du chemin.\n");
            return;
        }

        // Vérifier si le répertoire existe dans le disque virtuel
        FILE *fichier = fopen("projet.bin", "rb");
        if (!fichier) {
            perror("Erreur de lecture");
            return;
        }

        char ligne[MAX_CHEMIN];
        int trouve = 0;
        while (fgets(ligne, sizeof(ligne), fichier)) {
            ligne[strcspn(ligne, "\n")] = '\0'; // Supprimer le saut de ligne
            if (strcmp(ligne, nouveauChemin) == 0) {
                trouve = 1;
                break;
            }
        }
        fclose(fichier);

        if (trouve) {
            // Mettre à jour le chemin actuel
            strcpy(cheminActuel, nouveauChemin);
        } else {
            printf("⚠️ Répertoire non trouvé.\n");
        }
    }

    // Afficher le chemin actuel
    printf("📂 Répertoire actuel: %s\n", cheminActuel);
}

// Fonction pour supprimer un répertoire
void supprimerRepertoire(const char *nomFichier, const char *nomRepertoire) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");
        return;
    }

    // Construire le chemin du répertoire à supprimer
    char chemin[MAX_CHEMIN + MORE_CHEMIN];
    snprintf(chemin, sizeof(chemin), "%s%s/", cheminActuel, nomRepertoire);

    // Ouvrir le fichier en mode lecture
    FILE *fichier = fopen(nomFichier, "rb");
    if (!fichier) {
        perror("Erreur de lecture");
        return;
    }

    // Lire tout le contenu du fichier dans un buffer
    fseek(fichier, 0, SEEK_END);
    long tailleFichier = ftell(fichier);
    fseek(fichier, 0, SEEK_SET);

    char *contenu = (char *)malloc(tailleFichier + 1);
    if (!contenu) {
        perror("Erreur d'allocation mémoire");
        fclose(fichier);
        return;
    }

    fread(contenu, 1, tailleFichier, fichier);
    contenu[tailleFichier] = '\0';  // Ajouter un terminateur de chaîne
    fclose(fichier);

    // Parcourir le contenu pour supprimer les lignes correspondantes
    char *ligne = strtok(contenu, "\n");
    char nouveauContenu[MAX_CHEMIN * 1000] = "";  // Buffer pour stocker le nouveau contenu
    int trouve = 0;

    while (ligne != NULL) {
        // Vérifier si la ligne commence par le chemin du répertoire à supprimer
        if (strncmp(ligne, chemin, strlen(chemin)) != 0) {
            // Si la ligne ne correspond pas, l'ajouter au nouveau contenu
            strcat(nouveauContenu, ligne);
            strcat(nouveauContenu, "\n");
        } else {
            // Si la ligne correspond, la marquer comme trouvée
            trouve = 1;
        }
        ligne = strtok(NULL, "\n");
    }

    // Réécrire le fichier avec le nouveau contenu
    fichier = fopen(nomFichier, "wb");
    if (!fichier) {
        perror("Erreur d'écriture");
        free(contenu);
        return;
    }

    fprintf(fichier, "%s", nouveauContenu);
    fclose(fichier);

    // Libérer la mémoire
    free(contenu);

    if (trouve) {
        printf("✅ Répertoire '%s' supprimé avec succès !\n", nomRepertoire);
    } else {
        printf("⚠️ Répertoire '%s' non trouvé.\n", nomRepertoire);
    }
}

// Fonction pour supprimer un fichier
void supprimerFichier(const char *nomFichier, const char *nomfichier) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");
        return;
    }

    // Construire le chemin du fichier à supprimer
    char chemin[MAX_CHEMIN + MORE_CHEMIN];
    snprintf(chemin, sizeof(chemin), "%s%s", cheminActuel, nomfichier);

    // Ouvrir le fichier en mode lecture
    FILE *fichier = fopen(nomFichier, "rb");
    if (!fichier) {
        perror("Erreur de lecture");
        return;
    }

    // Lire tout le contenu du fichier dans un buffer
    fseek(fichier, 0, SEEK_END);
    long tailleFichier = ftell(fichier);
    fseek(fichier, 0, SEEK_SET);

    char *contenu = (char *)malloc(tailleFichier + 1);
    if (!contenu) {
        perror("Erreur d'allocation mémoire");
        fclose(fichier);
        return;
    }

    fread(contenu, 1, tailleFichier, fichier);
    contenu[tailleFichier] = '\0';  // Ajouter un terminateur de chaîne
    fclose(fichier);

    // Parcourir le contenu pour supprimer la ligne correspondante et les 7 lignes suivantes
    char *ligne = strtok(contenu, "\n");
    char nouveauContenu[MAX_CHEMIN * 1000] = "";  // Buffer pour stocker le nouveau contenu
    int trouve = 0;
    int lignesASupprimer = 0;  // Compteur pour les lignes à supprimer

    while (ligne != NULL) {
        // Vérifier si la ligne correspond au fichier à supprimer
        if (strcmp(ligne, chemin) == 0) {
            trouve = 1;
            lignesASupprimer = 8;  // Supprimer cette ligne et les 7 suivantes
        }

        // Si nous ne sommes pas en train de supprimer des lignes, ajouter la ligne au nouveau contenu
        if (lignesASupprimer == 0) {
            strcat(nouveauContenu, ligne);
            strcat(nouveauContenu, "\n");
        } else {
            // Décrémenter le compteur de lignes à supprimer
            lignesASupprimer--;
        }

        ligne = strtok(NULL, "\n");
    }

    // Réécrire le fichier avec le nouveau contenu
    fichier = fopen(nomFichier, "wb");
    if (!fichier) {
        perror("Erreur d'écriture");
        free(contenu);
        return;
    }

    fprintf(fichier, "%s", nouveauContenu);
    fclose(fichier);

    // Libérer la mémoire
    free(contenu);

    if (trouve) {
        printf("✅ Fichier '%s' supprimé avec succès !\n", nomfichier);
    } else {
        printf("⚠️ Fichier '%s' non trouvé.\n", nomfichier);
    }
}

// Fonction pour vérifier si une chaîne commence par un préfixe donné
int commencePar(const char *chaine, const char *prefixe) {
    return strncmp(chaine, prefixe, strlen(prefixe)) == 0;
}

// Fonction pour déplacer un fichier
void deplacerFichier(const char *nomFichier, const char *nomfile, const char *nomdossier) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");
        return;
    }

    // Construire le chemin du fichier à déplacer
    char cheminfile[MAX_CHEMIN + MORE_CHEMIN];
    snprintf(cheminfile, sizeof(cheminfile), "%s%s", cheminActuel, nomfile);

    // Construire le chemin du répertoire de destination
    char cheminrep[MAX_CHEMIN + MORE_CHEMIN];

    // Vérifier si le chemin de destination est absolu (commence par "User/")
    if (commencePar(nomdossier, "User/")) {
        // Utiliser le chemin absolu fourni
        snprintf(cheminrep, sizeof(cheminrep), "%s/", nomdossier);
    } else {
        // Utiliser le chemin relatif au répertoire actuel
        snprintf(cheminrep, sizeof(cheminrep), "%s%s/", cheminActuel, nomdossier);
    }

    // Construire le nouveau chemin du fichier après déplacement
    char chemindeplacement[MAX_CHEMIN + MORE_CHEMIN];
    snprintf(chemindeplacement, sizeof(chemindeplacement), "%s%s", cheminrep, nomfile);

    // Ouvrir le fichier en mode lecture
    FILE *fichier = fopen(nomFichier, "rb");
    if (!fichier) {
        perror("Erreur de lecture");
        return;
    }

    // Variables pour vérifier l'existence du fichier et du répertoire
    int existrep = 0, existfile = 0;

    // Lire le fichier pour vérifier l'existence du fichier et du répertoire
    char ligne[MAX_CHEMIN];
    while (fgets(ligne, sizeof(ligne), fichier)) {
        // Supprimer le saut de ligne
        ligne[strcspn(ligne, "\n")] = '\0';

        // Vérifier si la ligne correspond au fichier à déplacer
        if (strcmp(ligne, cheminfile) == 0) {
            existfile = 1;
        }

        // Vérifier si la ligne correspond au répertoire de destination
        if (strcmp(ligne, cheminrep) == 0) {
            existrep = 1;
        }
    }
    fclose(fichier);

    // Vérifier si le fichier et le répertoire existent
    if (existfile == 0) {
        printf("⚠️ Fichier '%s' non trouvé.\n", nomfile);
        return;
    }
    if (existrep == 0) {
        printf("⚠️ Répertoire '%s' non trouvé.\n", nomdossier);
        return;
    }

    // Ouvrir le fichier en mode lecture et écriture
    fichier = fopen(nomFichier, "rb");
    if (!fichier) {
        perror("Erreur de lecture");
        return;
    }

    // Créer un fichier temporaire pour écrire les modifications
    FILE *tempFile = fopen("temp.bin", "wb");
    if (!tempFile) {
        perror("Erreur de création du fichier temporaire");
        fclose(fichier);
        return;
    }

    // Parcourir le fichier ligne par ligne
    while (fgets(ligne, sizeof(ligne), fichier)) {
        // Supprimer le saut de ligne
        ligne[strcspn(ligne, "\n")] = '\0';

        // Si la ligne correspond au fichier à déplacer, la remplacer par le nouveau chemin
        if (strcmp(ligne, cheminfile) == 0) {
            fprintf(tempFile, "%s\n", chemindeplacement);
        } else {
            // Sinon, copier la ligne telle quelle
            fprintf(tempFile, "%s\n", ligne);
        }
    }

    fclose(fichier);
    fclose(tempFile);

    // Remplacer le fichier original par le fichier temporaire
    remove(nomFichier);
    rename("temp.bin", nomFichier);

    printf("✅ Fichier '%s' déplacé vers '%s' avec succès !\n", nomfile, nomdossier);
}

// Fonction pour faire un split et renvoyer le premier élément
char* split_first(const char *str, const char *delim) {
    if (str == NULL || delim == NULL) {
        return NULL; // Vérification de sécurité
    }

    // Créer une copie de la chaîne pour éviter de modifier l'originale
    char *copy = strdup(str);
    if (copy == NULL) {
        return NULL; // Échec de l'allocation mémoire
    }

    // Utilisation de strtok pour obtenir le premier token
    char *token = strtok(copy, delim);
    
    // Sauvegarder le premier token avant de libérer la copie
    char *result = token ? strdup(token) : NULL;

    // Libérer la copie après utilisation
    free(copy);

    return result; // Retourne une nouvelle chaîne dynamique
}

// Fonction pour donner les horaires et les dates
void getCurrentDateTime(char *buffer, size_t size) {
    if (buffer == NULL || size < 20) {
        return; // Taille insuffisante pour stocker la date et l'heure
    }
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(buffer, size, "%d/%m/%Y %H:%M", &tm);
}

// Fonction pour écrire les métadonnées dans un fichier
void writeMetadata(FILE *file, const char *file_name, const char *date_creation, const char *last_edit) {
    if (file == NULL || file_name == NULL || date_creation == NULL || last_edit == NULL) {
        return; // Vérification de sécurité
    }
    
    const char *ident = identite;
    fprintf(file, "##D\n");
    fprintf(file, "file: %s\n", file_name);
    fprintf(file, "owner: %s\n", ident);
    fprintf(file, "date_creat: %s\n", date_creation);
    fprintf(file, "last_edit: %s\n", last_edit);
    fprintf(file, "##F\n");
}

// Fonction pour créer un nouveau fichier
void creerFichier(const char *nomFichier, char *file_name, char *nomdossier) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");
        return;
    }
    
    char cheminAct[MAX_CHEMIN];
    if (nomdossier == NULL) {
        snprintf(cheminAct, sizeof(cheminAct), "%s", cheminActuel);
    } else {
        snprintf(cheminAct, sizeof(cheminAct), "%s", nomdossier);
    }

    // Construire le chemin du fichier à créer
    char chemin[MAX_CHEMIN + MORE_CHEMIN];
    snprintf(chemin, sizeof(chemin), "%s%s", cheminAct, file_name);

    int file_exist = fileExistance(nomFichier, file_name, cheminAct);
    // Vérifier si le fichier existe déjà
    if (file_exist == 1) {
        printf("⚠️ Le fichier '%s' existe déjà.\n", file_name);
        return;
    }

    const char delim[] = ".";
    char *extension = file_name;
    char *first = split_first(extension, delim);

    printf("<f><f><f>:::::::::::<><><> %s\n\n", first);

    // Construire le chemin du fichier méta données
    char nomMeta[MAX_CHEMIN];
    char finalnomMeta[MAX_CHEMIN + MAX_CHEMIN + MORE_CHEMIN];
    snprintf(nomMeta, sizeof(nomMeta), "%s%s", first, ".meta");
    snprintf(finalnomMeta, sizeof(finalnomMeta), "%s%s", cheminAct, nomMeta);

    // Ouvrir le fichier en mode ajout binaire
    FILE *fichier = fopen(nomFichier, "ab");
    if (!fichier) {
        perror("Erreur de lecture");
        return;
    }
    
    fprintf(fichier, "%s\n", chemin);  // Écrire le chemin dans le fichier
    fprintf(fichier, "%s\n", finalnomMeta);  // Écrire le chemin des métadonnées

    // Récupérer la date actuelle
    char date_creation[20];
    char last_edit[20];
    getCurrentDateTime(date_creation, sizeof(date_creation));
    strcpy(last_edit, date_creation);

    // Écrire les métadonnées dans le fichier .meta
    writeMetadata(fichier, file_name, date_creation, last_edit);

    fclose(fichier);  // Fermer le fichier
    if (nomdossier == NULL) {
        printf(" 📄 Fichier '%s' créé avec succès dans ce dossier '%s' !\n", file_name, cheminAct);
    } 
}

// Permet de copier les éléments d'un fichier dans un nouveau
void copyFile(const char *nomFichier, const char *fichierSource, char *fichierDestination, char *nomdossier) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");
        return;
    }

    // Vérification des paramètres
    if (fichierSource == NULL || fichierDestination == NULL) {
        printf("⚠️ Les noms de fichiers ne peuvent pas être NULL.\n");
        return;
    }

    if (strcmp(fichierSource, fichierDestination) == 0) {
        printf("⚠️ Les noms de fichiers ne peuvent pas être identiques.\n");
        return;
    }

    // Vérifier si le fichier source existe
    if (fileExistance(nomFichier, fichierSource, NULL)==0) {
        printf("⚠️ Le fichier source '%s' n'existe pas.\n", fichierSource);
        return;
    }

    char cheminDestination[MAX_CHEMIN];
    if (nomdossier == NULL) {
        snprintf(cheminDestination, sizeof(cheminDestination), "%s%s", cheminActuel, fichierDestination);
    } else {
        if (repertoireExistance(nomFichier, nomdossier)) {
            snprintf(cheminDestination, sizeof(cheminDestination), "%s/%s", nomdossier, fichierDestination); 
        } 
        char cheminTest[MAX_CHEMIN+MAX_CHEMIN];
        snprintf(cheminTest, sizeof(cheminTest), "%s%s/", cheminActuel, nomdossier);
        if (repertoireExistance(nomFichier, cheminTest))  {
            snprintf(cheminDestination, sizeof(cheminDestination), "%s/%s", nomdossier, fichierDestination);    
        }
    }

    // Vérifier si le fichier Destination existe
    if (fileExistance(nomFichier, fichierDestination, cheminDestination)==1) {
        printf("⚠️ Le fichier Destination '%s' existe déja.\n", fichierDestination);
        return;
    }

    creerFichier(nomFichier, fichierDestination, nomdossier); 
    printf("✅ Fichier '%s' copié avec succès vers '%s'\n", fichierSource, fichierDestination);
}

// Fonction pour extraire les métadonnées d'un fichier
void extraireMetadata(FILE *fichier) {
    char ligne[MAX_CHEMIN];
    int in_section = 0;

    while (fgets(ligne, sizeof(ligne), fichier)) {
        ligne[strcspn(ligne, "\n")] = '\0'; // Supprimer le saut de ligne

        // Détecter le début de la section des métadonnées
        if (strstr(ligne, "##D") != NULL) {
            in_section = 1; // Commencer l'extraction
            continue;
        }

        // Détecter la fin de la section des métadonnées
        if (strstr(ligne, "##F") != NULL) {
            in_section = 0; // Arrêter l'extraction
            break;
        }

        // Afficher les métadonnées si on est dans la section
        if (in_section) {
            printf("%s\n", ligne);
        }
    }
}

// Fonction améliorée pour afficher les métadonnées d'un fichier
void metaDatafichier(const char *nomFichier, const char *file_name) {
    if (idUtilisateurConnecte == -1) {
        printf("⚠️ Veuillez vous connecter d'abord.\n");
        return;
    }

    // Vérifier si le fichier existe
    if (fileExistance(nomFichier, file_name, NULL)) {
        printf("⚠️ Le fichier '%s' n'existe pas.\n", file_name);
        return;
    }

    // Construire le chemin du fichier de métadonnées
    const char delim[] = ".";
    char *nom_sans_extension = split_first((char *)file_name, delim);
    if (nom_sans_extension == NULL) {
        printf("⚠️ Erreur lors de l'extraction du nom du fichier.\n");
        return;
    }

    char cheminMeta[MAX_CHEMIN+MAX_CHEMIN];
    snprintf(cheminMeta, sizeof(cheminMeta), "%s%s.meta", cheminActuel, nom_sans_extension);

    // Ouvrir le fichier de métadonnées
    FILE *fichier = fopen(nomFichier, "rb");
    if (!fichier) {
        perror("⚠️ Erreur lors de l'ouverture du fichier");
        return;
    }

    // Afficher les métadonnées
    printf("\n📄 Métadonnées du fichier '%s' :\n", nom_sans_extension);
    extraireMetadata(fichier);

    fclose(fichier);
}

// Fonction principale
int main(int argc, char *argv[]) {
    const char *nomFichier = PARTITION;
    char input[MAX_INPUT];  // Buffer pour stocker l'entrée utilisateur

    // Initialiser le disque virtuel s'il n'existe pas
    if (!fichierExiste(nomFichier)) initialiserDisqueVirtuel(nomFichier);

    // Vérifier les arguments de la ligne de commande
    if (argc < 2) return printf("Usage: %s -account <nom> | -show | -showus | -connect <nom>\n", argv[0]), 1;

    // Gérer les commandes
    if (strcmp(argv[1], "-account") == 0 && argc == 3) {
        ajouterUtilisateur(nomFichier, argv[2]);  // Ajouter un utilisateur
    } else if (strcmp(argv[1], "-show") == 0) {
        afficherDisqueVirtuel(nomFichier);  // Afficher le contenu du disque virtuel
    } else if (strcmp(argv[1], "-showus") == 0) {
        afficherUslist(nomFichier);  // Afficher la liste des utilisateurs
    } else if (strcmp(argv[1], "-connect") == 0 && argc == 3) {
        // Connecter un utilisateur et entrer dans la boucle de commandes
        if (connectionCompte(nomFichier, argv[2])) {
            while (1) {
                printf("\nCommande (-exit pour quitter) : ");
                fgets(input, MAX_INPUT, stdin);  // Lire l'entrée utilisateur
                input[strcspn(input, "\n")] = 0;  // Supprimer le saut de ligne

                // Gérer les commandes de l'utilisateur
                if (strcmp(input, "-exit") == 0) break;  // Quitter la boucle
                else if (strcmp(input, "-pwd") == 0) afficherCheminActuel();  // Afficher le chemin actuel
                else if (strncmp(input, "-mkdir ", 7) == 0) creerRepertoireUtilisateur(nomFichier, input + 7);  // Créer un répertoire
                else if (strcmp(input, "-lt") == 0) listerRepertoiresFichiers(nomFichier);  // Lister les répertoires et fichiers
                else if (strncmp(input, "-cd ", 4) == 0) changerRepertoire(input + 4);  // Changer de répertoire
                else if (strncmp(input, "-rmdir ", 7) == 0) supprimerRepertoire(nomFichier, input + 7);  // Supprimer un répertoire
                else if (strncmp(input, "-rm ", 4) == 0) supprimerFichier(nomFichier, input + 4);  // Supprimer un fichier
                else if (strncmp(input, "-touch ", 7) == 0) creerFichier(nomFichier, input + 7,NULL);  // Créer un fichier
                else if (strncmp(input, "-meta", 5) == 0) metaDatafichier(nomFichier, input + 7);  // Afficher le contenu d'un fichier
                else if (strncmp(input, "-cp ", 4) == 0) {
                    // Extraire les arguments pour la commande -cp
                    char *args = input + 4;  // Pointeur vers le début des arguments
                    char *fichierSource = strtok(args, " ");  // Premier argument : fichier source
                    char *fichierDestination = strtok(NULL, " ");  // Deuxième argument : fichier destination
                    char *nomdossier = strtok(NULL, " ");  // Troisième argument : nom du dossier (facultatif)

                    // Vérifier que les deux premiers arguments sont présents
                    if (fichierSource == NULL || fichierDestination == NULL) {
                        printf("⚠️ Usage: -cp <fichierSource> <fichierDestination> [<nomDossier>]\n");
                    } else {
                        // Appeler la fonction copyFile avec les arguments extraits
                        copyFile(nomFichier, fichierSource, fichierDestination, nomdossier);
                    }
                }
                else if (strncmp(input, "-mv ", 4) == 0) {
                    // Extraire les arguments pour la commande -mv
                    char *args = input + 4;  // Pointeur vers le début des arguments
                    char *nomfile = strtok(args, " ");  // Premier argument : nom du fichier
                    char *nomdossier = strtok(NULL, " ");  // Deuxième argument : nom du dossier

                    // Vérifier que les deux arguments sont présents
                    if (nomfile == NULL || nomdossier == NULL) {
                        printf("⚠️ Usage: -mv <nomfile> <nomdossier>\n");
                    } else {
                        // Appeler la fonction deplacerFichier avec les arguments extraits
                        deplacerFichier(nomFichier, nomfile, nomdossier);
                    }
                }
                else {
                    printf("⚠️ Commande inconnue.\n");
                }
            }
        }
    }
    return 0;
}
