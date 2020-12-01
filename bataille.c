#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// On déclare quelques constantes...
#define SIZE_GRID 10

// On déclare les énumérations.
enum typeCase {
    /** On définit l'énumation du type d'une case.
    Celle-ci peut être soit un case contenant un bateau (SHIP), soit de l'eau (WATER). */

    SHIP, WATER
};

enum stateCase {
    /** On définit l'énumation de l'état d'une case.
    Celle-ci peut être soit un case qui a été touché (TOUCHED), soit non (NOT_TOUCHED). */

    TOUCHED, NOT_TOUCHED
};
enum orientedShip {
    /** On définit l'énumation de l'orientation du bauteau.
    Soit le bateau est position verticalement, soit horizontalement. */

    VERTICAL, HORIZONTAL
};

enum stateShip {
    /** On définit l'énumation de l'était du bauteau.
    Soit toutes les cases du bateau ont été touchés, dans ce cas il est considéré comme détruit(DESTROYED), 
    soit il reste des cases du bateau non touchées et il reste en vie (ALIVE). */

    DESTROYED, ALIVE
};

enum typeShip {
    /** On définit l'énumation de tous les type de bateau.
    CARRIER = 5 cases (porte-avion), CRUISER = 4 cases (croiseur), 
    DESTROYER = 3 cases (destroyer), SUBMARINE = 3 cases (sous-marin) et TORPEDO = 2 cases (torpilleur) */

    CARRIER, CRUISER, DESTROYER, SUBMARINE, TORPEDO
};

// On déclare les structures.
typedef struct _Case {
    /** On définit la structure d'une case composé des ses positions (x, y), 
    de son type de case (si il contient un bateau ou de l'eau) et de l'état de cette case
    (si elle a été touché ou non). */

    int x;
    int y;
    enum typeCase type;
    enum stateCase state;
} *Case;

typedef struct _Ship {
    /** On définit la structure d'un bateau composé d'un tableau de case (contenant les cases qui compose ce bateau), 
    de sa longueur, et de son orientation (verticale, ou horizontale). */

    Case *tabCase;
    int length;
    enum orientedShip oriented;
    enum stateShip state;
    enum typeShip type;
} *Ship;

typedef struct _Player {
    /** On définit la structure d'un joueur composé de son nom, 
    de son nombre de bateau, et ceux encore en vie, de son plateau de jeu,
    ainsi que d'un tableau contenant ses bateaux. */

    char *name;
    int nbShip;
    int nbShip_alive;
    Case **grid;
    Ship *tab_ship;
} *Player;

/** ----- Fonctions ----- */
Case **initGrid() {
    /** Fonction qui ne prend pas de paramètre et crée une matrice de Case,
    qui sera un plateau de jeu. Chaque case est par défaut une case d'eau qui n'a pas été touché. */

    Case **grid = malloc(sizeof(Case *) * SIZE_GRID);
    for(int i = 0; i < SIZE_GRID; i++) {
        grid[i] = malloc(sizeof(Case) * SIZE_GRID);
        for(int j = 0; j < SIZE_GRID;j++) {
            grid[i][j] = malloc(sizeof(Case));
            grid[i][j]->x = i;
            grid[i][j]->y = j;
            grid[i][j]->type = WATER;
            grid[i][j]->state = NOT_TOUCHED;
        }
    }
    return grid;
}

Player initPlayer(char *name) {
    /** Prend en paramètre le nom d'un joueur et son nombre de bateau.
    La fonction va initialiser un joueur et lui crée son plateau de jeu et son tableau de bateau à null. */

    Player p = malloc(sizeof(Player));
    p->name = name;
    p->nbShip = p->nbShip_alive = 0;
    p->grid = initGrid();
    p->tab_ship = NULL;
    return p;
}

Ship initShip(int l, enum orientedShip o, enum typeShip t) {
    /** Prend en paramètre la longueur et l'orientation d'un bateau.
    La fonction va initialiser un bateau ses valeurs par défaud et les valeurs en paramètre.. */

    Ship s = malloc(sizeof(Ship));
    s->tabCase = NULL;
    s->length = l;
    s->oriented = o;
    s->state = ALIVE;
    s->type = t;
    return s;
}

int addShip(Case **grid, Ship s, int x, int y) {
    /** Prend en paramètre une grille de jeu, un bateau et les positions (x, y) où le placer.
    La fonction va ajouter le bateau à la grille, si le bateau à bien été ajouté, on retourne 1, sinon 0. */    

    int val_x, val_y, n = 0, libre, stop = 0, nbCase = 0;
    Case *tab_tmp = malloc(sizeof(Case) * s->length), c;

    // on déclare le tableau de cases du bateau.
    s->tabCase = malloc(sizeof(Case) * s->length);

    // si les coordonnées indiqués ne sont pas dans la grille, on ne fait rien.
    if(x < 0 || x > SIZE_GRID || y < 0 || y > SIZE_GRID) {
        return 0;
    }

    // Pour chaque case du futur bateau..
    while(n < s->length && !stop) {

        // En fonction de l'orientation du bateau, on regarde quel case prendre.
        if(s->oriented == VERTICAL) {
            val_x = x+n, val_y = y;
        } else {
            val_x = x, val_y = y+n;
        }

        // On vérifie si la case à regardé est dans le tableau.
        if((val_x >= 0 && val_x < SIZE_GRID) && (val_y >= 0 && val_y < SIZE_GRID)) {
            // Si c'est le cas, on va regarder si les cases autour de la case à ajouté sont vides.
            for(int j = val_x-1;j < val_x + 2;j++) {
                if(j >= 0 && j < SIZE_GRID) {
                    for(int k = val_y-1;k < val_y + 2;k++) {
                        // Si la case n'est pas vide, on ne peut pas ajouter le tableau et on s'arrête.
                        if(k >= 0 && k < SIZE_GRID && !grid[j][k]->type == WATER) {
                            stop = 1;
                        }
                    }
                }
            }
            // Si la case et les autres autour sont libres, on l'ajoute au tableau.
            tab_tmp[nbCase++] = grid[val_x][val_y];
        // Si la case à regardé n'est pas dans le tableau, on s'arrête.
        } else {
            stop = 1;
        }
        // on incrémente n pour passé à la case suivante.
        n++;
    }
    // Si à fait tout la longueur du bateau, alors le bateau est bien ajouté.
    if(n == s->length && !stop) {
        for(int i = 0; i < nbCase;i++) {

            c = grid[tab_tmp[i]->x][tab_tmp[i]->y];
            c->type = SHIP;
            c->state = NOT_TOUCHED;
            s->tabCase[i] = c;
        }
    // Sinon, cela veut dire qu'on à rencontré une erreur, et on n'ajoute pas le bateau.
    } else {
    // on retourne 0 pour dire que le bateau n'à pas été ajouté.
        return 0;
    }

    // on libére l'espace mémoire stocké par le tableau de case temporaire.
    free(tab_tmp);

    // on retourne 1 pour dire que le bateau à bien été ajouté.
    return 1;

}

void printGrid(Player p1, Player p2) {
    /** Fonction qui prend en paramètre deux joueurs (p1, p2),
    et va afficher les grilles des deux joueurs l'une à côté de l'autre. */

    Case c;

    printf("\t\tGrille de %s \t\t\t\t\tGrille de l'%s\n", p1->name, p2->name);
    printf("----------------------------------------------\t\t");
    printf("----------------------------------------------\n");
    printf("|    |");
    for(int i = 0; i < SIZE_GRID * 2; i++) {
        if(i == SIZE_GRID) printf("\t\t|    |");
        printf(" %c |", 'A' + (i % SIZE_GRID));
    }
    printf("\n----------------------------------------------\t\t");
    printf("----------------------------------------------\n");

    for(int i = 0; i < SIZE_GRID; i++) {
        
        if(i == 9) printf("| %d | ", i+1);
        else printf("| %d  | ", i+1);
        for(int j = 0; j < SIZE_GRID * 2;j++) {

            if(j < SIZE_GRID) {
                c = p1->grid[i][j];
            } else {
                c = p2->grid[i][j%SIZE_GRID];
            }

            if(c->state == TOUCHED) {
                if(c->type == SHIP) {
                    printf("#");
                } else {
                    printf("X");
                }
            } else {
                if(c->type == SHIP) {
                    printf("O");
                } else {
                    printf(" ");
                }
            }
            if(j == SIZE_GRID -1) {
                printf(" |\t\t");
                if(i == 9) printf("| %d", i+1);
                else printf("| %d ", i+1);
            }
            printf(" | ");

        }
        printf("\n----------------------------------------------\t\t");
        printf("----------------------------------------------\n");
    }
}

void fillGrid(Case **g, enum typeShip *tabShip, int nbShips) {
    /** Fonction qui prend en paramètre la grille de jeu d'un joueur, le tableau des types de bateaux à ajoutés 
    ainsi que leurs nombres, et va remplir aléatoirement sa grille de jeu de nbShips bateaux. */
    
    Ship s;
    int i = 0, val_x, val_y, lenShip;
    
    while(i < nbShips) {

        if(tabShip[i] == CARRIER) {
            lenShip = 5;
        } else if(tabShip[i] == CRUISER) {
            lenShip = 4;
        } else if(tabShip[i] == SUBMARINE || tabShip[i] == DESTROYER) {
            lenShip = 3;
        } else {
            lenShip = 2;
        }

        val_x = rand() % 10;
        val_y = rand() % 10;
        s = initShip(lenShip, (i % 2 ? VERTICAL : HORIZONTAL), tabShip[i]);

        if(s->oriented == VERTICAL && (val_x + s->length < 10)){
            if(addShip(g, s, val_x, val_y)) {
                i++;
            }
        }
        if(s->oriented == HORIZONTAL && (val_y + s->length < 10)){
            if(addShip(g, s, val_x, val_y)) {
                i++;
            }
        }
    }
}

Case *standardShoot(Case **grid, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir sur une seule case où l'on va déclarer et retourne le tableau contenant cette case. */

    Case *caseCible = malloc(sizeof(Case));
    caseCible[0] = grid[x][y];
    return caseCible;
}

Case *squareShoot(Case **grid, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir sur une seule case où l'on va déclarer et retourne le tableau contenant cette case. */

    Case *caseCible = malloc(sizeof(Case) * 10);
    int nbCase = 0;
    for(int i = x-1; i < x + 2;i++) {
        if(i >= 0 && i < SIZE_GRID) {
            for(int j = y-1; j < y + 2;j++) {
                if(j >= 0 && j < SIZE_GRID) {
                    caseCible[nbCase++] = grid[i][j];
                }
            }
        }
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

Case *lineShootH(Case **grid, int x) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir sur une seule case où l'on va déclarer et retourne le tableau contenant cette case. */

    Case *caseCible = malloc(sizeof(Case) * SIZE_GRID);
    int nbCase = 0;
    for(int i = 0; i < SIZE_GRID;i++) {
        caseCible[nbCase++] = grid[x][i];
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

Case *lineShootV(Case **grid, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir sur une seule case où l'on va déclarer et retourne le tableau contenant cette case. */

    Case *caseCible = malloc(sizeof(Case) * SIZE_GRID);
    int nbCase = 0;
    for(int i = 0; i < SIZE_GRID;i++) {
        caseCible[nbCase++] = grid[i][y];
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

int shoot(Case *caseCible) {
    /** Fonction qui prend en paramètre un tableau de case ciblées,
     et va tirer dans chacunes des cases du tableau. */

    for(int i = 0; caseCible[i];i++) {
        caseCible[i]->state = TOUCHED;
    }
}

void initGame(Player *p1, Player *p2) {
    *p1 = initPlayer("Edward");
    *p2 = initPlayer("IA");
}

void startGame(Player p1, Player p2) {
    /** Fonction qui prend en paramètre un joueur p1, et lance le jeu. */

    srand(time(NULL));
    enum typeShip tabShip[5] = {CARRIER, CRUISER, DESTROYER, SUBMARINE, TORPEDO};
    // on rempli la grille de jeu du joueur.
    fillGrid(p1->grid, tabShip, 5);
    fillGrid(p2->grid, tabShip, 5);
}

void playGame(Player p1, Player p2) {

    // On demande à l'utilisateur les coordonées de la case qu'il veut tirer.
    int cord_x, cord_y, cord_valide = 0, play = 1;
    char cord[3];

    while(play) {

        printGrid(p1, p2);
        cord_valide = 0;

        while(!cord_valide) {

            printf("\nA quelle case voulez-vous tirer ? (exemple : B2) : ");
            scanf("%s", cord);


            char letter = cord[0];
            cord_x = atoi(cord+1);
            if(letter >= 'A' && letter <= 'J' && cord_x >= 1 && cord_x <= 10) {
                cord_y = letter - 'A';
                cord_x -= 1;
                cord_valide = 1;
            } else {
                printf("'%s' n'est pas une case valide.\n", cord);
            }
        }

        // Une fois que les coordonnées sont récupérées, on lui demande quel tire il choisit.        
        Case *tab = lineShootH(p1->grid, cord_x);
        shoot(tab);
    }
}

/** Fonction main */
void main() {

    // On déclare les deux joeuurs et on les initialises.
    Player p1, p2;
    initGame(&p1, &p2);

    startGame(p1, p2);
    playGame(p1, p2);
}
