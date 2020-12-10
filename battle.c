#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "battle.h"

/** ----- Fonctions d'initialisation ----- */
Grid initGrid(int lenGrid) {
    /** Fonction qui ne prend pas de paramètre et crée une matrice de Case,
    qui sera un plateau de jeu. Chaque case est par défaut une case d'eau qui n'a pas été touché. */

    Grid g = malloc(sizeof(Grid));
    g->cases = malloc(sizeof(Case *) * lenGrid);
    for(int i = 0; i < lenGrid; i++) {
        g->cases[i] = malloc(sizeof(Case) * lenGrid);
        for(int j = 0; j < lenGrid;j++) {
            g->cases[i][j] = malloc(sizeof(Case));
            g->cases[i][j]->x = i;
            g->cases[i][j]->y = j;
            g->cases[i][j]->type = WATER;
            g->cases[i][j]->state = NOT_TOUCHED;
        }
    }
    g->length = lenGrid;
    return g;
}

Player initPlayer(char *name, int lenGrid) {
    /** Prend en paramètre le nom d'un joueur et son nombre de bateau.
    La fonction va initialiser un joueur et lui crée son plateau de jeu et son tableau de bateau à null. */

    Player p = malloc(sizeof(Player));
    p->name = name;
    p->nbShip = p->nbShip_alive = 5;
    p->tab_ship = malloc(sizeof(Ship) * p->nbShip);
    p->grid = initGrid(lenGrid);

    // On initialise le nombre de tirs spéciaux à 1 chacun.
    p->shoot = malloc(sizeof(int)*4);
    for (int i = 0;i <4;i++) p->shoot[i] = 1;
    return p;
}

Ordi initOrdi(int lenGrid) {
    /** Fonction qui ne prend aucun paramètre et va, initialiser une IA.
    Une IA est un joueur qui contient en plus un état, un tableau de cases à ciblées,
    et une matrice contenant l'historique de ces cases déjà tirées. */

    Ordi o = malloc(sizeof(Ordi));
    char *n = "IA";
    o->ordi = initPlayer(n, lenGrid);
    o->state = RESEARCH;
    o->shootOriented = UNDEFINED;
    o->lastCase = NULL;
    o->history = malloc(sizeof(int *) * lenGrid);

    for(int i = 0; i < lenGrid; i++) {
        o->history[i] = malloc(sizeof(int *) * lenGrid);
        for(int j = 0; j < lenGrid;j++) {
            o->history[i][j] = -1;
        }
    }
    return o;
}

Ship initShip(int l, OrientationShip o, typeShip t) {
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

int addShip(Grid g, Ship s, int x, int y) {
    /** Prend en paramètre une grille de jeu, un bateau et les positions (x, y) où le placer.
    La fonction va ajouter le bateau à la grille, si le bateau à bien été ajouté, on retourne 1, sinon 0. */    

    int val_x, val_y, n = 0, stop = 0, nbCase = 0;
    Case *tab_tmp = malloc(sizeof(Case) * s->length), c;

    // on déclare le tableau de cases du bateau.
    s->tabCase = malloc(sizeof(Case) * s->length);

    // si les coordonnées indiqués ne sont pas dans la grille, on ne fait rien.
    if(x < 0 || x > g->length || y < 0 || y > g->length) {
        free(s->tabCase);
        free(tab_tmp);
        free(s);
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
        if((val_x >= 0 && val_x < g->length) && (val_y >= 0 && val_y < g->length)) {
            // Si c'est le cas, on va regarder si les cases autour de la case à ajouté sont vides.
            for(int j = val_x-1;j < val_x + 2;j++) {
                if(j >= 0 && j < g->length) {
                    for(int k = val_y-1;k < val_y + 2;k++) {
                        // Si L'une des cases en haut, en bas, à droite ou à gauche n'est pas de l'eau, on s'arrête.
                        if(k >= 0 && k < g->length && !g->cases[j][k]->type == WATER && (j == val_x || k == val_y)) {
                            stop = 1;
                        }
                    }
                }
            }
            // Si la case et les autres autour sont libres, on l'ajoute au tableau.
            tab_tmp[nbCase++] = g->cases[val_x][val_y];
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

            c = g->cases[tab_tmp[i]->x][tab_tmp[i]->y];
            c->type = SHIP;
            c->state = NOT_TOUCHED;
            s->tabCase[i] = c;
        }
    // Sinon, cela veut dire qu'on à rencontré une erreur, et on n'ajoute pas le bateau.
    } else {
    // on retourne 0 pour dire que le bateau n'à pas été ajouté.
    
        free(s->tabCase);
        free(s);
        free(tab_tmp);
        return 0;
    }

    // on libére l'espace mémoire stocké par le tableau de case temporaire.
    free(tab_tmp);

    // on retourne 1 pour dire que le bateau à bien été ajouté.
    return 1;
}

void fillGrid(Player p, typeShip *tabShip, int nbShips) {
    /** Fonction qui prend en paramètre la grille de jeu d'un joueur, le tableau des types de bateaux à ajoutés 
    ainsi que leurs nombres, et va remplir aléatoirement sa grille de jeu de nbShips bateaux. */
    
    Grid g = p->grid;
    Ship s;
    Ship * tab_tmp = malloc(sizeof(Ship)* nbShips);
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

        val_x = rand() % p->grid->length;
        val_y = rand() % p->grid->length;
        s = initShip(lenShip, (i % 2 ? VERTICAL : HORIZONTAL), tabShip[i]);

        if(s->oriented == VERTICAL && (val_x + s->length < p->grid->length)){
            if(addShip(g, s, val_x, val_y)) {
                tab_tmp[i++] = s;
            }
        }
        else if(s->oriented == HORIZONTAL && (val_y + s->length < p->grid->length)){
            if(addShip(g, s, val_x, val_y)) {
                tab_tmp[i++] = s;
            }
        } else free(s);
    }
    p->tab_ship = tab_tmp;
}

void printGrid(Player p1, Player p2) {
    /** Fonction qui prend en paramètre deux joueurs (p1, p2),
    et va afficher les grilles des deux joueurs l'une à côté de l'autre. */

    Case c;

    printf("\n\t\tGrille de %s \t\t\t\t\tGrille de l'%s\n", p1->name, p2->name);

    for(int i = 0; i < ((p1->grid->length * 4) + 5) * 2;i++) {
        printf("-");
        if(i == ((p1->grid->length * 4) + 5) - 1) printf("-\t\t");
    } printf("-\n|    |");
    for(int i = 0; i < p1->grid->length * 2; i++) {
        if(i == p1->grid->length) printf("\t\t|    |");
        printf(" %c |", 'A' + (i % p1->grid->length));
    } printf("\n");

    for(int i = 0; i < ((p1->grid->length * 4) + 5) * 2;i++) {
        printf("-");
        if(i == ((p1->grid->length * 4) + 5) - 1) printf("-\t\t");
    } printf("-\n");

    for(int i = 0; i < p1->grid->length; i++) {
        
        if((i+1) > 9) printf("| %d | ", i+1);
        else printf("| %d  | ", i+1);
        for(int j = 0; j < p1->grid->length * 2;j++) {

            if(j < p1->grid->length) {
                c = p1->grid->cases[i][j];
            } else {
                c = p2->grid->cases[i][j%p1->grid->length];
            }

            if(c->state == TOUCHED) {
                if(c->type == SHIP) {
                    printf("#");
                } else {
                    printf("X");
                }
            } else {
                if(c->type == SHIP && j < p1->grid->length) {
                    printf("O");
                } else {
                    printf(" ");
                }
            }
            if(j == p1->grid->length - 1) {
                printf(" |\t\t");
                if((i+1) > 9) printf("| %d", i+1);
                else printf("| %d ", i+1);
            }
            printf(" | ");

        }
        printf("\n");
        for(int i = 0; i < ((p1->grid->length * 4) + 5) * 2;i++) {
            printf("-");
            if(i == ((p1->grid->length * 4) + 5) - 1) printf("-\t\t");
        } printf("-\n");
    }
}
int isDestroyed(Ship s) {
    /** Fonction qui prend en paramètre un bateau, 
    et va regarder si ce bateau est détruit. Si toutes ces cases ont été touchées, on actualise son statut,
    sinon on retourne faux. */

    if(s->state == DESTROYED){
        return 1;
    } else {
        int i = 0;
        while(i < s->length && s->tabCase[i]->state == TOUCHED) i++;
        if(i == s->length){
            s->state = DESTROYED;
            return 2;
        }
    }
    return 0;
}

int isAlive(Player p, typeShip t) {
    /** Fonction qui prend en paramètre un joueur et un type de bateau.
    On va regarder dans sa liste de bateau, celui avec le type "t" est en vie. 1 si il l'est, 0 sinon. */
    
    for(int i = 0;i < p->nbShip;i++){
        if(p->tab_ship[i]->type == t && !isDestroyed(p->tab_ship[i])) return 1;
    }
    return 0;
}

int shipsDestroyed(Player p) {
    /** Fonction qui prend en paramètre un joueur p,
    et va regarder si tous ses bateaux sont détruits. Si c'est le cas, on retourne 1 si c'est vrai, 0 sinon. */

    int nbShipsAlive = 0, stat;

    for(int i = 0; i < p->nbShip; i++) {
        stat = isDestroyed(p->tab_ship[i]);
        if (stat == 2) {
            printf("Un bateau de %s vient d'être détruit !\n", p->name);
        } else if(stat == 0) {
            nbShipsAlive++;
        }
    }
    p->nbShip_alive = nbShipsAlive;
    if(!nbShipsAlive) return 1;
    return 0;
}

/** ----- Fonctions de tirs ----- */
Case *standardShoot(Grid g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir sur une seule case où l'on va déclarer et retourne le tableau contenant cette case. */

    Case *caseCible = malloc(sizeof(Case));
    caseCible[0] = g->cases[x][y];
    return caseCible;
}

Case *lineShootH(Grid g, int x) {
    /** Prend en paramètre une grille de jeu, et un ligne x de cette grille.
    La fonction concerne un tir sur l'ensemble des cases de la ligne horizontale (x) donnée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * g->length);
    int nbCase = 0;
    for(int i = 0; i < g->length;i++) {
        caseCible[nbCase++] = g->cases[x][i];
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

Case *lineShootV(Grid g, int y) {
    /** Prend en paramètre une grille de jeu, et un ligne y de cette grille.
    La fonction concerne un tir sur l'ensemble des cases de la ligne verticale (y) donnée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * g->length);
    int nbCase = 0;
    for(int i = 0; i < g->length;i++) {
        caseCible[nbCase++] = g->cases[i][y];
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

Case *crossShoot(Grid g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * 6);
    caseCible[0] = g->cases[x][y];
    int nbCase = 1;
    for(int i = x-1; i < x + 2;i++) {
        if(i >= 0 && i < g->length) {
            for(int j = y-1; j < y + 2;j++) {
                if(j >= 0 && j < g->length && i !=  x && j != y) {
                    caseCible[nbCase++] = g->cases[i][j];
                }
            }
        }
    }
    return caseCible;
}

Case *plusShoot(Grid g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * 6);
    caseCible[0] = g->cases[x][y];
    int nbCase = 1;
    for(int i = x-1; i < x + 2;i++) {
        if(i >= 0 && i < g->length) {
            for(int j = y-1; j < y + 2;j++) {
                if(j >= 0 && j < g->length && i == x || j == y) {
                    caseCible[nbCase++] = g->cases[i][j];
                }
            }
        }
    }
    return caseCible;
}

Case *squareShoot(Grid g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case *caseCible = malloc(sizeof(Case) * 10);
    int nbCase = 0;
    for(int i = x-1; i < x + 2; i++) {
        if(i >= 0 && i < g->length) {
            for(int j = y-1; j < y + 2;j++) {
                if(j >= 0 && j < g->length) {
                    caseCible[nbCase++] = g->cases[i][j];
                }
            }
        }
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    return caseCible;
}

int shoot(Case *caseCible) {
    /** Fonction qui prend en paramètre un tableau de case ciblées,
     et va tirer dans chacunes des cases du tableau. Si on a tiré dans un bateau on retourne 1, sinon 0. */

    int sCase = 0;
    for(int i = 0; caseCible[i];i++) {

        if(caseCible[i]->type == SHIP) sCase = 1;
        caseCible[i]->state = TOUCHED;
    }

    return sCase;
}

/** ----- Fonctions pour la partie ----- */
void initGame(Player *p1, Ordi *ord) {
    /** Fonction qui prend en paramètre deux joueurs, et les initialises.
    On retourne également le nombre de joueurs présent dans la partie (hormis l'ORDI). */

    srand(time(NULL));
    puts("-------- BATAILLE NAVALE --------");
    puts("Règles : La bataille navale oppose deux joueurs qui s'affrontent. Chacun a une flotte composée de 5 bateaux, qui sont, en général, les suivants :");
    puts("1 porte-avion (5 cases), 1 croiseur (4 cases), 1 destroyer (3 cases), 1 sous-marin (3 cases), 1 torpilleur (2 cases)");
    puts("Les bateaux ne doivent pas être collés entre eux.\n");

    char *name = malloc(sizeof(char) * 25);
    int sizeGrid = 0;

    while(sizeGrid < 8 || sizeGrid > 15) {
        printf("Indiquez la taille de la grille des joueurs (comprise entre 8 et 15) :\n> ");
        scanf("%d", &sizeGrid);
    }
    printf("Indiquez le nom du joueur n°1 :\n> ");
    scanf("%s", name);
    

    *p1 = initPlayer(name, sizeGrid);
    *ord = initOrdi(sizeGrid);
}

int *askCords() {
    /** Fonction qui ne prend aucun paramètre, et va demander des coordonnées tant qu'ils ne sont pas valide.
    Si les coordonnées sont valides, on les retournes dans un tableau [x, y]. */

    int *tabCords = malloc(sizeof(int) * 2);
    int cords_valid = 0, cord_x, cord_y, user_shoot; char cords[3];

    while(!cords_valid) {
        scanf("%s", cords);

        cord_x = atoi(cords+1);
        if(cords[0] >= 'A' && cords[0] <= 'J' && cord_x >= 1 && cord_x <= 10) {
            tabCords[0] = cord_x - 1;
            tabCords[1] = cords[0] - 'A';
            cords_valid = 1;
        } else {
            printf("'%s' n'est pas une case valide.\n", cords);
        }
    }

    return tabCords;
}

void placeShips(Player p, typeShip *tabShip, int nbShips) {
    /** Prend en paramètre un joueur, un tableau de bateau à assigner, et leurs nombres.
    La fonction va demander au joueur les coordonnées des bateau à placer. */

    int ships_placed = 0, *cords, ch_orientation = 0, lenShip, res = 0;
    Ship s; OrientationShip o;
    
    while(ships_placed < nbShips) {

        ch_orientation = 0;
        printf("Indiquez les coordonnées pour placer votre ");
        switch (tabShip[ships_placed]) {
            case CARRIER:
                printf("porte-avion de 5 cases :\n> ");
                lenShip = 5;
                break;
        
            case CRUISER:
                printf("croiseur de 4 cases :\n> ");
                lenShip = 4;
                break;
        
            case DESTROYER:
                printf("destroyer de 3 cases :\n> ");
                lenShip = 3;
                break;
            case SUBMARINE:
                printf("sous-marin de 3 cases :\n> ");
                lenShip = 3;
                break;
            case TORPEDO:
                printf("torpilleur de 2 cases :\n> ");
                lenShip = 2;
                break;
        }
        cords = askCords();

        do {
            puts("Orientation possibles :");
            puts("1 > HORIZONTALE");
            puts("2 > VERTICALE\n");

            printf("Indiquez l'orientation du bateau :\n> ");
            scanf("%d", &ch_orientation);
        } while(ch_orientation < 1 || ch_orientation > 2);

        if(ch_orientation == 1) o = HORIZONTAL;
        else o = VERTICAL;

        s = initShip(lenShip, o, tabShip[ships_placed]);
        res = addShip(p->grid, s, cords[0], cords[1]);
        if(res == 1) {
            ships_placed++;
            puts("Bateau placé avec succès !\n");
        } else puts("Vous ne pouvez pas placer le bateau ici !");
    }

}

void manageShoot(Player p, Player op, int *tabCords) {
    /** Prend en paramètre le joueur, son adversaire et les coordonnées de la case ciblée. 
    La fonction va demander au joueur le tir de son choix, et va effectuer ce tir si les conditions sont bien respectés. 
    (si le tir spécial n'a pas déjà été utilisé et que le bateau associé n'est pas détruit) */

    Case *tabCases = NULL;
    int user_shoot = 0, shoot_valid = 0, shoot_line_valid = 0;
    char line_shoot[1];

    while (!shoot_valid) {
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

        if(user_shoot == 1) {
            tabCases = standardShoot(op->grid, tabCords[0], tabCords[1]);

        } else if(user_shoot == 2 && p->shoot[0]) {
            if(isAlive(p, SUBMARINE)) {
                shoot_line_valid = 0;
                while(!shoot_line_valid) {
                    puts("Choix de sens possible :");
                    puts("L > En ligne");
                    puts("C > En colonne\n");

                    printf("Dans quel sens s'effectue le tir en ligne ? (exemple : C)\n> ");
                    scanf("%s", line_shoot);

                    if(line_shoot[0] == 'C') {
                        tabCases = lineShootV(op->grid, tabCords[1]);
                    } else if(line_shoot[0] == 'L') {
                        tabCases = lineShootH(op->grid, tabCords[0]);
                    }
                    shoot_line_valid = 1;
                }
                p->shoot[0] = 0;
            } else {
                printf("> Vous ne pouvez pas tirer car votre SOUS-MARIN a été détruit !\n");                    
            }

        } else if(user_shoot == 3 && p->shoot[1]) {
            if(isAlive(p, CRUISER)) {
                tabCases = crossShoot(op->grid, tabCords[0], tabCords[1]);
                p->shoot[1] = 0;
            } else {
                printf("> Vous ne pouvez pas tirer car votre CROISEUR a été détruit !\n");                    
            }

        } else if(user_shoot == 4 && p->shoot[2]) {
            if(isAlive(p, CRUISER)) {
                tabCases = plusShoot(op->grid, tabCords[0], tabCords[1]);
                p->shoot[2] = 0;
            } else {
                printf("> Vous ne pouvez pas tirer car votre CROISEUR a été détruit !\n");                    
            }

        } else if(user_shoot == 5 && p->shoot[3]) {
            if(isAlive(p, CARRIER)) {
                tabCases = squareShoot(op->grid, tabCords[0], tabCords[1]);
                p->shoot[3] = 0;
            } else {
                printf("> Vous ne pouvez pas tirer car votre PORTE-AVION a été détruit !\n");                    
            }
        } else {
            puts("> Vous avez déjà effectué ce tir !\n");
        }

        // Si le tableau est bien rempli, on s'arrête.
        if(tabCases != NULL) shoot_valid = 1; 
    }

    // On tir dans chaque cases du tableau de cases ciblées.
    shoot(tabCases);

    puts("-------------------------------------------");
    printf("Résumé de la partie précédente : \n> %s a effectué un tir en %c%d.\n", p->name, 'A' + tabCases[0]->y, tabCases[0]->x+1);
    free(tabCords);
    free(tabCases);
}

void startGame(Player p1, Player p2) {
    /** Fonction qui prend en paramètre un joueur p1, et lance le jeu. */
    
    // On déclare quelques variables...
    typeShip tabShip[5] = {CARRIER, CRUISER, DESTROYER, SUBMARINE, TORPEDO};
    int ch = 0;

    // On demande comment le joueur souhaite placer ses navires.
    while(ch < 1 || ch > 2) {
        puts("\nComment voulez-vous placer vos bateaux ?");
        printf("1 > Manuellement\n2 > Aléatoirement\n> ");
        scanf("%d", &ch);
    }
    // une fois le choix valide.
    // Si c'est 1, il les ajoute manuellement.
    if(ch == 1) {
        printGrid(p1, p2);
        placeShips(p1, tabShip, 5);
    // Sinon il sont ajoutés de manière aléatoire.
    } else {
        fillGrid(p1, tabShip, 5);
    }
    // on rempli la grille de jeu de l'ORDI.
    fillGrid(p2, tabShip, 5);
}

void roundPlayer(Player p1, Player p2) {
    /** Fonction qui prend en paramètre deux joueurs et effectue le tour du joueur p1 contre le joueur p2,
    On demande les coordonnées à tirer, et le choix du tir. */

    
    printf("C'est au tour de %s !\n", p1->name);

    int *tabCords;
    
    printf("\nA quelle case voulez-vous tirer ? (exemple : B2)\n> ");
    tabCords = askCords();
        
    // on demande puis effectue le tir aux coordonnées indiquées.
    manageShoot(p1, p2, tabCords);
}

void roundOrdi(Ordi ord, Player p1) {
    /** Prend en paramètre un ordi de type IA et un joueur.
    La fonction va gérer le tour d'un ordi et effectuer un tir selon son état. */
    
    // on déclare quelques variables...
    int val_x, val_y;
    int valShoot, found = 0, chx, i;
    Case *tabCases; OrientationShoot tmp;

    // si l'IA est en état de recherche d'un bateau.
    if(ord->state == RESEARCH) {

        // si on on déjà effectué un tir.
        if(ord->lastCase != NULL) {

            found = 0;chx = 0;
            // on cherche une case autour de la précédente à une distance de : case + 2.
            while(!found && chx < 4) {
                // on sauvegarde les coordonées de tir,
                // si il n'a pas déjà tirer à la case y+2 vers le bas.
                if(ord->lastCase->y+2 < ord->ordi->grid->length && ord->history[ord->lastCase->x][ord->lastCase->y+2] == -1) {
                    val_x = ord->lastCase->x;
                    val_y = ord->lastCase->y+2;
                    found = 1;
                // si il n'a pas déjà tirer à la case y+2 vers le haut.
                } else if(ord->lastCase->y-2 >= ord->ordi->grid->length && ord->history[ord->lastCase->x][ord->lastCase->y-2] == -1) {
                    val_x = ord->lastCase->x;
                    val_y = ord->lastCase->y-2;
                    found = 1;
                // si il n'a pas déjà tirer à la case x+2 vers la droite.
                } else if(ord->lastCase->x+2 < ord->ordi->grid->length && ord->history[ord->lastCase->x][ord->lastCase->x+2] == -1) {
                    val_x = ord->lastCase->x+2;
                    val_y = ord->lastCase->y;
                    found = 1;
                // si il n'a pas déjà tirer à la case x-2 vers la gauche.
                } else if(ord->lastCase->x-2 >= ord->ordi->grid->length && ord->history[ord->lastCase->x][ord->lastCase->x-2] == -1) {
                    val_x = ord->lastCase->x-2;
                    val_y = ord->lastCase->y;
                    found = 1;
                } chx++;
            }
        }

        // si il n'a pas trouvé de case où tirer ou si c'est sa première action durant la partie.
        // on effectue un tir à une coordonées aléatoire.
        if(!found || ord->lastCase == NULL) {
            val_x = rand() % ord->ordi->grid->length;
            val_y = rand() % ord->ordi->grid->length;
        }

    } else if(ord->state == ORIENTATION) {

        val_x = ord->lastCase->x;
        val_y = ord->lastCase->y;

        tmp = UNDEFINED;

        if(val_x-1 >= 0 && ord->history[val_x-1][val_y] == -1) {
            val_x -= 1;
            tmp = TOP;
        } else if(val_x+1 < ord->ordi->grid->length && ord->history[val_x+1][val_y] == -1) {
            val_x += 1;
            tmp = BOTTOM;
        } else if(val_y-1 >= 0 && ord->history[val_x][val_y-1] == -1) {
            val_y -= 1;
            tmp = LEFT;
        } else if(val_y+1 < ord->ordi->grid->length && ord->history[val_x][val_y+1] == -1) {
            val_y += 1;
            tmp = RIGHT;
        } else {
            ord->state = RESEARCH;
            val_x = rand() % ord->ordi->grid->length;
            val_y = rand() % ord->ordi->grid->length;
        }
        
    } else if(ord->state == DESTRUCTION) {

        found = 0;
        // on prends en coordonées la case précédente.
        val_x = ord->lastCase->x;
        val_y = ord->lastCase->y;        

        // tant qu'on a pas trouvé une case non touchée on cherche.
        while(!found) {

            found = 0;
            // si on l'orientation est vers le haut.
            if(ord->shootOriented == TOP) {
                //si la case suivante n'a pas encore été touche, on la prends
                if(ord->history[val_x-1][val_y] = -1 && val_x - 1 >= 0) {
                    val_x = val_x-1;
                    found = 1;
                // si la case suivante à déjà été tirée et que c'est un bateau,
                // on la garde en mémoire
                } else if(ord->history[val_x-1][val_y] == 1 && val_x - 1 >= 0) {
                    val_x = val_x-1;
                // si c'est de l'eau, on va dans l'autre sens.
                } else {
                    ord->shootOriented = BOTTOM;
                }
            // si on l'orientation est vers le bas.
            } else if(ord->shootOriented == BOTTOM) {
                //si la case suivante n'a pas encore été touche, on la prends
                if(ord->history[val_x+1][val_y] = -1 && val_x + 1 < ord->ordi->grid->length) {
                    val_x = val_x+1;
                    found = 1;
                // si la case suivante à déjà été tirée et que c'est un bateau,
                // on la garde en mémoire
                } else if(ord->history[val_x+1][val_y] == 1 && val_x + 1 < ord->ordi->grid->length) {
                    val_x = val_x+1;
                } else {
                    // si c'est de l'eau, on va dans l'autre sens.
                    ord->shootOriented = TOP;
                }
            // si on l'orientation est vers la gauche.
            } else if(ord->shootOriented == LEFT) {
                //si la case suivante n'a pas encore été touche, on la prends
                if(ord->history[val_x][val_y-1] = -1  && val_y - 1 >= 0) {
                    val_y = val_y-1;
                    found = 1;
                // si la case suivante à déjà été tirée et que c'est un bateau,
                // on la garde en mémoire
                } else if(ord->history[val_x][val_y-1] == 1  && val_y - 1 >= 0) {
                    val_y = val_y-1;
                } else {
                    // si c'est de l'eau, on va dans l'autre sens.
                    ord->shootOriented = RIGHT;
                }

            // si on l'orientation est vers la droite.
            } else if(ord->shootOriented == RIGHT) {
                //si la case suivante n'a pas encore été touche, on la prends
                if(ord->history[val_x][val_y+1] = -1 && val_y + 1 < ord->ordi->grid->length) {
                    val_y = val_y+1;
                    found = 1;
                // si la case suivante à déjà été tirée et que c'est un bateau,
                // on la garde en mémoire
                } else if(ord->history[val_x][val_y+1] == 1 && val_y + 1 < ord->ordi->grid->length) {
                    val_y = val_y+1;
                } else {
                    // si c'est de l'eau, on va dans l'autre sens.
                    ord->shootOriented = LEFT;
                }
            }
        }
    }

    tabCases = standardShoot(p1->grid, val_x, val_y);
    valShoot = shoot(tabCases);

    if(valShoot == 1) {
        ord->history[val_x][val_y] = 1;
        ord->lastCase = p1->grid->cases[val_x][val_y];
        if(ord->state == RESEARCH) {
        
            ord->state = ORIENTATION;

        } else if(ord->state == ORIENTATION) {
            ord->shootOriented = tmp;

            int nbS_1 = p1->nbShip_alive;
            shipsDestroyed(p1);
            int nbS_2 = p1->nbShip_alive;

            if(nbS_1 != nbS_2) {
                ord->state = RESEARCH;
            } else {
                ord->state = DESTRUCTION;
            }
        } else if(ord->state == DESTRUCTION) {

            int nbS_1 = p1->nbShip_alive;
            shipsDestroyed(p1);
            int nbS_2 = p1->nbShip_alive;

            if(nbS_1 != nbS_2) {
                ord->state = RESEARCH;
                ord->lastCase = NULL;
            }
        }
    } else {
        ord->history[val_x][val_y] = 0;
        if(ord->state == RESEARCH) {
            ord->lastCase = p1->grid->cases[val_x][val_y];
        } else if(ord->state == DESTRUCTION) {
            if(ord->shootOriented == TOP) ord->shootOriented = BOTTOM;
            else if(ord->shootOriented == BOTTOM) ord->shootOriented = TOP;
            else if(ord->shootOriented == LEFT) ord->shootOriented = RIGHT;
            else if(ord->shootOriented == RIGHT) ord->shootOriented = LEFT;
        }
    }

    printf("> %s a effectué un tir en %c%d.\n", ord->ordi->name, 'A' + val_y, val_x+1);
    free(tabCases);
}

void playGame(Player p1, Ordi ord) {
    /** Prend en paramètre deux joueurs p1 et p2.
    La fonction va s'occuper du déroulement de la partie entre les deux joueurs. */

    int end = 0;
    Player p = p1;
    Ordi ia = ord;

    // Le jeu continue tant que les deux joueurs ont encore au moins un bateau en vie.
    while(!end) {

        // En fonction du point de vu du joueur à qui c'est le tour...
        // on lui affiche la grille (de son point de vue)
        printGrid(p, ia->ordi);

        // On effectue le round du joueur 1.
        roundPlayer(p1, ord->ordi);

        // on vérifie si les bateaux adverses ne sont pas tous détruit.
        if(shipsDestroyed(ia->ordi)) {
            // Si c'est le cas, le jeu s'arrête, sinon on continue.
            puts("Partie terminé ! Vous avez détruit l'ensemble des navires ennemis.");
            end = 1;
        } else {
            // Sinon, on effectue le round du joueur 2 (soit l'IA).
            roundOrdi(ia, p);
            if(shipsDestroyed(p1)) {
                puts("Partie terminé ! L'ORDI à détruit l'ensemble de vos navires.");
                end = 1;
            }
        }
    }
}

void cleanPlayer(Player p1){  
    int i,j;  
    for ( i = 0; i < p1->grid->length; i++) { 
        for (j = 0; j < p1->grid->length; j++) { 
            free(p1->grid->cases[i][j]); 
        } 
        free(p1->grid->cases[i]); 
    } 
    free(p1->grid->cases); 
    for(i = 0; i < p1->nbShip;i++) {
        free(p1->tab_ship[i]->tabCase);
        free(p1->tab_ship[i]);
    }
    free(p1->shoot); 
    free(p1->tab_ship);  
    free(p1->grid);
    free(p1);
}  

void cleanIA(Ordi o){ 
    cleanPlayer(o->ordi); 
    int i; 
    for ( i = 0; i < 10; i++) { 
        free(o->history[i]); 
    } 
    free(o->history); 
    free(o); 
} 

/** Fonction main */
void battleShip() {
     // On déclare les deux joeuurs et on les initialises.
    Player p1; Ordi ord;
    initGame(&p1, &ord);

    // on demande des précisions au joueur avant de démarrer la partie.
    startGame(p1, ord->ordi);

    // on lance la partie
    playGame(p1, ord);

    // on vide la mémoire.
    free(p1->name);
    cleanPlayer(p1);
    cleanIA(ord);
}