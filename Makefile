# Nom de l'exécutable final
TARGET = unix_os

# Fichier source
SRC = unix_os_project.c

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -g

# Dossier d'installation utilisateur
INSTALL_DIR = ~/.local/bin

# Règle par défaut : compilation
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Exécution locale (dans le dossier courant)
run: $(TARGET)
	./$(TARGET)

# Nettoyage des fichiers compilés
clean:
	rm -f $(TARGET)

# Installation dans ~/.local/bin
install: $(TARGET)
	mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)/
	chmod +x $(INSTALL_DIR)/$(TARGET)
	@echo "✅ Installé dans $(INSTALL_DIR)"

# Suppression de l'exécutable installé
uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "🗑️ Supprimé de $(INSTALL_DIR)"

# Réinstallation (clean + compile + install)
reinstall: clean all install

# Débogage avec gdb
debug: $(TARGET)
	gdb ./$(TARGET)
