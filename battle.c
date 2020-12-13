#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "battle.h"

/** ----- Fonctions d'initialisation ----- */
Grid *initGrid(int lenGrid) {
    /** Fonction qui ne prend pas de paramètre et crée une matrice de Case,
    qui sera un plateau de jeu. Chaque case est par défaut une case d'eau qui n'a pas été touché. */

    Grid *g = malloc(sizeof(Grid));
    g->cases =  malloc(sizeof(Case *) * lenGrid);
    for(int i = 0; i < lenGrid; i++) {
        g->cases[i] = malloc(sizeof(Case) * lenGrid);
        for(int j = 0; j < lenGrid;j++) {
            g->cases[i][j] = malloc(sizeof(Case *));
            g->cases[i][j]->x = i;
            g->cases[i][j]->y = j;
            g->cases[i][j]->type = WATER;
            g->cases[i][j]->state = NOT_TOUCHED;
        }
    }
    g->length = lenGrid;
    return g;
}

Player *initPlayer(char *name, int lenGrid) {
    /** Prend en paramètre le nom d'un joueur et son nombre de bateau.
    La fonction va initialiser un joueur et lui crée son plateau de jeu et son tableau de bateau à null. */

    Player *p = malloc(sizeof(Player));
    p->name = name;
    p->nbShip = 5;
    p->nbShip_alive = 5;
    p->tab_ship = malloc(sizeof(Ship) * p->nbShip);
    p->grid = initGrid(lenGrid);

    // On initialise le nombre de tirs spéciaux à 1 chacun.
    p->shoot = (int *) malloc(sizeof(int) * 4);
    for (int i = 0;i <4;i++) p->shoot[i] = 1;
    return p;
}

Ordi *initOrdi(int lenGrid) {
    /** Fonction qui ne prend aucun paramètre et va, initialiser une IA.
    Une IA est un joueur qui contient en plus un état, un tableau de cases à ciblées,
    et une matrice contenant l'historique de ces cases déjà tirées. */

    Ordi *o = malloc(sizeof(Ordi));
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

Ship *initShip(int l, OrientationShip o, typeShip t) {
    /** Prend en paramètre la longueur et l'orientation d'un bateau.
    La fonction va initialiser un bateau ses valeurs par défaud et les valeurs en paramètre.. */

    Ship *s = malloc(sizeof(Ship));
    s->tabCase = NULL;
    s->length = l;
    s->oriented = o;
    s->state = ALIVE;
    s->type = t;
    return s;
}

int addShip(Grid *g, Ship *s, int x, int y) {
    /** Prend en paramètre une grille de jeu, un bateau et les positions (x, y) où le placer.
    La fonction va ajouter le bateau à la grille, si le bateau à bien été ajouté, on retourne 1, sinon 0. */    

    // quelques vairables pour la fonction...
    int val_x, val_y, n = 0, stop = 0, nbCase = 0;
    Case **tab_tmp = malloc(sizeof(Case) * s->length), *c;

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
        // on retourne 0 pour dire que le bateau n'à pas été ajouté,
        // et on libére l'espace mémoire utilisé.
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

void fillGrid(Player *p, typeShip *tabShip, int nbShips) {
    /** Fonction qui prend en paramètre la grille de jeu d'un joueur, le tableau des types de bateaux à ajoutés 
    ainsi que leurs nombres, et va remplir aléatoirement sa grille de jeu de nbShips bateaux. */
    
    // quelques vairables pour la fonction...
    Grid *g = p->grid;
    Ship *s;
    Ship **tab_tmp = p->tab_ship;
    int i = 0, val_x, val_y, lenShip;

    // tant que l'on n'a pas ajoutés tous les bateaux..
    while(i < nbShips) {

        // on détermine la longueur selon son type.
        // 5 si c'est un porte-avion.
        if(tabShip[i] == CARRIER) {
            lenShip = 5;
        // 4 si c'est un croiseur
        } else if(tabShip[i] == CRUISER) {
            lenShip = 4;
        // 3 si c'est un destroyer ou un sous-marin.
        } else if(tabShip[i] == SUBMARINE || tabShip[i] == DESTROYER) {
            lenShip = 3;
        // 2 si c'est un torpilleur.
        } else {
            lenShip = 2;
        }

        // on détermine aléatoirement une case de positionnement.
        val_x = rand() % p->grid->length;
        val_y = rand() % p->grid->length;

        // on initialise ce bateau avec toutes les infos.
        s = initShip(lenShip, (i % 2 ? VERTICAL : HORIZONTAL), tabShip[i]);

        // si l'orientation est vertical et que le bateau ne sort pas de la grille...
        if(s->oriented == VERTICAL && (val_x + s->length < p->grid->length)) {
            // on l'ajoute à la grille du joueur.
            if(addShip(g, s, val_x, val_y)) {
                // si c'est bien ajouté on l'add au tableau de bateau.
                tab_tmp[i++] = s;
            }
        }
        // si l'orientation est horizontale et que le bateau ne sort pas de la grille...
        else if(s->oriented == HORIZONTAL && (val_y + s->length < p->grid->length)) {
            // on l'ajoute à la grille du joueur.
            if(addShip(g, s, val_x, val_y)) {
                // si c'est bien ajouté on l'add au tableau de bateau.
                tab_tmp[i++] = s;
            }
        // sinon on libère l'espace mémoire du bateau qui ne peut être ajouté.
        } else free(s);
    }
    // on défini le tableau de bateau du joueur.
    p->tab_ship = tab_tmp;
}

void printGrid(Player *p1, Player *p2) {
    /** Fonction qui prend en paramètre deux joueurs (p1, p2),
    et va afficher les grilles des deux joueurs l'une à côté de l'autre. */

    Case *c;

    // on affiche les entêtes des grilles.
    for(int i = 0; i < ((p1->grid->length * 4) + 3);i++) {
        if(i == 0) printf("\nGrille de %s", p1->name);
        printf(" ");
        if(i == (p1->grid->length * 4) + 2) printf("Grille de l'%s\n",p2->name);
    }

    // on affiche la délimitation du haut des grilles.
    for(int i = 0; i < ((p1->grid->length * 4) + 5) * 2;i++) {
        printf("-");
        if(i == ((p1->grid->length * 4) + 5) - 1) printf("-\t\t");
    } printf("-\n|    |");

    // on affiche chaque lettre sur la ligne.
    for(int i = 0; i < p1->grid->length * 2; i++) {
        if(i == p1->grid->length) printf("\t\t|    |");
        printf(" %c |", 'A' + (i % p1->grid->length));
    } printf("\n");
    
    // on affiche la délimitation en dessous des lettres.
    for(int i = 0; i < ((p1->grid->length * 4) + 5) * 2;i++) {
        printf("-");
        if(i == ((p1->grid->length * 4) + 5) - 1) printf("-\t\t");
    } printf("-\n");

    // pour chaque grille en largeur.
    for(int i = 0; i < p1->grid->length; i++) {

        // on affiche le numéro.
        if((i+1) > 9) printf("| %d | ", i+1);
        else printf("| %d  | ", i+1);

        // pour toutes la ligne, on affiche la case et son statut.
        for(int j = 0; j < p1->grid->length * 2;j++) {

            if(j < p1->grid->length) {
                c = p1->grid->cases[i][j];
            } else {
                c = p2->grid->cases[i][j%p1->grid->length];
            }
            // si il est touché.
            if(c->state == TOUCHED) {
                // # si c'est un bateau.
                if(c->type == SHIP) {
                    printf("\033[1;32m#\033[00m");
                // X si c'est de l'eau.
                } else {
                    printf("\033[1;31mX\033[00m");
                }
            // si il n'est pas encore touché.
            } else {
                // et qu'il s'agit de la grille du joueur
                if(c->type == SHIP && j < p1->grid->length) {
                    // on affiche O
                    printf("O");
                } else {
                    // sinon rien.
                    printf(" ");
                }
            }
            // si on arrive à la grille suivante, on affiche leurs numéro de ligne.
            if(j == p1->grid->length - 1) {
                printf(" |\t\t");
                if((i+1) > 9) printf("| %d", i+1);
                else printf("| %d ", i+1);
            }
            printf(" | ");

        }
        printf("\n");
        // on va remettre un tiré pour séparer de la ligne suivante.
        for(int i = 0; i < ((p1->grid->length * 4) + 5) * 2;i++) {
            printf("-");
            if(i == ((p1->grid->length * 4) + 5) - 1) printf("-\t\t");
        } printf("-\n");
    }
}

int isDestroyed(Ship *s) {
    /** Fonction qui prend en paramètre un bateau, 
    et va regarder si ce bateau est détruit. Si toutes ces cases ont été touchées, on actualise son statut,
    sinon on retourne faux. */

    // si le bateau est déjà détruit, on retourne 1.
    if(s->state == DESTROYED) {
        return 1;
    // sinon.
    } else {
        int i = 0;
        // pour chaque case composant le navire, si il est touché, on incrémente.
        while(i < s->length && s->tabCase[i]->state == TOUCHED) i++;
        // si toutes les cases sont touché,
        if(i == s->length){
            // on l'actualise comme détruit, et on retourne 2.
            s->state = DESTROYED;
            return 2;
        }
    }
    // on retourne 0 si le navire n'a rien.
    return 0;
}

int isAlive(Player *p, typeShip t) {
    /** Fonction qui prend en paramètre un joueur et un type de bateau.
    On va regarder dans sa liste de bateau, celui avec le type "t" est en vie. 1 si il l'est, 0 sinon. */
    
    for(int i = 0;i < p->nbShip;i++) {
        if(p->tab_ship[i]->type == t && !isDestroyed(p->tab_ship[i])) return 1;
    }
    return 0;
}

int shipsDestroyed(Player *p) {
    /** Fonction qui prend en paramètre un joueur p,
    et va regarder si tous ses bateaux sont détruits. Si c'est le cas, on retourne 1 si c'est vrai, 0 sinon. */

    int nbShipsAlive = 0, stat;
    // on vérifie pour chaque navires.
    for(int i = 0; i < p->nbShip; i++) {
        // on regarde si il est détruit ou non.
        stat = isDestroyed(p->tab_ship[i]);
        // si c'est le cas on affiche un message.
        if (stat == 2) {
            printf("Un bateau de %s vient d'être détruit !\n", p->name);
        // sinon le bateau est encore en vie.
        } else if(stat == 0) {
            nbShipsAlive++;
        }
    }
    // on actualise le nombre de bateaux vivant du joueur.
    p->nbShip_alive = nbShipsAlive;
    // si il est null, ils sont tous détruits, on retourne 1.
    if(!nbShipsAlive) return 1;
    // sinon 0.
    return 0;
}

/** ----- Fonctions de tirs ----- */
Case **standardShoot(Grid *g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir sur une seule case où l'on va déclarer et retourne le tableau contenant cette case. */

    Case **caseCible = malloc(sizeof(Case));
    caseCible[0] = g->cases[x][y];
    return caseCible;
}

Case **lineShootH(Grid *g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et un ligne x de cette grille.
    La fonction concerne un tir sur l'ensemble des cases de la ligne horizontale (x) donnée en paramètre. */

    Case **caseCible = malloc(sizeof(Case) * g->length);
    int nbCase = 0;
    for(int i = 0; i < g->length;i++) {
        caseCible[nbCase++] = g->cases[x][i];
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

Case **lineShootV(Grid *g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et un ligne y de cette grille.
    La fonction concerne un tir sur l'ensemble des cases de la ligne verticale (y) donnée en paramètre. */

    Case **caseCible = malloc(sizeof(Case) * g->length);
    int nbCase = 0;
    for(int i = 0; i < g->length;i++) {
        caseCible[nbCase++] = g->cases[i][y];
    }
    caseCible = realloc(caseCible, sizeof(Case) * nbCase);
    
    return caseCible;
}

Case **crossShoot(Grid *g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case **caseCible = malloc(sizeof(Case) * 6);
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

Case **plusShoot(Grid *g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case **caseCible = malloc(sizeof(Case) * 6);
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

Case **squareShoot(Grid *g, int x, int y) {
    /** Prend en paramètre une grille de jeu, et une position (x, y) d'une case.
    La fonction concerne un tir en carré de taille 3x3 centré sur une case (x, y) passée en paramètre. */

    Case **caseCible = malloc(sizeof(Case) * 10);
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

void shootPlayer(Grid *g, int x, int y, Case ** (*pShoot) (Grid *, int, int)) {
    /** Prend en paramètre une grille, des coordonées x, y, et un pointeur de fonction de tir.
    La fonction a tirer dans chacunes des cases du tableau. Si on a tiré dans un bateau on retourne 1, sinon 0. */

    // on récupére le tableau de case correspondant à la fonction de tir.
    Case **caseCible = (*pShoot) (g, x, y);

    for(int i = 0; caseCible[i];i++) {
        caseCible[i]->state = TOUCHED;
    }
    free(caseCible);
}

void shootOrdi(Ordi *ord, Grid *g, int x, int y, Case ** (*pShoot) (Grid *, int, int)) {
    /** Prend en paramètre l'ordi, une grille, des coordonées x, y, et un pointeur de fonction de tir.
    La fonction a tirer dans chacunes des cases du tableau. Si on a tiré dans un bateau on retourne 1, sinon 0. */

    // on récupére le tableau de case correspondant à la fonction de tir.
    Case **caseCible = (*pShoot) (g, x, y);
    int shipCase = 0;

    for(int i = 0; caseCible[i];i++) {
        caseCible[i]->state = TOUCHED;
        if(caseCible[i]->type == SHIP) {
            ord->history[x][y] = 1;
        } else {
            ord->history[x][y] = 0;
        }
    }
    free(caseCible);
}

/** ----- Fonctions pour la partie ----- */
int *askCords(int lenMax) {
    /** Fonction qui ne prend aucun paramètre, et va demander des coordonnées tant qu'ils ne sont pas valide.
    Si les coordonnées sont valides, on les retournes dans un tableau [x, y]. */

    // on déclare quelques variables...
    int *tabCords = malloc(sizeof(int) * 2);char *cords;
    int cords_valid = 0, cord_x, cord_y, user_shoot, c;

    // tant que les coordonées ne sont pas valides.
    while(!cords_valid) {
        // on demande des coordonnées.
        printf("> ");
        scanf("%s", cords);

        cord_x = atoi(cords+1);
        // si les coordonnées sont valides.        
        if(strlen(cords) < 4 && cords[0] >= 'A' && cords[0] <= ('A' + lenMax) && cord_x >= 1 && cord_x <= 10) {     
            // on les ajoutes au tableau. 
            tabCords[0] = cord_x - 1;
            tabCords[1] = cords[0] - 'A';
            cords_valid = 1;
        // sinon on affiche le message d'erreur.
        } else {
            printf("'%s' n'est pas une case valide.\n", cords);
        }

        // on vide le buffer après chaque entrée.
        while ((c = getchar()) != '\n' && c != EOF);
    }

    // on retourne le tableau [x, y] des coordonnées de la case.
    return tabCords;
}

void placeShips(Player *p, typeShip *tabShip, int nbShips) {
    /** Prend en paramètre un joueur, un tableau de bateau à assigner, et leurs nombres.
    La fonction va demander au joueur les coordonnées des bateau à placer. */

    // on déclare quelques variables...
    int ships_placed = 0, *cords, ch_orientation = 0, lenShip, res = 0, c;
    Ship *s; OrientationShip o;

    // tant que les bateaux ne sont pas tous placés.    
    while(ships_placed < nbShips) {

        // on demande la coordonnées où placé le navire spécifique.
        ch_orientation = 0;
        printf("Indiquez les coordonnées pour placer votre ");

        // en fonction du navire qu'on va placé actuellement.
        // on va récupérer sa taille selon son type.
        switch (tabShip[ships_placed]) {
            case CARRIER:
                puts("porte-avion de 5 cases :");
                lenShip = 5;
                break;
        
            case CRUISER:
                puts("croiseur de 4 cases :");
                lenShip = 4;
                break;
        
            case DESTROYER:
                puts("destroyer de 3 cases :");
                lenShip = 3;
                break;
            case SUBMARINE:
                puts("sous-marin de 3 cases :");
                lenShip = 3;
                break;
            case TORPEDO:
                puts("torpilleur de 2 cases :");
                lenShip = 2;
                break;
        }
        // on récupére les coordonnées demandés.
        cords = askCords(p->grid->length);
        // tant que l'orientation n'est pas valide..
        while(ch_orientation < 1 || ch_orientation > 2) {
            puts("Orientation possibles :");
            puts("1 > HORIZONTALE");
            puts("2 > VERTICALE\n");

            // on l'a demande..
            printf("Indiquez l'orientation du bateau :\n> ");
            scanf("%d", &ch_orientation);

            // on vide le buffer.
            while ((c = getchar()) != '\n' && c != EOF); 
        }
        // on défini l'orientation en fonction du choix donnée.
        if(ch_orientation == 1) o = HORIZONTAL;
        else o = VERTICAL;

        // on initialise le navire.
        s = initShip(lenShip, o, tabShip[ships_placed]);
        // puis on le place.
        res = addShip(p->grid, s, cords[0], cords[1]);
        // si c'est bien placé, on incrémente le nombre de bateaux déjà positionnés.
        if(res == 1) {
            ships_placed++;
            puts("Bateau placé avec succès !\n");
        // sinon erreur, on recommence.
        } else puts("Vous ne pouvez pas placer le bateau ici !");
    }
}

void manageShoot(Player *p, Player *op, int *tabCords) {
    /** Prend en paramètre le joueur, son adversaire et les coordonnées de la case ciblée. 
    La fonction va demander au joueur le tir de son choix, et va effectuer ce tir si les conditions sont bien respectés. 
    (si le tir spécial n'a pas déjà été utilisé et que le bateau associé n'est pas détruit) */

    // on déclare quelques variables..
    int user_shoot = 0, shoot_valid = 0, shoot_line_valid = 0, c;
    char line_shoot[1];

    // tant que le tir n'est pas valide..
    while (!shoot_valid) {
        user_shoot = 0;

        // on demande au joueur d'indiqué un tir.
        while (user_shoot < 1 || user_shoot > 6) {
            puts("Liste des tirs possibles :\n1 > Tir normal (permet de viser une case)");
            puts("2 > Tir en ligne (permet de viser toute une ligne ou toute une colonne de la grille en une fois)");
            puts("3 > Tir en croix (permet de viser en une seule fois un ”x” centré sur une case et de 3 cases de circonférence)");
            puts("4 > Tir en plus (permet de viser en une seule fois un ”+” centré sur une case et de 3 cases de circonférence)");
            puts("5 > Tir en carée (permet de viser en une seule fois un carré de 3 cases par 3 centré sur une case)\n");
            
            printf("Quel type de tir voulez-vous utiliser ? (exemple : 1)\n> ");
            scanf("%d", &user_shoot);

            // on vide le buffer.
            while ((c = getchar()) != '\n' && c != EOF);
        }

        // une fois le choix de tir valide.
        // si le choix est 1, on effectue un tir standard.
        if(user_shoot == 1) {
            shootPlayer(op->grid, tabCords[0], tabCords[1], &standardShoot);
            shoot_valid = 1;

        // si le choix est 2 et que il n'a pas déjà été utilisé.
        } else if(user_shoot == 2 && p->shoot[0]) {
            // si le navire associé au tir est encore en vie.
            if(isAlive(p, SUBMARINE)) {
                shoot_line_valid = 0;
                // on demande l'option du tir en ligne.
                while(!shoot_line_valid) {
                    puts("Choix de sens possible :");
                    puts("L > En ligne");
                    puts("C > En colonne\n");

                    printf("Dans quel sens s'effectue le tir en ligne ? (exemple : C)\n> ");
                    scanf("%s", line_shoot);

                    // on effectue un tir en ligne en colonne.
                    if(line_shoot[0] == 'C') {
                        shootPlayer(op->grid, tabCords[0], tabCords[1], &lineShootV);
                        shoot_line_valid = 1;
                    
                    // on effectue un tir en ligne en ligne.
                    } else if(line_shoot[0] == 'L') {
                        shootPlayer(op->grid, tabCords[0], tabCords[1], &lineShootH);
                        shoot_line_valid = 1;
                    }
                }
                // on actualise les valeurs.
                shoot_valid = 1;
                p->shoot[0] = 0;
            // sinon erreur.
            } else {
                printf("> Vous ne pouvez pas tirer car votre SOUS-MARIN a été détruit !\n");                    
            }
        // si le choix est 3 et que il n'a pas déjà été utilisé.
        } else if(user_shoot == 3 && p->shoot[1]) {
            // si le navire associé est encore en vie.
            if(isAlive(p, CRUISER)) {
                // on effectue un tir en croix.
                shootPlayer(op->grid, tabCords[0], tabCords[1], &crossShoot);
                shoot_valid = 1;
                p->shoot[1] = 0;
            // sinon erreur.
            } else {
                printf("> Vous ne pouvez pas tirer car votre CROISEUR a été détruit !\n");                    
            }
        // si le choix est 4 et que il n'a pas déjà été utilisé.
        } else if(user_shoot == 4 && p->shoot[2]) {
            // si le navire associé est encore en vie.
            if(isAlive(p, CRUISER)) {
                // on effectue un tir en plus.
                shootPlayer(op->grid, tabCords[0], tabCords[1], &plusShoot);
                shoot_valid = 1;
                p->shoot[2] = 0;
            // sinon erreur.
            } else {
                printf("> Vous ne pouvez pas tirer car votre CROISEUR a été détruit !\n");                    
            }
        // si le choix est 5 et que il n'a pas déjà été utilisé.
        } else if(user_shoot == 5 && p->shoot[3]) {
            // si le navire associé est encore en vie.
            if(isAlive(p, CARRIER)) {
                // on effectue un tir en carré.
                shootPlayer(op->grid, tabCords[0], tabCords[1], &squareShoot);
                shoot_valid = 1;
                p->shoot[3] = 0;
            // sinon erreur.
            } else {
                printf("> Vous ne pouvez pas tirer car votre PORTE-AVION a été détruit !\n");                    
            }
        // sinon erreur.
        } else {
            puts("> Vous avez déjà effectué ce tir !\n");
        }
    }
    // on libére ensuite le tableau de coordonnées.
}

void startGame(Player *p1, Player *p2) {
    /** Prend en paramètre un joueur p1.
    La fonction va demander comment placer les bateaux du joueur et effectuer ce placement. */
    
    // On déclare quelques variables...
    typeShip tabShip[5] = {CARRIER, CRUISER, DESTROYER, SUBMARINE, TORPEDO};
    int ch = 0, c, stop = 0;

    // On demande comment le joueur souhaite placer ses navires.
    while(ch < 1 || ch > 2) {
        puts("\nComment voulez-vous placer vos bateaux ?");
        printf("1 > Manuellement\n2 > Aléatoirement\n> ");
        scanf("%d", &ch);

        // on vide le buffer.
        while ((c = getchar()) != '\n' && c != EOF);
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

void roundPlayer(Player *p1, Player *p2) {
    /** Fonction qui prend en paramètre deux joueurs et effectue le tour du joueur p1 contre le joueur p2,
    On demande les coordonnées à tirer, et le choix du tir. */

    // Un simple print pour indiqué au joueur de jouer.
    printf("C'est au tour de %s !\n", p1->name);
    
    // On demande la coordonées de tir.
    puts("\nA quelle case voulez-vous tirer ? (exemple : B2)");
    int *tabCords = askCords(p1->grid->length);
        
    // on demande puis effectue le tir aux coordonnées indiquées.
    manageShoot(p1, p2, tabCords);
    
    // on affiche sur la sortie standart le tir du joueur.
    printf("Résumé de la partie :\n> %s a effectué un tir en %c%d.\n", p1->name, 'A' + tabCords[0], tabCords[0]+1);
    free(tabCords);
}

void roundOrdi(Ordi *ord, Player *p1) {
    /** Prend en paramètre un ordi de type IA et un joueur.
    La fonction va gérer le tour d'un ordi et effectuer un tir selon son état. */
    
    // on déclare quelques variables...
    int val_x, val_y, found = 0, nbS_1, nbS_2, cords_valide;
    OrientationShoot tmp;

    // si l'IA est en état de recherche d'un bateau.
    if(ord->state == RESEARCH) {
        // si on on déjà effectué un tir.
        if(ord->lastCase != NULL) {
            // on sauvegarde les coordonées de tir,
            val_x = ord->lastCase->x;
            val_y = ord->lastCase->y;
            found = 0;

            // si il n'a pas déjà tirer à la case y+2 vers le bas.
            if(ord->lastCase->y+2 < ord->ordi->grid->length && ord->history[ord->lastCase->x][ord->lastCase->y+2] == -1) {
                val_y += 2;
                found = 1;
            // si il n'a pas déjà tirer à la case y+2 vers le haut.
            } else if(ord->lastCase->y-2 >= 0 && ord->history[ord->lastCase->x][ord->lastCase->y-2] == -1) {
                val_y -= 2;
                found = 1;
            // si il n'a pas déjà tirer à la case x+2 vers la droite.
            } else if(ord->lastCase->x+2 < ord->ordi->grid->length && ord->history[ord->lastCase->x+2][ord->lastCase->x] == -1) {
                val_x += 2;
                found = 1;
            // si il n'a pas déjà tirer à la case x-2 vers la gauche.
            } else if(ord->lastCase->x-2 >= 0 && ord->history[ord->lastCase->x-2][ord->lastCase->x] == -1) {
                val_x -= 2;
                found = 1;
            }

            if(ord->history[val_x][val_y] != -1) found = 0;
        }
        // sinon, on effectue le premier tir de manière aléatoire.
        if(!found || ord->lastCase == NULL) {
            cords_valide = 0;

            val_x = rand() % ord->ordi->grid->length;
            val_y = rand() % ord->ordi->grid->length;
        }
    // si l'IA est en état de détection d'orientation.
    } else if(ord->state == ORIENTATION) {

        // on sauvegarde les coordonées de tir,
        val_x = ord->lastCase->x;
        val_y = ord->lastCase->y;
        // on défini l'orientation du tir en UNDEFINED.
        tmp = UNDEFINED;

        // on vérifie si la case au dessus n'a pas encore été tiré.
        if(val_x-1 >= 0 && ord->history[val_x-1][val_y] == -1) {
            // si c'est le cas, on sauvegarde la coordonée et l'orientation en haut.
            val_x -= 1;
            tmp = TOP;
        // on vérifie si la case au dessus n'a pas encore été tiré.
        } else if(val_x+1 < ord->ordi->grid->length && ord->history[val_x+1][val_y] == -1) {
            // si c'est le cas, on sauvegarde la coordonée et l'orientation en bas.
            val_x += 1;
            tmp = BOTTOM;
        // on vérifie si la case au dessus n'a pas encore été tiré.
        } else if(val_y-1 >= 0 && ord->history[val_x][val_y-1] == -1) {
            // si c'est le cas, on sauvegarde la coordonée et l'orientation à gauche.
            val_y -= 1;
            tmp = LEFT;
        // on vérifie si la case au dessus n'a pas encore été tiré.
        } else if(val_y+1 < ord->ordi->grid->length && ord->history[val_x][val_y+1] == -1) {
            // si c'est le cas, on sauvegarde la coordonée et l'orientation à droite.
            val_y += 1;
            tmp = RIGHT;
        // si toutes les cases on été tirés, on effectue 
        } else {
            ord->state = RESEARCH;
            val_x = rand() % ord->ordi->grid->length;
            val_y = rand() % ord->ordi->grid->length;
        }
    // si l'IA est en état de destruction du navire.
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
                if(val_x - 1 >= 0 && ord->history[val_x-1][val_y] == -1) {
                    val_x -=1;
                    found = 1;
                // si la case suivante à déjà été tirée et que c'est un bateau,
                // on la garde en mémoire
                } else if(val_x - 1 >= 0 && ord->history[val_x-1][val_y] == 1) {
                    val_x -= 1;
                // si c'est de l'eau, on va dans l'autre sens.
                } else {
                    ord->shootOriented = BOTTOM;
                }
            // si on l'orientation est vers le bas.
            } else if(ord->shootOriented == BOTTOM) {
                //si la case suivante n'a pas encore été touche, on la prends
                if(val_x + 1 < ord->ordi->grid->length && ord->history[val_x+1][val_y] == -1) {
                    val_x += 1;
                    found = 1;
                // si la case suivante à déjà été tirée et que c'est un bateau,
                // on la garde en mémoire
                } else if(val_x + 1 < ord->ordi->grid->length && ord->history[val_x+1][val_y] == 1) {
                    val_x += 1;
                } else {
                    // si c'est de l'eau, on va dans l'autre sens.
                    ord->shootOriented = TOP;
                }
            // si on l'orientation est vers la gauche.
            } else if(ord->shootOriented == LEFT) {
                //si la case suivante n'a pas encore été touche, on la prends
                if(val_y - 1 >= 0 && ord->history[val_x][val_y-1] == -1) {
                    val_y -= 1;
                    found = 1;
                // si la case suivante à déjà été tirée et que c'est un bateau,
                // on la garde en mémoire
                } else if(val_y - 1 >= 0 && ord->history[val_x][val_y-1] == 1) {
                    val_y -= 1;
                } else {
                    // si c'est de l'eau, on va dans l'autre sens.
                    ord->shootOriented = RIGHT;
                }

            // si on l'orientation est vers la droite.
            } else if(ord->shootOriented == RIGHT) {
                //si la case suivante n'a pas encore été touche, on la prends
                if(val_y + 1 < ord->ordi->grid->length && ord->history[val_x][val_y+1] == -1) {
                    val_y += 1;
                    found = 1;
                // si la case suivante à déjà été tirée et que c'est un bateau,
                // on la garde en mémoire
                } else if(val_y + 1 < ord->ordi->grid->length && ord->history[val_x][val_y+1] == 1) {
                    val_y += 1;
                } else {
                    // si c'est de l'eau, on va dans l'autre sens.
                    ord->shootOriented = LEFT;
                }
            }
        }
    }
    // on effectue le tir.
    shootOrdi(ord, p1->grid, val_x, val_y, &standardShoot);

    // Si on tire sur un bateau..
    if(ord->history[val_x][val_y] == 1) {

        // on sauevarde la case.
        ord->lastCase = p1->grid->cases[val_x][val_y];

        // si on est en état de recherche
        if(ord->state == RESEARCH) {

            // on passe à un état d'orientation.        
            ord->state = ORIENTATION;
        
        // si on est en état d'orientation.
        } else if(ord->state == ORIENTATION) {
            // on donne l'orientation trouvé à l'ordi. 
            ord->shootOriented = tmp;

            // on regarde si le nombre de bateau vivant du joueur.
            // avant et après avoir check ses bateaux.
            nbS_1 = p1->nbShip_alive;
            shipsDestroyed(p1);
            nbS_2 = p1->nbShip_alive;

            // si le nombre de bateaux vivant à changés,
            // c'est qu'il vient d'être détruit.
            if(nbS_1 != nbS_2) {
                // on repasse donc en état de recherche d'un nouveau bateau.
                ord->state = RESEARCH;
            } else {
                // sinon on passe en état de destruction.
                ord->state = DESTRUCTION;
            }
        // si on est en état de destruction du navire.
        } else if(ord->state == DESTRUCTION) {
            // on regarde si le nombre de bateau vivant du joueur.
            // avant et après avoir check ses bateaux.
            nbS_1 = p1->nbShip_alive;
            shipsDestroyed(p1);
            nbS_2 = p1->nbShip_alive;

            // si le nombre de bateaux vivant à changés,
            // c'est qu'il vient d'être détruit.
            if(nbS_1 != nbS_2) {
                // on repasse donc en état de recherche d'un nouveau bateau.
                ord->state = RESEARCH;
                ord->lastCase = NULL;
            }
        }
    // Si il tire dans l'eau..
    } else {
        
       
        // Si on est en état de recherche, on sauvegarde la case tiré.
        if(ord->state == RESEARCH) {
            ord->lastCase = p1->grid->cases[val_x][val_y];

        // Si on est en état de destruction & que le bateau n'est pas encore détruit.
        } else if(ord->state == DESTRUCTION) {

            // On va à la direction opposé pour continuer la destruction du bateau.Z
            if(ord->shootOriented == TOP) ord->shootOriented = BOTTOM;
            else if(ord->shootOriented == BOTTOM) ord->shootOriented = TOP;
            else if(ord->shootOriented == LEFT) ord->shootOriented = RIGHT;
            else if(ord->shootOriented == RIGHT) ord->shootOriented = LEFT;
        }
        
    }
    // on affiche sur la sortie standart le tir de l'ORDI.
    printf("> %s a effectué un tir en %c%d.\n", ord->ordi->name, 'A' + val_y, val_x+1);
}

void playGame(Player *p1, Ordi *ord) {
    /** Prend en paramètre deux joueurs p1 et p2.
    La fonction va s'occuper du déroulement de la partie entre les deux joueurs. */

    int end = 0;
    Player *p = p1;
    Ordi *ia = ord;

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
            printGrid(p, ia->ordi);
            puts("\nPartie terminé ! Vous avez détruit l'ensemble des navires ennemis.");
            end = 1;
        } else {
            // Sinon, on effectue le round du joueur 2 (soit l'IA).
            roundOrdi(ia, p);
            if(shipsDestroyed(p1)) {
                printGrid(p, ia->ordi);
                puts("\nPartie terminé ! L'ORDI à détruit l'ensemble de vos navires.");
                end = 1;
            }
        }
    }
}

void cleanPlayer(Player *p) {
    /** Fonction qui prend en paramètre un joueur,
    et va libérer l'espace mémoire qui lui été réservé. */

    for(int i = 0; i < p->grid->length; i++) { 
        for(int j = 0; j < p->grid->length; j++) { 
            free(p->grid->cases[i][j]); 
        } 
        free(p->grid->cases[i]); 
    } 
    free(p->grid->cases); 
    for(int i = 0; i < p->nbShip;i++) {
        free(p->tab_ship[i]->tabCase);
        free(p->tab_ship[i]);
    }
    free(p->shoot); 
    free(p->tab_ship);  
    free(p->grid);
    free(p);
}  

void cleanIA(Ordi *ord) {
    /** Fonction qui prend en paramètre un ORDI,
    et va libérer l'espace mémoire qui lui été réservé. */

    for(int i = 0; i < ord->ordi->grid->length; i++) { 
        free(ord->history[i]); 
    }
    free(ord->history);
    cleanPlayer(ord->ordi);
    free(ord);
} 

void battleShip() {
    /** Fonction principale du jeu qui ne prend pas de paramètre,
    et va initialiser les joueurs, lancer la partie et à la fin libérer l'espace mémoire utilisé durant la partie. */

    // On déclare les deux joeuurs et on les initialises.
    Player *p1; Ordi *ord;
    char *name = malloc(sizeof(char) * 25);int sizeGrid = 0, c;
    srand(time(NULL));

    // message d'accueil
    puts("-------- BATAILLE NAVALE --------");
    puts("Règles : La bataille navale oppose deux joueurs qui s'affrontent. Chacun a une flotte composée de 5 bateaux, qui sont, en général, les suivants :");
    puts("1 porte-avion (5 cases), 1 croiseur (4 cases), 1 destroyer (3 cases), 1 sous-marin (3 cases), 1 torpilleur (2 cases)");
    puts("Les bateaux ne doivent pas être collés entre eux.\n");

    // on demande la taille de la grille.
    while(sizeGrid < 8 || sizeGrid > 15) {
        printf("Indiquez la taille de la grille des joueurs (comprise entre 8 et 15) :\n> ");
        scanf("%d", &sizeGrid);

        // On vide le buffer.
        while ((c = getchar()) != '\n' && c != EOF);
    }

    // on demande le nom du joueur.
    printf("Indiquez le nom du joueur n°1 :\n> ");
    scanf("%s", name);
    
    // on initialise le joueur et l'ordi.
    p1 = initPlayer(name, sizeGrid);
    ord = initOrdi(sizeGrid);

    // on demande des précisions au joueur avant de démarrer la partie.
    startGame(p1, ord->ordi);
    playGame(p1, ord);

    // on vide la mémoire.
    free(p1->name);
    cleanPlayer(p1);
    cleanIA(ord);
}
