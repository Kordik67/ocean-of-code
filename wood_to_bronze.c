#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define MAP_SIZE 15
#define max(x,y) (((x) >= (y)) ? (x) : (y))

typedef char charMap[MAP_SIZE][MAP_SIZE];

// Structure pour représenter une position sur la carte
typedef struct {
    int x;
    int y;
} Cell;

// Structure pour représenter la carte
typedef struct {
    charMap water; // 1 si c'est de l'eau, 0 sinon
    charMap island; // 1 si c'est une île, 0 sinon
    charMap path; // 1 si la case a été visitée, 0 sinon
    charMap enemy_path; // 1 si la case a été visitée, 0 sinon
    charMap enemy; // 1 si l'ennemi peut se trouver sur la case, 0 sinon
    charMap me; // 1 si je peux me trouver sur la case, 0 sinon
} Map;

// Directions
enum {
    NORTH,
    SOUTH,
    WEST,
    EAST
} Directions;

// Structure pour représenter un sous-marin
typedef struct {
    int hp;
    char actions[300][30]; // Chaque joueur a 300 tours max
    Cell torpedo[9]; // Cases touchées par la dernière torpille lancée
    Cell position; // Position actuelle
    int sector; // Secteur analysé
} Submarine;

// Variables globales
Cell directions[4];
char directions_char[4];
int orders = 0; // indice des actions
Map game_map = {0};
charMap distance_iles = {0};
charMap torpedo_range[MAP_SIZE][MAP_SIZE] = {0};
Cell enemy_rectangle[2]; // Rectangle dans lequel se trouve l'ennemi. [0] = coin haut gauche, [1] = coin bas droite
int turn = 1;
int silence = 1; // 1 si l'ennemi à fait silence, 0 sinon

// Fonction pour initialiser la map
void init_map() {
    // Les trois infos obligatoires de début de partie
    int width, height, my_id;
    scanf("%d%d%d", &width, &height, &my_id); fgetc(stdin);

    // Map
    for (int i = 0; i < height; i++) {
        char line[width + 1];
        scanf("%[^\n]", line); fgetc(stdin);

        for (int x = 0; x < width; x++) {
            game_map.island[x][i] = (line[x] == 'x');
            game_map.water[x][i] = (line[x] == '.');
            game_map.path[x][i] = 0;
            game_map.enemy_path[x][i] = 0;
            game_map.enemy[x][i] = (line[x] == '.');
            game_map.me[x][i] = (line[x] == '.');
        }
    }
}

// Fonction pour afficher la map (débug)
void print_map(charMap map) {
    for (int y = 0; y < MAP_SIZE; y++) {
        if (y % 5 == 0 && y != 0) {
            fprintf(stderr, "________________________________\n");
        }

        for (int x = 0; x < MAP_SIZE; x++) {
            if (x % 5 == 0 && x != 0) {
                fputc('|', stderr);
            }

            fprintf(stderr, "%2hhd", map[x][y]);
        }

        fprintf(stderr, "\n");
    }
}

// Fonction pour calculer la distance entre deux positions
int distance(Cell a, Cell b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

int distance2(Cell a, Cell b) {
    return max(abs(a.x - b.x), abs(a.y - b.y));
}

// Fonction pour vérifier si une position est sur la carte
char is_on_map(Cell p) {
    return (p.x >= 0 && p.x < MAP_SIZE && p.y >= 0 && p.y < MAP_SIZE);
}

// Fonction pour vérifier si une case peut être visitée
char can_visit(Cell p) {
    return (is_on_map(p) && !game_map.island[p.x][p.y] && !game_map.path[p.x][p.y]);
}

// Fonction pour calculer la meilleure case de départ
Cell best_starting_position() {
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
                if (distance_iles[x][y] == -1) nb_undefined++;

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

    //print_map(distance_iles);
    //fprintf(stderr, "_____________________\n");

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

void set_zero_everywhere(charMap *map) {
    fprintf(stderr, "Appelée\n");
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            (*map)[x][y] = 0;
        }
    }
}

int non_zero_values(charMap map) {
    int count = 0;

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            count += (map[x][y] != 0);
        }
    }

    return count;
}

void analyse_north_direction() {
    for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                //if (!map.enemy[x][y]) continue;

                Cell newPosition = {x, y-1};

                // Vérifier si la nouvelle position est sur la carte et n'est pas une île
                if (!is_on_map(newPosition) || game_map.island[newPosition.x][newPosition.y]) {
                    game_map.enemy[x][y] = 0;
                    continue;
                }

                // Vérifier si la nouvelle position est sur l'eau
                if (game_map.island[x][y] || !game_map.enemy[x][y]) {
                    game_map.enemy[newPosition.x][newPosition.y] = 0;
                } else if (game_map.water[newPosition.x][newPosition.y]) {
                    game_map.enemy[newPosition.x][newPosition.y] = 1;
                    game_map.enemy[x][y] = 0;
                }
            }

            //fprintf(stderr, "Y = %d : %d\n", y, non_zero_values(game_map.enemy));
            //print_map(map.enemy);
            //fprintf(stderr, "Y = %d\n", y);
        }
}

void analyse_south_direction() {
    for (int y = MAP_SIZE-1; y >= 0; y--) {
            for (int x = 0; x < MAP_SIZE; x++) {

                Cell newPosition = {x, y+1};

                // Vérifier si la nouvelle position est sur la carte et n'est pas une île
                if (!is_on_map(newPosition) || game_map.island[newPosition.x][newPosition.y]) {
                    game_map.enemy[x][y] = 0;
                    continue;
                }

                // Vérifier si la nouvelle position est sur l'eau
                if (game_map.island[x][y] || !game_map.enemy[x][y]) {
                    game_map.enemy[newPosition.x][newPosition.y] = 0;
                } else if (game_map.water[newPosition.x][newPosition.y]) {
                    game_map.enemy[newPosition.x][newPosition.y] = 1;
                    game_map.enemy[x][y] = 0;
                }
            }

            //fprintf(stderr, "Y = %d : %d\n", y, non_zero_values(game_map.enemy));
        }
}

void analyse_west_direction() {
    for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                //if (!map.enemy[x][y]) continue;

                Cell newPosition = {x-1, y};

                // Vérifier si la nouvelle position est sur la carte et n'est pas une île
                if (!is_on_map(newPosition) || game_map.island[newPosition.x][newPosition.y]) {
                    game_map.enemy[x][y] = 0;
                    continue;
                }

                // Vérifier si la nouvelle position est sur l'eau
                if (game_map.island[x][y] || !game_map.enemy[x][y]) {
                    game_map.enemy[newPosition.x][newPosition.y] = 0;
                } else if (game_map.water[newPosition.x][newPosition.y]) {
                    game_map.enemy[newPosition.x][newPosition.y] = 1;
                    game_map.enemy[x][y] = 0;
                }
            }

            //fprintf(stderr, "Y = %d : %d\n", y, non_zero_values(game_map.enemy));
            //print_map(map.enemy);
            //fprintf(stderr, "Y = %d\n", y);
        }
}

void analyse_east_direction() {
    for (int x = MAP_SIZE-1; x >= 0; x--) {
        for (int y = 0; y < MAP_SIZE; y++) {
            //if (!map.enemy[x][y]) continue;

            Cell newPosition = {x+1, y};

            // Vérifier si la nouvelle position est sur la carte et n'est pas une île
            if (!is_on_map(newPosition) || game_map.island[newPosition.x][newPosition.y]) {
                game_map.enemy[x][y] = 0;
                continue;
            }

            // Vérifier si la nouvelle position est sur l'eau
            if (game_map.island[x][y] || !game_map.enemy[x][y]) {
                game_map.enemy[newPosition.x][newPosition.y] = 0;
            } else if (game_map.water[newPosition.x][newPosition.y]) {
                game_map.enemy[newPosition.x][newPosition.y] = 1;
                game_map.enemy[x][y] = 0;
            }
        }

        //fprintf(stderr, "X = %d : %d\n", x, non_zero_values(game_map.enemy));
        //print_map(game_map.enemy);
        //fprintf(stderr, "Y = %d\n", y);
    }
}

void update_possible_enemy_positions(char *action, Submarine *enemy, int opp_life, char sonar_result[4], Submarine me, int my_life) {
    if (strcmp(sonar_result, "NA") != 0) {
        if (sonar_result[0] == 'N') { // NO
            int yy = ((me.sector - 1) / 3) * 5;
            int xx = ((me.sector - 1) % 3) * 5;

            for (int y = yy; y < yy+5; y++) {
                for (int x = xx; x < xx+5; x++) {
                    game_map.enemy[x][y] = 0;
                }
            }
        } else { // YES
            for (int y = 0; y < MAP_SIZE; y++) {
                for (int x = 0; x < MAP_SIZE; x++) {
                    if ((x / 5) + (y / 5) * 3 + 1 != me.sector) {
                        game_map.enemy[x][y] = 0;
                    }
                }
            }
        }
    }

    // VERIFIER AVEC SES ANCIENS MOVES SI IL A PU S'INFLIGER LUI-MEME DES DEGATS
    
    int x, y;
    // Si il a torpillé au tour précédent
    if (strstr(action, "TORPEDO ")) {
        int counter = 0;

        if (sscanf(action, "TORPEDO %d %d", &x, &y) == 2) {
            //fprintf(stderr, "Torpillé (%s) : (%d,%d)\n", action, x, y);
            // Les cases touchées vont de (x-1, y-1) à (x+1, y+1)
            for (int Y = y-1; Y <= y+1; Y++) {
                for (int X = x-1; X <= x+1; X++) {
                    enemy->torpedo[counter++] = (Cell){X, Y};
                }
            }

            for (int Y = 0; Y < MAP_SIZE; Y++) {
                for (int X = 0; X < MAP_SIZE; X++) {
                    if (game_map.water[X][Y]) {
                        if (!torpedo_range[x][y][X][Y]) {
                            game_map.enemy[X][Y] = 0;
                        }
                    }
                }
            }
        }

        // Si les hp actuels de l'ennemi sont inférieurs à ceux du tour précédent (si il a torpillé), il s'est donc tiré dessus
        if (enemy->hp != opp_life) {
            // Si il a perdu 1 pv, il se trouve dans une des huit cases autour
            if (enemy->hp - opp_life == 1) {
                fprintf(stderr, "S'est infligé 1 dmg : (%d,%d)\n", enemy->torpedo[4].x, enemy->torpedo[4].y);
                //set_zero_everywhere(game_map.enemy);

                //print_map(game_map.enemy);
                // On enlève les autres cases actuellement possibles qui ne sont pas dans les 8 cases
                for (int y = 0; y < MAP_SIZE; y++) {
                    for (int x = 0; x < MAP_SIZE; x++) {
                        Cell pos = {x, y};
                        if (distance2(pos, enemy->torpedo[4]) != 1) {
                            game_map.enemy[pos.x][pos.y] = 0;
                        }
                        /*if (game_map.enemy[pos.x][pos.y]) {
                            bool is_in_eight = false;
                            for (int k = 0; k < 9; k++) {
                                if (k != 4) {
                                    if (enemy->torpedo[k].x == pos.x && enemy->torpedo[k].y == pos.y) {
                                        is_in_eight = true;
                                        break;
                                    }
                                }
                            }
                            if (!is_in_eight) {
                                game_map.enemy[pos.x][pos.y] = 0;
                            }
                        }*/
                    }
                }

                //print_map(game_map.enemy);
            } else if (enemy->hp - opp_life == 2) { // Si il a perdu 2 pv, on connait sa position
                set_zero_everywhere(&game_map.enemy);
                Cell pos = enemy->torpedo[4];
                game_map.enemy[pos.x][pos.y] = 1;
            }
        }
    }

    // Si j'ai torpillé au tour précédent
    if (strstr(me.actions[orders-1], "TORPEDO ")) {
        int counter = 0;

        if (sscanf(me.actions[orders-1], "TORPEDO %d %d", &x, &y) == 2) {
            // Les cases touchées vont de (x-1, y-1) à (x+1, y+1)
            for (int Y = y-1; Y <= y+1; Y++) {
                for (int X = x-1; X <= x+1; X++) {
                    me.torpedo[counter++] = (Cell){X, Y};
                }
            }
        }

        // Si ses pv sont différents du tour précédent, on l'a touché
        if (enemy->hp != opp_life) {
            // Si il a perdu 2 pv, on l'a touché
            if (enemy->hp - opp_life == 2) {
                fprintf(stderr, "Touché !\n");
                set_zero_everywhere(&game_map.enemy);
                Cell pos = me.torpedo[4];
                game_map.enemy[pos.x][pos.y] = 1;
            } else if (enemy->hp - opp_life == 1) { // Il a perdu 1 pv, on l'a touché mais on a pas sa pos exacte
                fprintf(stderr, "Touché : 1 pv\n");
                for (int y = 0; y < MAP_SIZE; y++) {
                    for (int x = 0; x < MAP_SIZE; x++) {
                        if (game_map.enemy[x][y] && distance2(me.torpedo[4], (Cell){x,y}) != 1) {
                            game_map.enemy[x][y] = 0;
                        }
                    }
                }
            }
        } else { // Je ne l'ai pas touché, je met à jour les pos possibles
            for (int i = 0; i < 9; i++) {
                Cell pos = me.torpedo[i];
                
                game_map.enemy[pos.x][pos.y] = 0;
            }
        }
    }

    const char *separator = "|";
    char *strToken = strtok(action, separator);

    while (strToken != NULL) {
        // Si l'action est un mouvement, on met à jour le tableau des positions possibles
        if (strstr(strToken, "MOVE")) {
            if (strstr(strToken, "N")) {
                analyse_north_direction();
                fprintf(stderr, "Analyse Nord : %d\n", non_zero_values(game_map.enemy));
            } else if (strstr(strToken, "S")) {
                analyse_south_direction();
                fprintf(stderr, "Analyse Sud : %d\n", non_zero_values(game_map.enemy));
                //direction = SOUTH;
            } else if (strstr(strToken, "W")) {
                //direction = WEST;
                analyse_west_direction();
                fprintf(stderr, "Analyse Ouest : %d\n", non_zero_values(game_map.enemy));
            } else {
                //direction = EAST;
                analyse_east_direction();
                fprintf(stderr, "Analyse Est : %d\n", non_zero_values(game_map.enemy));
            }
        }

        // Si l'action est un silence
        if (strstr(strToken, "SILENCE")) {
            charMap enemy_pos_copy;
            memcpy(enemy_pos_copy, game_map.enemy, sizeof(charMap));
            char last_enemy_direction;
            sscanf(enemy->actions[orders-2], "MOVE %c", &last_enemy_direction);
            fprintf(stderr, "Dernier move : %c\n", last_enemy_direction);

            for (int y = 0; y < MAP_SIZE; y++) {
                for (int x = 0; x < MAP_SIZE; x++) {
                    if (enemy_pos_copy[x][y]) {
                        int max_dist = 4;
                        for (int i = 0; i < 4; i++) {
                            Cell dir = directions[i];
                            char dir_char = directions_char[i];

                            // Il ne peux pas revenir en arrière
                            if ((last_enemy_direction == 'N' && dir_char == 'S') || (last_enemy_direction == 'S' && dir_char == 'N') || (last_enemy_direction == 'E' && dir_char == 'W') || (last_enemy_direction == 'W' && dir_char == 'E')) {
                                continue;
                            }

                            for (int j = 1; j <= max_dist; j++) {
                                Cell c = {x + dir.x * j, y + dir.y * j};

                                if (!is_on_map(c) || game_map.island[c.x][c.y]) {
                                    break;
                                } else {
                                    game_map.enemy[c.x][c.y] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Si l'action est une surface
        if (strstr(strToken, "SURFACE")) {
            if (non_zero_values(game_map.enemy) > 1) {
                int sector;
                sscanf(strToken, "SURFACE %d", &sector);

                for (int y = 0; y < MAP_SIZE; y++) {
                    for (int x = 0; x < MAP_SIZE; x++) {
                        if ((x / 5) + (y / 5) * 3 + 1 != sector) {
                            game_map.enemy[x][y] = 0;
                        }
                    }
                }
            }
        }

        strToken = strtok(NULL, separator);
        //print_map(game_map.enemy);
    }

}

// On renvoie combien de positions possibles sont dans ce secteur
int sonar_sector(int sector) {
    int count = 0;
    int yy = ((sector - 1) / 3) * 5;
    int xx = ((sector - 1) % 3) * 5;

    for (int y = yy; y < yy+5; y++) {
        for (int x = xx; x < xx+5; x++) {
            if (game_map.enemy[x][y]) {
                count++;
            }
        }
    }

    return count;
}

// On utilise cette fonction uniquement quand on a trouvé la position de l'adversaire
void set_enemy_pos(Submarine *enemy) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (game_map.enemy[x][y]) {
                enemy->position = (Cell){x, y};
                return;
            }
        }
    }
}

// On utilise cette fonction uniquement quand on a trouvé la position de l'adversaire, pour connaitre dans quelle(s) direction(s) il peut bouger lorsqu'il utilise un SILENCE
void set_enemy_path(Submarine enemy) {
    Cell pos = enemy.position;
    // On parcourt la liste de ses actions à l'inverse, et on trace le chemin en fonction des mouvements
    // Actuellement, je ne prend en compte que les MOVE
    for (int i = 299; i >= 0; i--) {
        if (strlen(enemy.actions[i]) > 0) {
            char dir_char;
            sscanf(enemy.actions[i], "MOVE %c", &dir_char); // On récupère la direction

            switch (dir_char) { // Il faut inverser les positions, puisqu'on remonte à l'inverse
                case 'N':
                    game_map.enemy_path[pos.x][pos.y + 1] = 1;
                    pos.y++;
                    break;
                case 'S':
                    game_map.enemy_path[pos.x][pos.y - 1] = 1;
                    pos.y--;
                    break;
                case 'E':
                    game_map.enemy_path[pos.x - 1][pos.y] = 1;
                    pos.x--;
                    break;
                case 'W':
                    game_map.enemy_path[pos.x + 1][pos.y] = 1;
                    pos.x++;
                    break;
            }
        }
    }

    //print_map(game_map.enemy_path);
}

void move(Submarine *me, int torpedo_cooldown, int sonar_cooldown, int silence_cooldown, Submarine enemy) {
    Cell new_pos = {0};
    int nb_directions = 0;
    int possible_dir[4] = {0};
    int best_dir_index = -1, best_dir_score = -1000;
    int nb_enemy_pos = non_zero_values(game_map.enemy);

    if (nb_enemy_pos > 20) { // Si on ne connait pas la position exacte de l'ennemi
        // On se déplace sur la case contenant le plus gros score d'éloignement des îles
        for (int i = 0; i < 4; i++) {
            new_pos.x = me->position.x + directions[i].x;
            new_pos.y = me->position.y + directions[i].y;

            if (is_on_map(new_pos) && !game_map.path[new_pos.x][new_pos.y] && game_map.water[new_pos.x][new_pos.y]) {
                possible_dir[nb_directions++] = i;
                
                if (distance_iles[new_pos.x][new_pos.y] > best_dir_score) {
                    best_dir_score = distance_iles[new_pos.x][new_pos.y];
                    best_dir_index = i;
                }
            }
        }

        // Si le nombre de directions possibles est à 0, on fait surface
        if (nb_directions == 0) {
            printf("SURFACE | MSG %d\n", nb_enemy_pos);
            strcpy(me->actions[orders], "SURFACE\n");

            set_zero_everywhere(&game_map.path);
        } else {
            // On charge en premier un sonar. Ensuite, on ne charge que des torpilles
            // Si l'ennemi fait SILENCE, on chargera de nouveau un sonar

            if (silence && sonar_cooldown > 0) { // 4 charges
                printf("MOVE %c SONAR | MSG %d\n", directions_char[best_dir_index], nb_enemy_pos);
                sprintf(me->actions[orders], "MOVE %c SONAR\n", directions_char[best_dir_index]);
            } else if (sonar_cooldown == 0) {
                // On prend le secteur où l'ennemi a le plus de cases possibles d'être
                int sector, count = 0;
                for (int i = 1; i <= 9; i++) {
                    int nb_enemy_pos_in_sector = sonar_sector(i);
                    if (nb_enemy_pos_in_sector > count) {
                        count = nb_enemy_pos_in_sector;
                        sector = i;
                    }
                }

                printf("MOVE %c | SONAR %d | MSG %d\n", directions_char[best_dir_index], sector, nb_enemy_pos);
                me->sector = sector;
                sprintf(me->actions[orders], "MOVE %c | SONAR %d\n", directions_char[best_dir_index], sector);
            } else if (torpedo_cooldown > 0) {
                printf("MOVE %c TORPEDO | MSG %d\n", directions_char[best_dir_index], nb_enemy_pos);
                sprintf(me->actions[orders], "MOVE %c TORPEDO\n", directions_char[best_dir_index]);
            } else {
                printf("MOVE %c | MSG %d\n", directions_char[best_dir_index], nb_enemy_pos);
                sprintf(me->actions[orders], "MOVE %c\n", directions_char[best_dir_index]);
            }
        }
    } else if (nb_enemy_pos > 1) { // On commence à torpiller les positions proches sans se toucher
        for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                if (game_map.enemy[x][y] && torpedo_range[me->position.x][me->position.y][x][y] && distance2(me->position, (Cell){x,y}) >= 2 && distance2(me->position, (Cell){x,y}) <= 4 && torpedo_cooldown == 0) {
                    print_map(torpedo_range[me->position.x][me->position.y]);
                    printf("TORPEDO %d %d | MSG %d\n", x, y, nb_enemy_pos);
                    sprintf(me->actions[orders], "TORPEDO %d %d\n", x, y);
                    
                    // On enregistre l'endroit où a touché la torpille
                    int counter = 0;
                    for (int Y = y-1; Y <= y+1; Y++) {
                        for (int X = x-1; X <= x+1; X++) {
                            me->torpedo[counter++] = (Cell){X,Y};
                        }
                    }
                    return;
                }
            }
        }

        int score;
        for (int i = 0; i < 4; i++) {
            new_pos.x = me->position.x + directions[i].x;
            new_pos.y = me->position.y + directions[i].y;

            if (is_on_map(new_pos) && !game_map.path[new_pos.x][new_pos.y] && game_map.water[new_pos.x][new_pos.y]) {
                possible_dir[nb_directions++] = i;
                score = -distance(enemy_rectangle[0], new_pos) - distance(enemy_rectangle[1], new_pos);
                
                if (score > best_dir_score) {
                    best_dir_score = score;
                    best_dir_index = i;
                }
            }
        }

        if (nb_directions == 0) {
            printf("SURFACE\n");
            strcpy(me->actions[orders], "SURFACE\n");
            set_zero_everywhere(&game_map.path);
        } else {
            printf("MOVE %c TORPEDO | MSG %d\n", directions_char[best_dir_index], nb_enemy_pos);
            sprintf(me->actions[orders], "MOVE %c TORPEDO\n", directions_char[best_dir_index]);
        }
    } else {
        // On calcule le chemin le plus court entre nous et l'ennemi
        int dist = distance(me->position, enemy.position);

        //fprintf(stderr, "Data : %d - %d - %d - %d", torpedo_range[me->position.x][me->position.y][enemy.position.x][enemy.position.y] && enemy.hp <= 2 && torpedo_cooldown == 0, torpedo_range[me->position.x][me->position.y][enemy.position.x][enemy.position.y], enemy.hp, torpedo_cooldown);
        if ((torpedo_range[me->position.x][me->position.y][enemy.position.x][enemy.position.y] && torpedo_cooldown == 0) && (enemy.hp > 2 || (enemy.hp <= 2 && me->hp > 2))) { // On essaye de ne pas se tirer dessus
            printf("TORPEDO %d %d | MSG %d\n", enemy.position.x, enemy.position.y, nb_enemy_pos);
            fprintf(stderr, "Dist : %d\n", dist);
            sprintf(me->actions[orders], "TORPEDO %d %d\n", enemy.position.x, enemy.position.y);
        } else { // On se dirige vers l'ennemi
            int best_dir_index = -1, best_dir_score = -1000;
            for (int i = 0; i < 4; i++) {
                Cell new_pos = {me->position.x + directions[i].x, me->position.y + directions[i].y};

                if (is_on_map(new_pos) && game_map.path[new_pos.x][new_pos.y] == 0 && game_map.island[new_pos.x][new_pos.y] == 0 /* && distance2(new_pos, enemy.position) < dist*/) {
                    int score = /*distance_iles[new_pos.x][new_pos.y]*/ - distance2(new_pos, enemy_rectangle[0]) - distance2(new_pos, enemy_rectangle[1]);
                    fprintf(stderr, "Score : %c %d\n", directions_char[i], score);

                    if (score > best_dir_score) {
                        best_dir_score = score;
                        best_dir_index = i;
                    }
                }
            }

            if (best_dir_index == -1) { // Aucune direction n'est valide
                printf("SURFACE | MSG %d\n", nb_enemy_pos);
                fprintf(stderr, "Surface : %d\n", best_dir_index);
                strcpy(me->actions[orders], "SURFACE\n");
                set_zero_everywhere(&game_map.path);
            } else {
                printf("MOVE %c TORPEDO | MSG %d\n", directions_char[best_dir_index], nb_enemy_pos);
                sprintf(me->actions[orders], "MOVE %c TORPEDO\n", directions_char[best_dir_index]);
                me->position.x += directions[best_dir_index].x;
                me->position.y += directions[best_dir_index].y;
                game_map.path[me->position.x][me->position.y] = 1;
            }
        }
    }

    if (!sonar_cooldown && silence) {
        silence = 0;
    }
}

void torp_away(charMap *map, Cell current_pos, Cell hit_cell, int recursion, int *possible_cells_number) {
    if (recursion == 5) return;

    (*map)[current_pos.x][current_pos.y] = 1;
    (*possible_cells_number)++;

    Cell new_pos = {0};
    for (int i = 0; i < 4; i++) {
        new_pos.x = current_pos.x + directions[i].x;
        new_pos.y = current_pos.y + directions[i].y;

        if (is_on_map(new_pos) && !(*map)[new_pos.x][new_pos.y] && game_map.water[new_pos.x][new_pos.y] && distance(new_pos, hit_cell) > recursion) {
            torp_away(map, new_pos, hit_cell, recursion+1, possible_cells_number);
        }
    }
}

void update_enemy_rectangle() {
    enemy_rectangle[0] = (Cell){20, 20};
    enemy_rectangle[1] = (Cell){-1, -1};

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (game_map.enemy[x][y]) {
                if (x < enemy_rectangle[0].x) {
                    enemy_rectangle[0].x = x;
                }

                if (y < enemy_rectangle[0].y) {
                    enemy_rectangle[0].y = y;
                }

                if (x > enemy_rectangle[1].x) {
                    enemy_rectangle[1].x = x;
                }

                if (y > enemy_rectangle[1].y) {
                    enemy_rectangle[1].y = y;
                }
            }
        }
    }
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    init_map();

    Submarine me = {0}, enemy = {0};
    me.hp = enemy.hp = 6;

    directions[NORTH] = (Cell){0, -1};
    directions[SOUTH] = (Cell){0, 1};
    directions[WEST] = (Cell){-1, 0};
    directions[EAST] = (Cell){1, 0};

    directions_char[NORTH] = 'N';
    directions_char[SOUTH] = 'S';
    directions_char[WEST] = 'W';
    directions_char[EAST] = 'E';

    int nb_cells = 0;
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if (game_map.water[x][y]) {
                torp_away(&torpedo_range[x][y], (Cell){x,y}, (Cell){x,y}, 0, &nb_cells);    

                //fprintf(stderr, "Coordonnées : (%d,%d) -> %d\n", x, y, nb_cells);
                nb_cells = 0;
            }
        }
    }

    // Write an action using printf(). DON'T FORGET THE TRAILING \n
    // To debug: fprintf(stderr, "Debug messages...\n");

    Cell best_pos = best_starting_position();
    game_map.enemy[best_pos.x][best_pos.y] = 0; // On considère que l'ennemi ne spawn pas au même endroit que nous

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
        scanf("%d%d%d%d%d%d%d%d", &x, &y, &my_life, &opp_life, &torpedo_cooldown, &sonar_cooldown, &silence_cooldown, &mine_cooldown);
        char sonar_result[4];
        scanf("%s", sonar_result); fgetc(stdin);
        char opponent_orders[201];
        scanf("%[^\n]", opponent_orders);

        me.position.x = x;
        me.position.y = y;
        strcpy(enemy.actions[orders++], opponent_orders);

        update_possible_enemy_positions(opponent_orders, &enemy, opp_life, sonar_result, me, my_life);
        print_map(game_map.enemy);

        update_enemy_rectangle();
        //fprintf(stderr, "Ennemi rec : (%d,%d) - (%d,%d)\n", enemy_rectangle[0].x, enemy_rectangle[0].y, enemy_rectangle[1].x, enemy_rectangle[1].y);

        if (non_zero_values(game_map.enemy) == 1) {
            set_enemy_pos(&enemy);
            set_enemy_path(enemy);
        }

        //update_possible_enemy_positions(me.actions[orders], &me, my_life);
        //print_map(game_map.me);

        // On update après pour faire la différence de points de vie dans la fonction précédente
        enemy.hp = opp_life;
        me.hp = my_life;
        
        //fprintf(stderr, "%s", opponent_orders);

        // Write an action using printf(). DON'T FORGET THE TRAILING \n
        // To debug: fprintf(stderr, "Debug messages...\n");

        //printf("MOVE N TORPEDO\n");
        game_map.path[x][y] = 1;
        move(&me, torpedo_cooldown, sonar_cooldown, silence_cooldown, enemy);
    }

    return 0;
}
