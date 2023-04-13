#include "utils.h"

// Directions
enum { NORTH, SOUTH, WEST, EAST } Directions;

Cell directions[4];

void update_possible_enemy_positions(Map *map, Cell enemyPosition,
                                     char enemyDirection,
                                     Cell *possiblePositions,
                                     int *numPossiblePositions) {
  int i, j;
  int newNumPossiblePositions = 0;

  for (i = 0; i < *numPossiblePositions; i++) {
    Cell position = possiblePositions[i];

    // Vérifie si la position est adjacente à l'ennemi dans la direction
    // actuelle
    int dx = position.x - enemyPosition.x;
    int dy = position.y - enemyPosition.y;
    int isValid = 0;

    switch (enemyDirection) {
    case 'N':
      isValid = dx == 0 && dy < 0;
      break;
    case 'S':
      isValid = dx == 0 && dy > 0;
      break;
    case 'W':
      isValid = dx < 0 && dy == 0;
      break;
    case 'E':
      isValid = dx > 0 && dy == 0;
      break;
    }

    if (isValid) {
      // Met à jour la liste des positions possibles avec les cases adjacentes
      // validées
      if (map->water[position.y][position.x]) {
        possiblePositions[newNumPossiblePositions++] = position;
      }
    }
  }

  // Met à jour le nombre de positions possibles
  *numPossiblePositions = newNumPossiblePositions;
}

// Fonction pour initialiser la map
void init_map(Map *game_map) {
  // Les trois infos obligatoires de début de partie
  int width, height, my_id;
  scanf("%d%d%d", &width, &height, &my_id);
  fgetc(stdin);

  // Map
  for (int i = 0; i < height; i++) {
    char line[width + 1];
    scanf("%[^\n]", line);
    fgetc(stdin);

    for (int x = 0; x < width; x++) {
      game_map->island[x][i] = (line[x] == 'x');
      game_map->water[x][i] = (line[x] == '.');
      game_map->path[x][i] = 0;
      game_map->enemy[x][i] = (line[x] == '.');
    }
  }
}

// Fonction pour afficher la map (débug)
void print_map(charMap map) {
  for (int y = 0; y < MAP_SIZE; y++) {
    for (int x = 0; x < MAP_SIZE; x++) {
      fprintf(stderr, "%2hhd ", map[x][y]);
    }
    fprintf(stderr, "\n");
  }
}

// Fonction pour calculer la distance entre deux positions
int distance(Cell a, Cell b) { return abs(a.x - b.x) + abs(a.y - b.y); }

// Fonction pour vérifier si une position est sur la carte
char is_on_map(Cell p) {
  return (p.x >= 0 && p.x < MAP_SIZE && p.y >= 0 && p.y < MAP_SIZE);
}

// Fonction pour vérifier si une case peut être visitée
char can_visit(Cell p, Map map) {
  return (is_on_map(p) && !map.island[p.x][p.y] && !map.path[p.x][p.y]);
}

// Fonction pour calculer la meilleure case de départ
Cell best_starting_position(Map game_map) {
  charMap distance_iles = {0};
  int dist = 0;
  int nb_undefined;

  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      if (game_map.island[x][y]) {
        distance_iles[x][y] = 0;
      } else {
        distance_iles[x][y] = -1;
      }
    }
  }

  do {
    nb_undefined = 0;

    for (int x = 0; x < MAP_SIZE; x++) {
      for (int y = 0; y < MAP_SIZE; y++) {
        if (distance_iles[x][y] == -1)
          nb_undefined++;

        if (distance_iles[x][y] == dist) {
          for (int i = 0; i < 4; i++) {
            Cell d = directions[i];
            Cell pos = {x + d.x, y + d.y};

            if (is_on_map(pos) && distance_iles[pos.x][pos.y] == -1) {
              distance_iles[pos.x][pos.y] = dist + 1;
            }
          }
        }
      }
    }

    dist++;
  } while (nb_undefined != 0);

  print_map(distance_iles);
  fprintf(stderr, "_____________________\n");

  Cell best_pos = {0};
  char best_distance = 0;

  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      if (distance_iles[x][y] > best_distance) {
        best_pos = (Cell){x, y};
        best_distance = distance_iles[x][y];
      }
    }
  }

  return best_pos;
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main() {
  Map game_map = {0};
  init_map(&game_map);

  directions[NORTH] = (Cell){0, -1};
  directions[SOUTH] = (Cell){0, 1};
  directions[WEST] = (Cell){-1, 0};
  directions[EAST] = (Cell){1, 0};

  // Write an action using printf(). DON'T FORGET THE TRAILING \n
  // To debug: fprintf(stderr, "Debug messages...\n");

  Cell best_pos = best_starting_position(game_map);
  game_map.enemy[best_pos.x][best_pos.y] =
      0; // On considère que l'ennemi ne spawn pas au même endroit que nous
  print_map(game_map.enemy);
  printf("%d %d\n", best_pos.x, best_pos.y);

  // game loop
  while (1) {
    int x;
    int y;
    int my_life;
    int opp_life;
    int torpedo_cooldown;
    int sonar_cooldown;
    int silence_cooldown;
    int mine_cooldown;
    scanf("%d%d%d%d%d%d%d%d", &x, &y, &my_life, &opp_life, &torpedo_cooldown,
          &sonar_cooldown, &silence_cooldown, &mine_cooldown);
    char sonar_result[4];
    scanf("%s", sonar_result);
    fgetc(stdin);
    char opponent_orders[201];
    scanf("%[^\n]", opponent_orders);

    // fprintf(stderr, "%s", opponent_orders);

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    printf("MOVE N TORPEDO\n");
  }

  return 0;
}
