# Projet de Système de Fichiers Virtuel

Ce projet est une implémentation simple d'un système de fichiers virtuel en C. Il permet de gérer des utilisateurs, des répertoires et des fichiers dans un environnement simulé. Le programme utilise un fichier binaire (`projet.bin`) pour stocker les données du système de fichiers.

## Fonctionnalités

### Gestion des utilisateurs :
- Créer un nouvel utilisateur.
- Se connecter en tant qu'utilisateur.
- Afficher la liste des utilisateurs.

### Gestion des répertoires :
- Créer un répertoire.
- Changer de répertoire (`cd`).
- Lister les répertoires et fichiers dans le répertoire actuel.

### Affichage :
- Afficher le contenu du disque virtuel.
- Afficher le chemin actuel (`pwd`).

## Utilisation

### Compilation

Pour compiler le programme, utilisez la commande suivante :

```bash
gcc -Wall fil1.c -o exe
```

Cela génère un exécutable nommé `exe`.

### Exécution

Le programme prend des arguments en ligne de commande pour effectuer différentes actions. Voici les commandes disponibles :

#### Créer un utilisateur :
```bash
./exe -account <nom_utilisateur>
```
Exemple :
```bash
./exe -account alice
```

#### Afficher le contenu du disque virtuel :
```bash
./exe -show
```

#### Afficher la liste des utilisateurs :
```bash
./exe -showus
```

#### Se connecter en tant qu'utilisateur :
```bash
./exe -connect <nom_utilisateur>
```
Exemple :
```bash
./exe -connect alice
```

Une fois connecté, vous pouvez utiliser les commandes suivantes :

- **Afficher le chemin actuel :**
  ```bash
  -mypwd
  ```

- **Créer un répertoire :**
  ```bash
  -mkdir <nom_répertoire>
  ```
  Exemple :
  ```bash
  -mkdir documents
  ```

- **Changer de répertoire :**
  ```bash
  -cd <nom_répertoire>
  ```
  Exemple :
  ```bash
  -cd documents
  ```

- **Lister les répertoires et fichiers :**
  ```bash
  -myls
  ```

- **Quitter la session :**
  ```bash
  -exit
  ```

## Structure du Projet

- `fil1.c` : Le fichier source contenant le code du programme.
- `projet.bin` : Le fichier binaire utilisé pour stocker les données du système de fichiers virtuel. Ce fichier est créé automatiquement lors de la première exécution du programme.

## Exemple d'Utilisation

```bash
./exe -account alice
./exe -connect alice
-mkdir documents
-cd documents
-mypwd
-myls
-exit
```

## Avertissements

- Le programme utilise un fichier binaire (`projet.bin`) pour stocker les données. **Ne modifiez pas ce fichier manuellement**, car cela pourrait corrompre le système de fichiers virtuel.
- Le programme ne gère pas encore les permissions ou les fichiers individuels. Il se concentre sur la gestion des répertoires et des utilisateurs.

