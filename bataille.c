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

Player initPlayer(char *name, int nbShip) {
    /** Prend en paramètre le nom d'un joueur et son nombre de bateau.
    La fonction va initialiser un joueur et lui crée son plateau de jeu et son tableau de bateau à null. */

    Player p = malloc(sizeof(Player));
    p->name = name;
    p->nbShip = p->nbShip_alive = nbShip;
    p->grid = initGrid();
    p->tab_ship = NULL;
    return p;
}

Ship initShip(int l, enum orientedShip o) {
    /** Prend en paramètre la longueur et l'orientation d'un bateau.
    La fonction va initialiser un bateau ses valeurs par défaud et les valeurs en paramètre.. */

    Ship s = malloc(sizeof(Ship));
    s->tabCase = NULL;
    s->length = l;
    s->oriented = o;
    s->state = ALIVE;
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

    printf("----------------------------------------------\n");
    printf("|    |");
    for(int i = 0; i < SIZE_GRID; i++) {
        printf(" %c |", 'a' + i);
    } printf("\n----------------------------------------------\n");

    for(int i = 0; i < SIZE_GRID; i++) {
        
        if(i == 9) printf("| %d | ", i+1);
        else printf("| %d  | ", i+1);
        for(int j = 0; j < SIZE_GRID;j++) {
            if(p1->grid[i][j]->state == TOUCHED) {
                if(p1->grid[i][j]->type == SHIP) {
                    printf("#");
                } else {
                    printf("X");
                }
            } else {
                if(p1->grid[i][j]->type == SHIP) {
                    printf("O");
                } else {
                    printf(" ");
                }
            }
            printf(" | ");
        } printf("\n----------------------------------------------\n");

    }
}

void fillGrid(Player p) {
    /** Fonction qui prend en paramètre un joueur p,
    et va remplir aléatoirement sa grille de jeu de nbShip bateaux. */
    
    Ship s;
    int tab_ship[4] = {1, 2, 1, 1}, cpt = 0, val_x, val_y;
    srand(time(NULL));
    
    for (int i = 0; i < 4; i++) {
        while (tab_ship[i] != 0) {   
            val_x = rand() % 10;
            val_y = rand() % 10;
            s = initShip(i+2, (cpt % 2 ? VERTICAL : HORIZONTAL));
            if(s->oriented == VERTICAL && (val_x + s->length < 10)){
                if (addShip(p->grid, s, val_x, val_y)){
                    tab_ship[i]--;
                }
            }
            if(s->oriented == HORIZONTAL && (val_y + s->length < 10)){
                if (addShip(p->grid, s, val_x, val_y)){
                    tab_ship[i]--;
                }
            }
            cpt++;
            free(s);
        }    
    }
}

int shoot(Case **grid, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction va tirer dans la case indiqué et changé l'état du bateau.
    On retourne 0 si on tire dans l'eau, 1 si la case à déjà été touché, 2 si on touche un bateau. */

    Case c = grid[x][y];

    if(c->state == NOT_TOUCHED) {
        c->state = TOUCHED;
        if(c->type == SHIP) {
            return 2;
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}
int deadShip(Ship s){
    if(s->state == DESTROYED){
        return 1;
    } else
    {
        int i=0;
        while (i<s->length && s->tabCase[i]->state == TOUCHED) i++;
        if(i<s->length){
            s->state=DESTROYED;
            return 1;
        }
    }
    return 0;
    
}
int deadShips(Player p){
    int i = 0; int destroyed=0;
    while (i<p->nbShip) 
    {
        if (deadShip(p->tab_ship[i])==0) return 0;
    }
    return 1;
}

void startGame(Player p1) {
    /** Fonction qui prend en paramètre un joueur p1, et lance le jeu. */

    // on rempli la grille de jeu du joueur.
    fillGrid(p1);
}

/** Fonction main */
void main() {

    // On initialise les deux joueurs de la partie.
    Player p1 = initPlayer("Edward", 5);
    Player p2 = initPlayer("IA", 5);

    // On lance le jeu.
    startGame(p1);

    // On affiche la grille des deux joueurs.
    printGrid(p1, p2);
}
