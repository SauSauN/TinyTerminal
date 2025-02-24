#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Définir les tailles maximales pour les noms, les chemins et les entrées utilisateur
#define MAX_NOM 50
#define MAX_INPUT 100
#define MAX_CHEMIN 1000  
#define MORE_CHEMIN 100

// Structure pour représenter un utilisateur
typedef struct {
    int id;                  // Identifiant unique de l'utilisateur
    char nom[MAX_NOM];       // Nom de l'utilisateur
} Utilisateur;

// Variables globales
int idUtilisateurConnecte = -1;  // ID de l'utilisateur connecté (-1 si aucun utilisateur n'est connecté)
char ident[MAX_NOM] = "";        // Nom de l'utilisateur connecté
char cheminActuel[MAX_CHEMIN] = "";  // Chemin actuel dans le système de fichiers virtuel

// Fonction pour vérifier si un fichier existe
int fichierExiste(const char *nomFichier) {
    FILE *fichier = fopen(nomFichier, "rb");  // Ouvrir le fichier en mode lecture binaire
    if (fichier) {
        fclose(fichier);  // Fermer le fichier s'il existe
        return 1;         // Retourner 1 si le fichier existe
    }
    return 0;  // Retourner 0 si le fichier n'existe pas
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
                break;
            }
        }
    }
    fclose(fichier);

    if (trouve) {
        printf("✅ Connexion réussie : %s\n", identifiant);
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
    char cheminUtilisateur[MAX_CHEMIN];
    // Construire le chemin de l'utilisateur connecté
    snprintf(cheminUtilisateur, sizeof(cheminUtilisateur), "User/id%d_%s/", idUtilisateurConnecte, ident);

    printf("\n📂 Contenu de votre espace (ID racine : %d) :\n", idUtilisateurConnecte); // Afficher l'ID du répertoire racine
    printf("────────────────────────────────\n");

    int trouve = 0;
    // Lire le fichier ligne par ligne et afficher les répertoires et fichiers
    while (fgets(ligne, sizeof(ligne), fichier)) {
        if (strncmp(ligne, cheminUtilisateur, strlen(cheminUtilisateur)) == 0) {
            printf("📁 %s", ligne + strlen(cheminUtilisateur));  // Afficher le contenu
            trouve = 1;
        }
    }

    fclose(fichier);  // Fermer le fichier
    if (!trouve) printf("📁 Aucun fichier ou dossier trouvé.\n");  // Afficher un message si aucun contenu n'est trouvé
}

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

    char chemin[MAX_CHEMIN];
    // Construire le chemin du répertoire à supprimer
    snprintf(chemin, sizeof(chemin), "User/id%d_%s/%s/\n", idUtilisateurConnecte, ident, nomRepertoire);

    FILE *fichier = fopen(nomFichier, "rb");
    if (!fichier) {
        perror("Erreur de lecture");
        return;
    }

    FILE *tempFile = fopen("temp.bin", "wb");
    if (!tempFile) {
        perror("Erreur de création du fichier temporaire");
        fclose(fichier);
        return;
    }

    char ligne[MAX_CHEMIN];
    int trouve = 0;

    // Lire le fichier ligne par ligne et copier les lignes sauf celle correspondant au répertoire à supprimer
    while (fgets(ligne, sizeof(ligne), fichier)) {
        if (strcmp(ligne, chemin)) {
            fputs(ligne, tempFile);
        } else {
            trouve = 1;
        }
    }

    fclose(fichier);
    fclose(tempFile);

    if (trouve) {
        // Remplacer le fichier original par le fichier temporaire
        remove(nomFichier);
        rename("temp.bin", nomFichier);
        printf("📁 Répertoire '%s' supprimé avec succès !\n", nomRepertoire);
    } else {
        remove("temp.bin");
        printf("⚠️ Répertoire '%s' non trouvé.\n", nomRepertoire);
    }
}


// Fonction principale
int main(int argc, char *argv[]) {
    const char *nomFichier = "projet.bin";  // Nom du fichier du disque virtuel
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
                else if (strcmp(input, "-mypwd") == 0) afficherCheminActuel();  // Afficher le chemin actuel
                else if (strncmp(input, "-mkdir ", 7) == 0) creerRepertoireUtilisateur(nomFichier, input + 7);  // Créer un répertoire
                else if (strcmp(input, "-mylt") == 0) listerRepertoiresFichiers(nomFichier);  // Lister les répertoires et fichiers
                else if (strncmp(input, "-cd ", 4) == 0) changerRepertoire(input + 4);  // Changer de répertoire
                else if (strncmp(input, "-rmdir ", 7) == 0) supprimerRepertoire(nomFichier, input + 7);  // Nouvelle commande
                else printf("⚠️ Commande inconnue.\n");
            }
        }
    }
    return 0;
}
