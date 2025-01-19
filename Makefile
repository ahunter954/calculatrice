# Liste des fichiers sources
SRCS = Color_deletion.c Image_segmenter.c interface.c Reseau_neurones.c decoupage.c Noises.c Increase_c.c

# Générer une liste d'exécutables avec les mêmes noms que les sources (sans extension)
EXES = $(SRCS:.c=)

# Compilateur
CC = gcc

# Options de compilation
CFLAGS = -Wall -Wextra -Wformat-truncation -g

# Options de liaison pour SDL2 et SDL2_image
LDFLAGS = -lSDL2 -lSDL2_image -lm

# Règle par défaut : compiler tous les exécutables
all: $(EXES) train  final_solver

train: train.o preprocess.o neural_network.o
	$(CC) -o train train.o preprocess.o neural_network.o $(LDFLAGS)

final_solver: final_solver.o Solver.o liste_mot.o grid_constructor.o recognize.o preprocess.o neural_network.o
	$(CC) -o final_solver Solver.o final_solver.o liste_mot.o grid_constructor.o recognize.o preprocess.o neural_network.o $(LDFLAGS)

final_solver.o: final_solver.c Solver.h liste_mot.h grid_constructor.h recognize.h preprocess.h neural_network.h
	$(CC) $(CFLAGS) -c final_solver.c

liste_mot.o: liste_mot.c liste_mot.h recognize.h preprocess.h neural_network.h
	$(CC) $(CFLAGS) -c liste_mot.c

grid_constructor.o: grid_constructor.c grid_constructor.h recognize.h preprocess.h neural_network.h
	$(CC) $(CFLAGS) -c grid_constructor.c

Solver.o: Solver.c Solver.h
	$(CC) $(CFLAGS) -c Solver.c

train.o: train.c preprocess.h neural_network.h
	$(CC) $(CFLAGS) -c train.c

recognize.o: recognize.c preprocess.h neural_network.h
	$(CC) $(CFLAGS) -c recognize.c

preprocess.o: preprocess.c preprocess.h
	$(CC) $(CFLAGS) -c preprocess.c

neural_network.o: neural_network.c neural_network.h
	$(CC) $(CFLAGS) -c neural_network.c

# Règle pour compiler chaque fichier source en un exécutable distinct
%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Nettoyer les fichiers générés
clean:
	rm -f $(EXES) *.o train final_solver Grid.txt Mots.txt letter_recognition.net train.data 
	rm -f -r Grille/ Grid/ Mot/
