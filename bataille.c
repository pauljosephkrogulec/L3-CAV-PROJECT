#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// On déclare quelques constantes...
#define SIZE_GRID 10

// On déclare les énumérations.
typedef enum {
    /** On définit l'énumation du type d'une case.
    Celle-ci peut être soit un case contenant un bateau (SHIP), soit de l'eau (WATER). */

    SHIP, WATER
} typeCase;

typedef enum {
    /** On définit l'énumation de l'état d'une case.
    Celle-ci peut être soit un case qui a été touché (TOUCHED), soit non (NOT_TOUCHED). */

    TOUCHED, NOT_TOUCHED
} stateCase;
typedef enum {
    /** On définit l'énumation de l'orientation du bauteau.
    Soit le bateau est position verticalement, soit horizontalement. */

    VERTICAL, HORIZONTAL
} orientedShip;

typedef enum {
    /** On définit l'énumation de l'était du bauteau.
    Soit toutes les cases du bateau ont été touchés, dans ce cas il est considéré comme détruit(DESTROYED), 
    soit il reste des cases du bateau non touchées et il reste en vie (ALIVE). */

    DESTROYED, ALIVE
} stateShip;

typedef enum {
    /** On définit l'énumation de tous les type de bateau.
    CARRIER = 5 cases (porte-avion), CRUISER = 4 cases (croiseur), 
    DESTROYER = 3 cases (destroyer), SUBMARINE = 3 cases (sous-marin) et TORPEDO = 2 cases (torpilleur) */

    CARRIER, CRUISER, DESTROYER, SUBMARINE, TORPEDO
} typeShip;

// On déclare les structures.
typedef struct _Case {
    /** On définit la structure d'une case composé des ses positions (x, y), 
    de son type de case (si il contient un bateau ou de l'eau) et de l'état de cette case
    (si elle a été touché ou non). */

    int x;
    int y;
    typeCase type;
    stateCase state;
} *Case;

typedef struct _Ship {
    /** On définit la structure d'un bateau composé d'un tableau de case (contenant les cases qui compose ce bateau), 
    de sa longueur, et de son orientation (verticale, ou horizontale). */

    Case *tabCase;
    int length;
    orientedShip oriented;
    stateShip state;
    typeShip type;
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

    // Le nombre de tirs spéciaux restants.
    int line;
    int cross;
    int plus;
    int square;
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
    p->nbShip = p->nbShip_alive = 5;
    p->tab_ship = malloc(sizeof(Ship) * p->nbShip);
    p->grid = initGrid();

    // On initialise le nombre de tirs spéciaux à 1 chacun.
    p->line = 1;
    p->cross = 1;
    p->plus = 1;
    p->square = 1;
    return p;
}

Ship initShip(int l, orientedShip o, typeShip t) {
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
                if(c->type == SHIP && j < SIZE_GRID) {
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

void fillGrid(Player p, typeShip *tabShip, int nbShips) {
    /** Fonction qui prend en paramètre la grille de jeu d'un joueur, le tableau des types de bateaux à ajoutés 
    ainsi que leurs nombres, et va remplir aléatoirement sa grille de jeu de nbShips bateaux. */
    
    Case ** g = p->grid;
    Ship s, *tmp = malloc(sizeof(Ship) * nbShips);
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
                tmp[i++] = s;
            }
        }
        if(s->oriented == HORIZONTAL && (val_y + s->length < 10)){
            if(addShip(g, s, val_x, val_y)) {
                
                tmp[i++] = s;
            }
        }
    }
    p->tab_ship = tmp;
}

int deadShip(Ship s){
    if(s->state == DESTROYED){
        return 0;
    } else
    {
        int i=0;
        while (i<s->length && s->tabCase[i]->state == TOUCHED) i++;
        if(i==s->length){
            s->state = DESTROYED;
            return 0;
        }
    }
    return 1;
    
}

int deadShips(Player p){
    int i = 0;
    while (i<p->nbShip) 
    {
        if (deadShip(p->tab_ship[i])) return 1;
        i++;
    }
    return 0;
}

Case *standardShoot(Case **grid, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir sur une seule case où l'on va déclarer et retourne le tableau contenant cette case. */

    Case *caseCible = malloc(sizeof(Case));
    caseCible[0] = grid[x][y];
    return caseCible;
}

Case *lineShootH(Case **grid, int x) {
    /** Prend en paramètre une grille de jeu, et un ligne x de cette grille.
    La fonction concerne un tir sur l'ensemble des cases de la ligne horizontale (x) donnée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * SIZE_GRID);
    int nbCase = 0;
    for(int i = 0; i < SIZE_GRID;i++) {
        caseCible[nbCase++] = grid[x][i];
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

Case *lineShootV(Case **grid, int y) {
    /** Prend en paramètre une grille de jeu, et un ligne y de cette grille.
    La fonction concerne un tir sur l'ensemble des cases de la ligne verticale (y) donnée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * SIZE_GRID);
    int nbCase = 0;
    for(int i = 0; i < SIZE_GRID;i++) {
        caseCible[nbCase++] = grid[i][y];
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

Case *crossShoot(Case **grid, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * 6);
    caseCible[0] = grid[x][y];
    int nbCase = 1;
    for(int i = x-1; i < x + 2;i++) {
        if(i >= 0 && i < SIZE_GRID) {
            for(int j = y-1; j < y + 2;j++) {
                if(j >= 0 && j < SIZE_GRID && i !=  x && j != y) {
                    caseCible[nbCase++] = grid[i][j];
                }
            }
        }
    }
    return caseCible;
}

Case *plusShoot(Case **grid, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * 6);
    caseCible[0] = grid[x][y];
    int nbCase = 1;
    for(int i = x-1; i < x + 2;i++) {
        if(i >= 0 && i < SIZE_GRID) {
            for(int j = y-1; j < y + 2;j++) {
                if(j >= 0 && j < SIZE_GRID && i == x || j == y) {
                    caseCible[nbCase++] = grid[i][j];
                }
            }
        }
    }
    return caseCible;
}

Case *squareShoot(Case **grid, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * 10);
    int nbCase = 0;
    for(int i = x-1; i < x + 2; i++) {
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

int shoot(Case *caseCible) {
    /** Fonction qui prend en paramètre un tableau de case ciblées,
     et va tirer dans chacunes des cases du tableau. */

    for(int i = 0; caseCible[i];i++) {
        caseCible[i]->state = TOUCHED;
    }
}

void initGame(Player *p1, Player *p2) {

    puts("-------- BATAILLE NAVALE --------");
    puts("Règles : La bataille navale oppose deux joueurs qui s'affrontent. Chacun a une flotte composée de 5 bateaux, qui sont, en général, les suivants :");
    puts("1 porte-avion (5 cases), 1 croiseur (4 cases), 1 destroyer (3 cases), 1 sous-marin (3 cases), 1 torpilleur (2 cases)");
    puts("Les bateaux ne doivent pas être collés entre eux.\n");

    char *name = malloc(sizeof(char) * 25);

    printf("Indiquez le nom du joueur n°1 : ");
    scanf("%s", name);

    *p1 = initPlayer(name);
    *p2 = initPlayer("IA");
}

void startGame(Player p1, Player p2) {
    /** Fonction qui prend en paramètre un joueur p1, et lance le jeu. */

    srand(time(NULL));
    typeShip tabShip[5] = {CARRIER, CRUISER, DESTROYER, SUBMARINE, TORPEDO};

    // on rempli la grille de jeu du joueur.
    fillGrid(p1, tabShip, 5);
    fillGrid(p2, tabShip, 5);
    
}

int isAlive(Player p, typeShip t) {
    /** Fonction qui prend en paramètre un joueur et un type de bateau.
    On va regarder dans sa liste de bateau, celui avec le type "t" est en vie. 1 si il l'est, 0 sinon. */
    
    for(int i = 0;i < p->nbShip;i++){
        if(p->tab_ship[i]->type == t && deadShip(p->tab_ship[i])) return 1;
    }
    return 0;
}

int *askCords() {
    /** Fonction qui ne prend pas de paramètre,
    et va demander au joueur des coordonées de tirs et les vérifies avant de retourner ce tablaau de coordonées. */

    int *tabCords = malloc(sizeof(int) * 2);
    int cords_valid = 0, cord_x, cord_y; char letter, *cords;

    while(!cords_valid) {

        printf("\nA quelle case voulez-vous tirer ? (exemple : B2)\n> ");
        scanf("%s", cords);

        letter = cords[0];
        cord_x = atoi(cords+1);
        if(letter >= 'A' && letter <= 'J' && cord_x >= 1 && cord_x <= 10) {
            tabCords[0] = cord_x - 1;
            tabCords[1] = letter - 'A';
            cords_valid = 1;
        } else {
            printf("'%s' n'est pas une case valide.\n", cords);
        }
    }

    return tabCords;
}

void playGame(Player p1, Player p2) {

    // On demande à l'utilisateur les coordonées de la case qu'il veut tirer.
    int *tabCords, play = 1, user_shoot = 0, shoot_valid = 0, shoot_line_valid = 0;
    char line_shoot[1];
    Case *tabCases;

    while(play) {
        printGrid(p1, p2);
        tabCords = askCords();

        shoot_valid = 0;
        tabCases = NULL;

        // Une fois que les coordonnées sont récupérées, on lui demande quel tire il choisit.
        while (!shoot_valid) {
            tabCases = NULL;
            user_shoot = 0;
            while (user_shoot < 1 || user_shoot > 6) {
                puts("Liste des tirs possibles :\n1 > Tir normal (permet de viser une case)");
                puts("2 > Tir en ligne (permet de viser toute une ligne ou toute une colonne de la grille en une fois)");
                puts("3 > Tir en croix (permet de viser en une seule fois un ”x” centré sur une case et de 3 cases de circonférence)");
                puts("4 > Tir en plus (permet de viser en une seule fois un ”+” centré sur une case et de 3 cases de circonférence)");
                puts("5 > Tir en carée (permet de viser en une seule fois un carré de 3 cases par 3 centré sur une case)\n");
                
                printf("Quel type de tir voulez-vous utiliser ? (exemple : 1)\n> ");
                scanf("%d", &user_shoot);
            }

            switch (user_shoot) {
                case 1:
                    if (isAlive(p1, CRUISER)) tabCases = standardShoot(p2->grid, tabCords[0], tabCords[1]);
                    break;
                case 2:
                    shoot_line_valid = 0;
                    while(!shoot_line_valid) {
                        puts("Choix de sens possible :");
                        puts("L > En ligne");
                        puts("C > En colonne\n");

                        printf("Dans quel sens s'effectue le tir en ligne ? (exemple : C)\n> ");
                        scanf("%s", line_shoot);

                        if(line_shoot[0] == 'C' && isAlive(p1, SUBMARINE)) {
                            tabCases = lineShootV(p2->grid, tabCords[1]);
                        } else if(line_shoot[0] == 'L' && isAlive(p1, SUBMARINE)) {
                            tabCases = lineShootH(p2->grid, tabCords[1]);
                        }
                    }
                    p1->line = 0;
                    break;
                case 3:
                    printf("cross = %d", p1->cross);
                    if (isAlive(p1, CRUISER) && p1->cross == 1) {
                        tabCases = crossShoot(p2->grid, tabCords[0], tabCords[1]);
                        p1->cross = 0;
                    }
                    break;
                case 4:
                    if (isAlive(p1, CRUISER) && p1->plus == 1) {
                        tabCases = plusShoot(p2->grid, tabCords[0], tabCords[1]);
                        p1->plus = 0;
                    }
                    break;
                case 5:
                    if (isAlive(p1, CARRIER) && p1->square == 1) {
                        tabCases = squareShoot(p2->grid, tabCords[0], tabCords[1]);
                        p1->square = 0;
                    }
                    break;            
                default:
                    break;
            }
            if(tabCases != NULL) shoot_valid = 1;
        }
        // On tir dans chaque cases du tableau de cases ciblées.
        shoot(tabCases);
        play = deadShips(p2);
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
