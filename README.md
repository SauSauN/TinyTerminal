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
  - `mkdir <nom> rep` — Crée un répertoire dans le répertoire 'rep'
  - `mkdir <nom> rep/sousrep/etc` — Crée un répertoire dans les sous-répertoires
- `cd <nom>` — Change de répertoire
  - `cd ..` — Remonte d'un niveau
  - `cd rep` — Va dans le répertoire 'rep'
  - `cd rep/sousrep/etc` — Chemin relatif
- `cpdir <src> <dest> [répertoire]` — Copie un répertoire
  - `cpdir <src> <dest>` — Copie un répertoire dans le répertoire actuel
  - `cpdir <src> <dest> ..` — Copie un répertoire dans le répertoire parent
  - `cpdir <src> <dest> rep` — Copie un répertoire dans le répertoire 'rep'
  - `cpdir <src> <dest> rep/sousrep/etc` — Copie un répertoire dans les sous-répertoires
- `mvdir <src> <répertoire>` — Déplace un répertoire
  - `mvdir <src> ..` — Déplace un répertoire dans le répertoire parent
  - `mvdir <src> rep` — Déplace un répertoire dans le répertoire 'rep'
  - `mvdir <src> rep/sousrep/etc` — Déplace un répertoire dans les sous-répertoires

### 📄 Gestion des fichiers

- `touch <nom>` — Crée un fichier vide
- `statf <nom>` — Affiche les métadonnées d'un fichier
- `write <nom> <cont>` — Écrit dans un fichier
- `cat <nom>` — Affiche le contenu d'un fichier
- `rm <nom>` — Supprime un fichier
- `cp <src> <dest> [répertoire]` — Copie un fichier
  - `cp <src> <dest>` — Copie un fichier dans un répertoire actuel
  - `cp <src> <dest> ..` — Copie un fichier dans un répertoire parent
  - `cp <src> <dest> rep` — Copie un fichier dans un répertoire 'rep'
  - `cp <src> <dest> rep/sousrep/etc` — Déplace un fichier dans les sous-répertoires
- `mv <src> <répertoire>` — Déplace/renomme un fichier
  - `mv <src> ..` — Déplace un fichier dans le répertoire parent
  - `mv <src> rep` — Déplace un fichier dans le répertoire 'rep'
  - `mv <src> rep/sousrep/etc` — Déplace un fichier dans les sous-répertoires

### 🔗 Gestion des liens

Symboliques :

- `lns <cible> <lien>` — Crée un lien symbolique
- `writes <lien> <cont>` — Écrit dans un lien symbolique
- `reads <lien>` — Lit un lien symbolique
- `rms <lien>` — Supprime un lien symbolique
- `stats <lien>` — Affiche les métadonnées d'un lien symbolique
- `lssymlinks <fic>` — Liste les liens symboliques pointant vers le fichier
- `mvs <lien> <rep>` — Déplace un lien symbolique
  - `mv <lien> ..` — Déplace un lien symbolique dans le répertoire parent
  - `mv <lien> rep` — Déplace un lien symbolique dans le répertoire 'rep'
  - `mv <lien> rep/sousrep/etc` — Déplace un lien symbolique dans les sous-répertoires

Matériels :

- `lnh <src> <dest>` — Crée un lien matériel
- `writeh <lien> <cont>` — Écrit dans un lien matériel
- `readh <lien>` — Lit un lien matériel
- `stath <lien>` — Affiche les métadonnées d'un lien matériel
- `lshardlinks <fic>` — Liste les liens matériels pointant vers le fichier
- `mvh <lien> <rep>` — Déplace un lien matériel
  - `mv <lien> ..` — Déplace un lien matériel dans le répertoire parent
  - `mv <lien> rep` — Déplace un lien matériel dans le répertoire 'rep'
  - `mv <lien> rep/sousrep/etc` — Déplace un lien matériel dans les sous-répertoires

### 🛡️ Permissions

- `chmodf <fichier> <cible> <perm>` — Modifie permissions fichier
- `chmodd <rep> <cible> <perm>` — Modifie permissions répertoire

> Cibles : `-Owner`, `-Group`, `-Others`  
> Perms : Combinaison de `rwx`, ex: `rw-`

### 👥 Gestion des groupes

- `lsgroups` — Liste les groupes de l'utilisateur
- `chgroup <nom>` — Change le groupe actuel
- `curgroup` — Affiche le groupe actuel
- `crtgroup <nom>` — Crée un nouveau groupe
- `leavegroup <nom>` — Quitter un groupe
- `lsmembers <nom>` — Liste les membres d'un groupe
- `sudo delgroup <nom>` — Supprime un groupe (admin)
- `sudo add <nom> <pers>` — Ajoute un utilisateur au groupe (admin)
- `sudo remove <pers> <nom>` — Retire un utilisateur du groupe (admin)

### 🔐 Commandes administrateur (sudo)

- `sudo passwd` — Affiche le mot de passe (admin)
- `sudo chgpasswd` — Change le mot de passe (admin)
- `sudo trace` — Affiche la trace d'exécution (admin)
- `sudo deluser <nom>` — Supprime un compte utilisateur (admin)
- `sudo resetuser <nom>` — Réinitialise un répertoire utilisateur (admin)

- `sudo addadmin <nom>` — Ajoute un utilisateur en admin (superadmin)
- `sudo deladmin <nom>` — Retire un utilisateur en admin (superadmin)

### 👤 Commandes utilisateur (sudo)

- `sudo infuser <nom>` — Affiche les informations de l'utilisateur (admin)

### 🧱 Système

- `free` — Affiche les blocs libres

---

## 📌 Remarques

- Les commandes admin nécessitent le mot de passe sudo.
- Tous les chemins sont relatifs au dossier courant.
- Utilisez `help` à tout moment pour revoir ce manuel.
