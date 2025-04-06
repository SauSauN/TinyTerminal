# 📁 Système de Fichiers Virtuel — Shell Interactif

Bienvenue dans le système de fichiers virtuel ! Ce programme simule un shell UNIX simplifié permettant la gestion complète de fichiers, dossiers, permissions, liens, groupes, et utilisateurs, avec des privilèges sudo/admin/superadmin.

---

## 📥 Guide d’installation

### Prérequis

- Un système UNIX/Linux
- Un compilateur C (ex: `gcc`)
- `make` installé

### Installation

```bash
git clone <repo>
cd <repo>
make
```

### Exécution

```bash
./filesystem
```

---

## 👤 Connexion Utilisateur

- Lors du premier lancement, entrez un **nom d'utilisateur**.  
- Si l'utilisateur existe déjà, il sera invité à entrer son **mot de passe**.  
- Sinon, un compte sera **créé automatiquement**, avec mot de passe, répertoire personnel et rôle initial :
  - Le **premier utilisateur** devient **superadmin**.
  - Les suivants sont des utilisateurs simples.

---

## 📜 Commandes Disponibles

### 🔧 Commandes de base

- `help` — Affiche cette aide
  - Affiche une liste complète de toutes les commandes disponibles avec une description succincte de chacune.

- `exit` — Quitte le shell
  - Ferme le programme et retourne à votre terminal initial.

- `clear` — Efface l’écran
  - Nettoie l’affichage du terminal pour une meilleure lisibilité.
  
- `whoami` — Affiche l'utilisateur actuel
  - Indique le nom de l'utilisateur connecté en cours.
  
- `pwd` — Affiche le répertoire courant
  - Affiche le chemin absolu du répertoire dans lequel vous travaillez actuellement.

### 📁 Gestion des répertoires

- `rmdir <nom>` — Supprime un répertoire
  - Supprime un répertoire vide portant le nom spécifié.
  
- `ls` — Liste le contenu du répertoire
  - Affiche la liste des fichiers et des sous-répertoires présents dans le répertoire courant.
  
- `lsl` — Liste avec métadonnées détaillées
  - Similaire à ls, mais affiche également des informations supplémentaires (permissions, taille, date de modification).

- `statd <nom>` — Affiche les métadonnées d'un répertoire
  - Fournit des détails comme les permissions, le propriétaire et d'autres attributs du répertoire spécifié.
  
- `mkdir <nom> [répertoire]` — Crée un répertoire
  - Crée un nouveau répertoire avec le nom indiqué. Un chemin optionnel permet de définir l'emplacement de création.

  - `mkdir <nom> rep` — Crée un répertoire dans le répertoire 'rep'
    - Place le nouveau dossier dans le sous-répertoires rep.

  - `mkdir <nom> rep/sousrep/etc` — Crée un répertoire dans les sous-répertoires
    - Permet de créer un répertoire au sein d'une arborescence de dossiers, en créant éventuellement des sous-répertoires si nécessaire.
  
- `cd <nom>` — Change de répertoire
  - Modifie le répertoire courant pour se positionner dans celui indiqué.

  - `cd ..` — Remonte d'un niveau
    - Permet de revenir au répertoire parent du répertoire courant.

  - `cd rep` — Va dans le répertoire 'rep'
    - Change le répertoire courant pour le répertoire spécifié.

  - `cd rep/sousrep/etc` — Chemin relatif
    - Permet de naviguer dans une arborescence de répertoires en spécifiant un chemin relatif.

- `cpdir <src> <dest> [répertoire]` — Copie un répertoire
  - Duplique un répertoire et son contenu vers la destination spécifiée.

  - `cpdir <src> <dest>` — Copie un répertoire dans le répertoire actuel
    - Crée une copie du répertoire source dans le répertoire courant.

  - `cpdir <src> <dest> ..` — Copie un répertoire dans le répertoire parent
    - Crée une copie du répertoire source dans le répertoire parent.

  - `cpdir <src> <dest> rep` — Copie un répertoire dans le répertoire 'rep'
    - Crée une copie du répertoire source dans le répertoire spécifié.
  - `cpdir <src> <dest> rep/sousrep/etc` — Copie un répertoire dans les sous-répertoires
    - Crée une copie du répertoire source dans les sous-répertoires spécifiés.

- `mvdir <src> <répertoire>` — Déplace un répertoire
  - Déplace un répertoire vers un nouvel emplacement.

  - `mvdir <src> ..` — Déplace un répertoire dans le répertoire parent
    - Déplace le répertoire source vers le répertoire parent.

  - `mvdir <src> rep` — Déplace un répertoire dans le répertoire 'rep'
    - Déplace le répertoire source vers le répertoire spécifié.

  - `mvdir <src> rep/sousrep/etc` — Déplace un répertoire dans les sous-répertoires
    - Déplace le répertoire source vers les sous-répertoires spécifiés.

### 📄 Gestion des fichiers

- `touch <nom>` — Crée un fichier vide
  - Crée un nouveau fichier vide avec le nom spécifié.

- `statf <nom>` — Affiche les métadonnées d'un fichier
  - Fournit des détails comme les permissions, le propriétaire et d'autres attributs du fichier spécifié.

- `write <nom> <cont>` — Écrit dans un fichier
  - Écrit le contenu spécifié dans le fichier indiqué.

- `cat <nom>` — Affiche le contenu d'un fichier
  - Affiche le contenu du fichier spécifié dans le terminal.

- `rm <nom>` — Supprime un fichier
  - Supprime le fichier spécifié du système.

- `cp <src> <dest> [répertoire]` — Copie un fichier
  - Copie un fichier vers une nouvelle destination. Le nom du répertoire est optionnel.

  - `cp <src> <dest>` — Copie un fichier dans un répertoire actuel
    - Crée une copie du fichier source dans le répertoire courant.

  - `cp <src> <dest> ..` — Copie un fichier dans un répertoire parent
    - Crée une copie du fichier source dans le répertoire parent.

  - `cp <src> <dest> rep` — Copie un fichier dans un répertoire 'rep'
    - Crée une copie du fichier source dans le répertoire spécifié.

  - `cp <src> <dest> rep/sousrep/etc` — Déplace un fichier dans les sous-répertoires
    - Crée une copie du fichier source dans les sous-répertoires spécifiés.

- `mv <src> <répertoire>` — Déplace/renomme un fichier
  - Déplace un fichier vers un nouvel emplacement ou le renomme.

  - `mv <src> ..` — Déplace un fichier dans le répertoire parent
    - Déplace un fichier vers le répertoire parent.

  - `mv <src> rep` — Déplace un fichier dans le répertoire 'rep'
    - Déplace un fichier vers le répertoire spécifié.

  - `mv <src> rep/sousrep/etc` — Déplace un fichier dans les sous-répertoires
    - Déplace un fichier vers les sous-répertoires spécifiés.

### 🔗 Gestion des liens

Symboliques :

- `lns <cible> <lien>` — Crée un lien symbolique
  - Crée un lien symbolique pointant vers un fichier cibble.

- `writes <lien> <cont>` — Écrit dans un lien symbolique
  - Écrit le contenu spécifié dans le fichier cible dépuis le lien symbolique.

- `reads <lien>` — Lit un lien symbolique
  - Affiche le contenu du fichier cible dépuis le lien symbolique.

- `rms <lien>` — Supprime un lien symbolique
  - Supprime le lien symbolique spécifié.

- `stats <lien>` — Affiche les métadonnées d'un lien symbolique
  - Fournit des détails comme les permissions, le propriétaire et d'autres attributs du lien symbolique spécifié.

- `lssymlinks <fic>` — Liste les liens symboliques pointant vers le fichier
  - Affiche tous les liens symboliques qui pointent vers le fichier spécifié.

- `mvs <lien> <rep>` — Déplace un lien symbolique
  - Déplace un lien symbolique vers un nouvel emplacement.

  - `mv <lien> ..` — Déplace un lien symbolique dans le répertoire parent
    - Déplace un lien symbolique vers le répertoire parent.

  - `mv <lien> rep` — Déplace un lien symbolique dans le répertoire 'rep'
    - Déplace un lien symbolique vers le répertoire spécifié.

  - `mv <lien> rep/sousrep/etc` — Déplace un lien symbolique dans les sous-répertoires
    - Déplace un lien symbolique vers les sous-répertoires spécifiés.

Matériels :

- `lnh <src> <dest>` — Crée un lien matériel
  - Crée un lien matériel pointant vers le fichier cible.

- `writeh <lien> <cont>` — Écrit dans un lien matériel
  - Écrit le contenu spécifié dans le fichier cible dépuis le lien matériel.

- `readh <lien>` — Lit un lien matériel
  - Affiche le contenu du fichier cible dépuis le lien matériel.

- `stath <lien>` — Affiche les métadonnées d'un lien matériel
  - Fournit des détails comme les permissions, le propriétaire et d'autres attributs du lien matériel spécifié.

- `lshardlinks <fic>` — Liste les liens matériels pointant vers le fichier
  - Affiche tous les liens matériels qui pointent vers le fichier spécifié.

- `mvh <lien> <rep>` — Déplace un lien matériel
  - Déplace un lien matériel vers un nouvel emplacement.

  - `mv <lien> ..` — Déplace un lien matériel dans le répertoire parent
    - Déplace un lien matériel vers le répertoire parent.

  - `mv <lien> rep` — Déplace un lien matériel dans le répertoire 'rep'
    - Déplace un lien matériel vers le répertoire spécifié.

  - `mv <lien> rep/sousrep/etc` — Déplace un lien matériel dans les sous-répertoires
    - Déplace un lien matériel vers les sous-répertoires spécifiés.

### 🛡️ Permissions

- `chmodf <fichier> <cible> <perm>` — Modifie permissions fichier
  - Modifie les permissions d'un fichier.

- `chmodd <rep> <cible> <perm>` — Modifie permissions répertoire
  - Modifie les permissions d'un répertoire.

>>> Cibles : `-Owner`, `-Group`, `-Others`  
>>> Perms : Combinaison de `rwx`, ex: `rw-`

### 👥 Gestion des groupes

- `lsgroups` — Liste les groupes de l'utilisateur
  - Liste tous les groupes auxquels l'utilisateur actuel appartient.

- `chgroup <nom>` — Change le groupe actuel
  - Modifie le groupe actif de l'utilisateur actuel.

- `curgroup` — Affiche le groupe actuel
  - Indique le groupe actif de l'utilisateur actuel.

- `crtgroup <nom>` — Crée un nouveau groupe
  - Crée un groupe avec le nom spécifié.

- `leavegroup <nom>` — Quitter un groupe
  - Permet à l'utilisateur de quitter le groupe spécifié.

- `lsmembers <nom>` — Liste les membres d'un groupe
  - Affiche les membres du groupe spécifié.

- `sudo delgroup <nom>` — Supprime un groupe (admin)
  - Supprime le groupe spécifié du système. Cette action nécessite des privilèges administratifs.

- `sudo add <nom> <pers>` — Ajoute un utilisateur au groupe (admin)
  - Ajoute un utilisateur au groupe spécifié. Cette action nécessite des privilèges administratifs.

- `sudo remove <pers> <nom>` — Retire un utilisateur du groupe (admin)
  - Retire un utilisateur du groupe spécifié. Cette action nécessite des privilèges administratifs.

### 🔐 Commandes administrateur (sudo)

- `sudo passwd` — Affiche le mot de passe (admin)
  - Affiche le mot de passe de l'utilisateur actuel.

- `sudo chgpasswd` — Change le mot de passe (admin)
  - Modifie le mot de passe de l'utilisateur actuel.

- `sudo trace` — Affiche la trace d'exécution (admin)
  - Affiche la trace des commandes exécutées par l'utilisateur actuel.

- `sudo deluser <nom>` — Supprime un compte utilisateur (admin)
  - Supprime le compte utilisateur spécifié du système.

- `sudo resetuser <nom>` — Réinitialise un répertoire utilisateur (admin)
  - Réinitialise le répertoire personnel de l'utilisateur spécifié.

- `sudo addadmin <nom>` — Ajoute un utilisateur en admin (superadmin)
  - Accorde des privilèges administratifs à l'utilisateur spécifié. Cette action nécessite des privilèges superadmin.

- `sudo deladmin <nom>` — Retire un utilisateur en admin (superadmin)
  - Retire les privilèges administratifs de l'utilisateur spécifié. Cette action nécessite des privilèges superadmin.

### 👤 Commandes utilisateur (sudo)

- `sudo infuser <nom>` — Affiche les informations de l'utilisateur (admin)
  - Affiche les informations de l'utilisateur spécifié.

### 🧱 Système

- `free` — Affiche les blocs libres
  - Affiche la quantité de blocs libres disponibles dans le système.

---

## 📌 Remarques

- Les commandes admin nécessitent le mot de passe sudo.
- Tous les chemins sont relatifs au dossier courant.
- Utilisez `help` à tout moment pour revoir ce manuel.
