#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAP_SIZE 15 // Taille de la grille (hauteur et largeur)

typedef char charMap[MAP_SIZE][MAP_SIZE]; // On défini le type charMap qui sera
                                          // un tableau 2D de char (ici 1 ou 0)

// Structure pour représenter une position de la carte
typedef struct {
  int x;
  int y;
} Cell;

// Structure pour représenter la carte
typedef struct {
  charMap water;  // 1 si c'est de l'eau, 0 sinon
  charMap island; // 1 si c'est une île, 0 sinon
  charMap path;   // 1 si la case a déjà été visitée, 0 sinon
  charMap enemy;  // 1 si l'ennemi peut se trouver sur la case, 0 sinon
} Map;
