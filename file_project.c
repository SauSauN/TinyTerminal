#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Définir les tailles maximales pour les noms, les chemins et les entrées utilisateur
#define MAX_NOM 50
#define MAX_INPUT 100
#define MAX_CHEMIN 1000

// Structure pour représenter un utilisateur
typedef struct
{
    int id;            // Identifiant unique de l'utilisateur
    char nom[MAX_NOM]; // Nom de l'utilisateur
} Utilisateur;

// Variables globales
int idUtilisateurConnecte = -1;     // ID de l'utilisateur connecté (-1 si aucun utilisateur n'est connecté)
char ident[MAX_NOM] = "";           // Nom de l'utilisateur connecté
char cheminActuel[MAX_CHEMIN] = ""; // Chemin actuel dans le système de fichiers virtuel

// Fonction pour vérifier si un fichier existe
int fichierExiste(const char *nomFichier)
{
    FILE *fichier = fopen(nomFichier, "rb"); // Ouvrir le fichier en mode lecture binaire
    if (fichier)
    {
        fclose(fichier); // Fermer le fichier s'il existe
        return 1;        // Retourner 1 si le fichier existe
    }
    return 0; // Retourner 0 si le fichier n'existe pas
}

// Fonction pour initialiser le disque virtuel
void initialiserDisqueVirtuel(const char *nomFichier)
{
    FILE *fichier = fopen(nomFichier, "wb"); // Ouvrir le fichier en mode écriture binaire
    if (!fichier)
    {
        perror("Erreur de création du disque virtuel"); // Afficher une erreur si le fichier ne peut pas être créé
        exit(1);                                        // Quitter le programme en cas d'erreur
    }
    // Écrire la structure de base du disque virtuel
    fprintf(fichier, "Répertoire: File/\n");
    fprintf(fichier, "Répertoire: User/\n");
    fclose(fichier);                                            // Fermer le fichier
    printf("📂 Disque virtuel '%s' initialisé.\n", nomFichier); // Confirmer l'initialisation
}

// Fonction pour générer un ID unique pour un nouvel utilisateur
int genererID(const char *nomFichier)
{
    FILE *fichier = fopen(nomFichier, "rb"); // Ouvrir le fichier en mode lecture binaire
    if (!fichier)
        return 10; // Retourner 10 si le fichier ne peut pas être ouvert

    int id, maxID = 9; // Initialiser maxID à 9
    char ligne[MAX_CHEMIN];

    // Lire le fichier ligne par ligne pour trouver le plus grand ID existant
    while (fgets(ligne, sizeof(ligne), fichier))
    {
        if (sscanf(ligne, "File/Uslist.txt: %d", &id) == 1)
        {
            if (id > maxID)
                maxID = id; // Mettre à jour maxID si un ID plus grand est trouvé
        }
    }
    fclose(fichier);  // Fermer le fichier
    return maxID + 1; // Retourner le prochain ID disponible
}

// Fonction pour vérifier si un utilisateur existe déjà
int utilisateurExiste(const char *nomFichier, const char *nomUtilisateur)
{
    FILE *fichier = fopen(nomFichier, "rb"); // Ouvrir le fichier en mode lecture binaire
    if (!fichier)
        return 0; // Retourner 0 si le fichier ne peut pas être ouvert

    char ligne[MAX_CHEMIN], nom[MAX_NOM];
    int id;

    // Lire le fichier ligne par ligne pour vérifier si l'utilisateur existe
    while (fgets(ligne, sizeof(ligne), fichier))
    {
        if (sscanf(ligne, "File/Uslist.txt: %d %s", &id, nom) == 2)
        {
            if (strcmp(nom, nomUtilisateur) == 0)
            {                    // Comparer le nom de l'utilisateur
                fclose(fichier); // Fermer le fichier
                return 1;        // Retourner 1 si l'utilisateur existe
            }
        }
    }
    fclose(fichier); // Fermer le fichier
    return 0;        // Retourner 0 si l'utilisateur n'existe pas
}

// Fonction pour ajouter un nouvel utilisateur
void ajouterUtilisateur(const char *nomFichier, const char *nomUtilisateur)
{
    if (utilisateurExiste(nomFichier, nomUtilisateur))
    { // Vérifier si l'utilisateur existe déjà
        printf("⚠️ Utilisateur '%s' existe déjà.\n", nomUtilisateur);
        return;
    }

    int id = genererID(nomFichier);          // Générer un nouvel ID pour l'utilisateur
    FILE *fichier = fopen(nomFichier, "ab"); // Ouvrir le fichier en mode ajout binaire
    if (!fichier)
    {
        perror("Erreur d'écriture"); // Afficher une erreur si le fichier ne peut pas être ouvert
        return;
    }

    // Ajouter l'utilisateur au fichier
    fprintf(fichier, "File/Uslist.txt: %d %s\n", id, nomUtilisateur);
    fprintf(fichier, "User/id%d_%s/\n", id, nomUtilisateur);
    fclose(fichier);                                                      // Fermer le fichier
    printf("✅ Utilisateur '%s' créé avec ID %d.\n", nomUtilisateur, id); // Confirmer la création
}

// Fonction pour afficher le contenu du disque virtuel
void afficherDisqueVirtuel(const char *nomFichier)
{
    FILE *fichier = fopen(nomFichier, "rb"); // Ouvrir le fichier en mode lecture binaire
    if (!fichier)
    {
        perror("Erreur de lecture"); // Afficher une erreur si le fichier ne peut pas être ouvert
        return;
    }

    char ligne[MAX_CHEMIN];
    printf("\n📂 Contenu du disque virtuel :\n");
    // Lire et afficher chaque ligne du fichier
    while (fgets(ligne, sizeof(ligne), fichier))
    {
        printf("%s", ligne);
    }
    fclose(fichier); // Fermer le fichier
}

// Fonction pour afficher la liste des utilisateurs
void afficherUslist(const char *nomFichier)
{
    FILE *fichier = fopen(nomFichier, "rb"); // Ouvrir le fichier en mode lecture binaire
    if (!fichier)
        return;

    char ligne[MAX_CHEMIN];
    printf("\n📄 Liste des utilisateurs :\n");

    // Lire le fichier ligne par ligne et afficher les utilisateurs
    while (fgets(ligne, sizeof(ligne), fichier))
    {
        if (strncmp(ligne, "File/Uslist.txt:", 16) == 0)
        {
            printf("%s", ligne + 17); // Afficher la ligne sans le préfixe
        }
    }
    fclose(fichier); // Fermer le fichier
}

// Fonction pour connecter un utilisateur
int connectionCompte(const char *nomFichier, const char *identifiant)
{
    FILE *fichier = fopen(nomFichier, "rb"); // Ouvrir le fichier en mode lecture binaire
    if (!fichier)
        return 0; // Retourner 0 si le fichier ne peut pas être ouvert

    char ligne[MAX_CHEMIN], nom[MAX_NOM];
    int id, trouve = 0;

    // Lire le fichier ligne par ligne pour trouver l'utilisateur
    while (fgets(ligne, sizeof(ligne), fichier))
    {
        if (sscanf(ligne, "File/Uslist.txt: %d %s", &id, nom) == 2)
        {
            if (strcmp(nom, identifiant) == 0)
            { // Comparer le nom de l'utilisateur
                trouve = 1;
                idUtilisateurConnecte = id; // Mettre à jour l'ID de l'utilisateur connecté
                strcpy(ident, identifiant); // Mettre à jour le nom de l'utilisateur connecté
                break;
            }
        }
    }
    fclose(fichier); // Fermer le fichier

    if (trouve)
    {
        printf("✅ Connexion réussie : %s\n", identifiant); // Confirmer la connexion
        return 1;
    }
    else
    {
        printf("⚠️ Utilisateur non trouvé.\n"); // Afficher un message d'erreur
        return 0;
    }
}

// Fonction pour afficher le chemin actuel
void afficherCheminActuel()
{
    if (idUtilisateurConnecte != -1)
        printf("📂 Vous êtes dans : User/id%d_%s\n", idUtilisateurConnecte, ident); // Afficher le chemin actuel
    else
        printf("⚠️ Aucun utilisateur connecté.\n"); // Afficher un message d'erreur
}

// Fonction pour créer un répertoire utilisateur
void creerRepertoireUtilisateur(const char *nomFichier, const char *nomRepertoire)
{
    if (idUtilisateurConnecte == -1)
    {
        printf("⚠️ Veuillez vous connecter d'abord.\n"); // Afficher un message d'erreur
        return;
    }

    char chemin[MAX_CHEMIN];
    // Construire le chemin du nouveau répertoire
    snprintf(chemin, sizeof(chemin), "User/id%d_%s/%s/\n", idUtilisateurConnecte, ident, nomRepertoire);

    FILE *fichier = fopen(nomFichier, "ab"); // Ouvrir le fichier en mode ajout binaire
    if (!fichier)
    {
        perror("Erreur d'écriture"); // Afficher une erreur si le fichier ne peut pas être ouvert
        return;
    }
    fprintf(fichier, "%s", chemin);                                   // Écrire le chemin dans le fichier
    fclose(fichier);                                                  // Fermer le fichier
    printf("📁 Répertoire '%s' créé avec succès !\n", nomRepertoire); // Confirmer la création
}

// Fonction pour lister les répertoires et fichiers d'un utilisateur
void listerRepertoiresFichiers(const char *nomFichier)
{
    if (idUtilisateurConnecte == -1)
    {
        printf("⚠️ Veuillez vous connecter d'abord.\n"); // Afficher un message d'erreur
        return;
    }

    FILE *fichier = fopen(nomFichier, "rb"); // Ouvrir le fichier en mode lecture binaire
    if (!fichier)
    {
        perror("Erreur de lecture"); // Afficher une erreur si le fichier ne peut pas être ouvert
        return;
    }

    char ligne[MAX_CHEMIN];
    char cheminUtilisateur[MAX_CHEMIN];
    // Construire le chemin de l'utilisateur connecté
    snprintf(cheminUtilisateur, sizeof(cheminUtilisateur), "User/id%d_%s/", idUtilisateurConnecte, ident);

    printf("\n📂 Contenu de votre espace (%s) :\n", cheminUtilisateur);
    printf("────────────────────────────────\n");

    int trouve = 0;
    // Lire le fichier ligne par ligne et afficher les répertoires et fichiers
    while (fgets(ligne, sizeof(ligne), fichier))
    {
        if (strncmp(ligne, cheminUtilisateur, strlen(cheminUtilisateur)) == 0)
        {
            printf("📁 %s", ligne + strlen(cheminUtilisateur)); // Afficher le contenu
            trouve = 1;
        }
    }

    fclose(fichier); // Fermer le fichier
    if (!trouve)
        printf("📁 Aucun fichier ou dossier trouvé.\n"); // Afficher un message si aucun contenu n'est trouvé
}

// Fonction pour changer de répertoire
void changerRepertoire(const char *nomDossier)
{
    if (idUtilisateurConnecte == -1)
    {
        printf("⚠️ Veuillez vous connecter d'abord.\n"); // Afficher un message d'erreur
        return;
    }

    // Calculer la taille requise pour le nouveau chemin
    size_t requiredSize = strlen(cheminActuel) + strlen(nomDossier) + 3; // +3 pour '/', '/' et '\0'
    char *nouveauChemin = malloc(requiredSize);                          // Allouer de la mémoire pour le nouveau chemin
    if (!nouveauChemin)
    {
        perror("Erreur d'allocation mémoire"); // Afficher une erreur si l'allocation échoue
        return;
    }

    // Cas 1 : Remonter d'un niveau (cd ..)
    if (strcmp(nomDossier, "..") == 0)
    {
        char *dernierSlash = strrchr(cheminActuel, '/'); // Trouver le dernier '/'
        if (dernierSlash != NULL && dernierSlash != cheminActuel)
        {
            *dernierSlash = '\0'; // Tronquer le chemin au dernier '/'
        }
        else
        {
            printf("⚠️ Impossible de remonter plus haut.\n"); // Afficher un message d'erreur
        }
    }
    // Cas 2 : Descendre dans un répertoire (cd <nom_du_répertoire>)
    else
    {
        // Construire le nouveau chemin avec un '/' à la fin
        snprintf(nouveauChemin, requiredSize, "%s/%s/", cheminActuel, nomDossier);

        // Vérifier si le répertoire existe dans le disque virtuel
        FILE *fichier = fopen("projet.bin", "rb"); // Ouvrir le fichier en mode lecture binaire
        if (!fichier)
        {
            perror("Erreur de lecture"); // Afficher une erreur si le fichier ne peut pas être ouvert
            free(nouveauChemin);         // Libérer la mémoire allouée
            return;
        }

        char ligne[MAX_CHEMIN];
        int trouve = 0;
        // Lire le fichier ligne par ligne pour vérifier si le répertoire existe
        while (fgets(ligne, sizeof(ligne), fichier))
        {
            ligne[strcspn(ligne, "\n")] = '\0'; // Supprimer le saut de ligne
            if (strcmp(ligne, nouveauChemin) == 0)
            {
                trouve = 1;
                break;
            }
        }
        fclose(fichier); // Fermer le fichier

        if (trouve)
        {
            // Mettre à jour le chemin actuel
            strcpy(cheminActuel, nouveauChemin);
        }
        else
        {
            printf("⚠️ Répertoire non trouvé.\n"); // Afficher un message d'erreur
        }
    }

    // Libérer la mémoire allouée
    free(nouveauChemin);

    // Afficher le chemin actuel
    printf("📂 Répertoire actuel: %s\n", cheminActuel);
}

/* Fonction pour écrire les métadonnées dans le fichier */
void writeMetadata(FILE *file, const char *file_name, const char *date_creation, const char *last_edit)
{
    fprintf(file, "##D\n");
    fprintf(file, "file: %s\n", file_name);
    fprintf(file, "owner: %s\n", ident);
    fprintf(file, "date_creat: %s\n", date_creation);
    fprintf(file, "last_edit: %s\n", last_edit);
    fprintf(file, "##F\n");
}

/* Fonction pour obtenir la date et l'heure actuelles sous forme de chaîne */
void getCurrentDateTime(char *buffer, size_t size)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(buffer, size, "%d/%m/%Y %H:%M", &tm);
}

// Permet de copier les éléments d'un fichier dans un nouveau
void copyFile(const char *fichierSource, const char *fichierDestination)
{
    if (idUtilisateurConnecte == -1)
    {
        printf("⚠️ Veuillez vous connecter d'abord.\n");
        return;
    }

    // Vérification des paramètres
    if (!fichierSource || !fichierDestination)
    {
        printf("⚠️ Les noms de fichiers ne peuvent pas être vides.\n");
        return;
    }

    // Construction des chemins complets avec le chemin actuel
    char cheminSource[MAX_CHEMIN];
    char cheminDestination[MAX_CHEMIN];
    char metaSource[MAX_CHEMIN];
    char metaDestination[MAX_CHEMIN];

    // Construire les chemins des fichiers et leurs métadonnées
    snprintf(cheminSource, sizeof(cheminSource), "%s%s", cheminActuel, fichierSource);
    snprintf(cheminDestination, sizeof(cheminDestination), "%s%s", cheminActuel, fichierDestination);
    snprintf(metaSource, sizeof(metaSource), "%.*s.meta", 
             (int)(strrchr(fichierSource, '.') - fichierSource), fichierSource);
    snprintf(metaDestination, sizeof(metaDestination), "%.*s.meta", 
             (int)(strrchr(fichierDestination, '.') - fichierDestination), fichierDestination);

    // Ouvrir le fichier source
    FILE *source = fopen(fichierSource, "r");
    if (!source)
    {
        printf("⚠️ Erreur : Impossible d'ouvrir le fichier source '%s'\n", fichierSource);
        return;
    }

    // Créer le fichier destination
    FILE *destination = fopen(fichierDestination, "w");
    if (!destination)
    {
        printf("⚠️ Erreur : Impossible de créer le fichier destination '%s'\n", fichierDestination);
        fclose(source);
        return;
    }

    // Copier le contenu
    char ligne[MAX_CHEMIN];
    while (fgets(ligne, sizeof(ligne), source))
    {
        fputs(ligne, destination);
    }

    // Fermer les fichiers
    fclose(source);
    fclose(destination);

    // Créer et écrire les métadonnées pour le fichier destination
    FILE *metaDest = fopen(metaDestination, "w");
    if (metaDest)
    {
        char dateActuelle[20];
        getCurrentDateTime(dateActuelle, sizeof(dateActuelle));
        
        fprintf(metaDest, "##D\n");
        fprintf(metaDest, "file: %s\n", fichierDestination);
        fprintf(metaDest, "owner: %s\n", ident);
        fprintf(metaDest, "date_creat: %s\n", dateActuelle);
        fprintf(metaDest, "last_edit: %s\n", dateActuelle);
        fprintf(metaDest, "##F\n");
        
        fclose(metaDest);
    }

    // Mettre à jour le disque virtuel
    FILE *disque = fopen("projet.bin", "a");
    if (disque)
    {
        fprintf(disque, "%s\n", cheminDestination);
        fclose(disque);
    }

    printf("✅ Fichier '%s' copié avec succès vers '%s'\n", fichierSource, fichierDestination);
}

// Fonction principale
int main(int argc, char *argv[])
{
    const char *nomFichier = "projet.bin"; // Nom du fichier du disque virtuel
    char input[MAX_INPUT];                 // Buffer pour stocker l'entrée utilisateur

    // Initialiser le disque virtuel s'il n'existe pas
    if (!fichierExiste(nomFichier))
        initialiserDisqueVirtuel(nomFichier);

    // Vérifier les arguments de la ligne de commande
    if (argc < 2)
        return printf("Usage: %s -account <nom> | -show | -showus | -connect <nom>\n", argv[0]), 1;

    // Gérer les commandes
    if (strcmp(argv[1], "-account") == 0 && argc == 3)
    {
        ajouterUtilisateur(nomFichier, argv[2]); // Ajouter un utilisateur
    }
    else if (strcmp(argv[1], "-show") == 0)
    {
        afficherDisqueVirtuel(nomFichier); // Afficher le contenu du disque virtuel
    }
    else if (strcmp(argv[1], "-showus") == 0)
    {
        afficherUslist(nomFichier); // Afficher la liste des utilisateurs
    }
    else if (strcmp(argv[1], "-connect") == 0 && argc == 3)
    {
        // Connecter un utilisateur et entrer dans la boucle de commandes
        if (connectionCompte(nomFichier, argv[2]))
        {
            while (1)
            {
                printf("\nCommande (-exit pour quitter) : ");
                fgets(input, MAX_INPUT, stdin);  // Lire l'entrée utilisateur
                input[strcspn(input, "\n")] = 0; // Supprimer le saut de ligne

                // Gérer les commandes de l'utilisateur
                if (strcmp(input, "-exit") == 0)
                    break; // Quitter la boucle
                else if (strcmp(input, "-mypwd") == 0)
                    afficherCheminActuel(); // Afficher le chemin actuel
                else if (strncmp(input, "-mkdir ", 7) == 0)
                    creerRepertoireUtilisateur(nomFichier, input + 7); // Créer un répertoire
                else if (strcmp(input, "-myls") == 0)
                    listerRepertoiresFichiers(nomFichier); // Lister les répertoires et fichiers
                else if (strncmp(input, "-cd ", 4) == 0)
                    changerRepertoire(input + 4); // Changer de répertoire
                                                  // Ajouter cette nouvelle condition pour la commande copy
                else if (strncmp(input, "-copy ", 6) == 0)
                {
                    char source[MAX_NOM], destination[MAX_NOM];
                    // Extraire les noms des fichiers source et destination
                    if (sscanf(input + 6, "%s %s", source, destination) == 2)
                    {
                        copyFile(source, destination);
                    }
                    else
                    {
                        printf("⚠️ Usage : -copy <fichier_source> <fichier_destination>\n");
                    }
                }
                else
                    printf("⚠️ Commande inconnue.\n");
            }
        }
    }
    return 0; // Terminer le programme
}
