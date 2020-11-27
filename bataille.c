#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_GRID 10

enum typeCase {
    SHIP, WATER
};

enum stateCase {
    TOUCHED, NOT_TOUCHED
};
enum orientedShip {
    VERTICAL, HORIZONTAL
};

enum stateShip {
    DESTROYED, ALIVE
};

typedef struct _Case {
    int x;
    int y;
    enum typeCase type;
    enum stateCase state;
} *Case;

typedef struct _Ship {
    int x;
    int y;
    int length;
    enum orientedShip oriented;
} *Ship;

typedef struct _Player {
    char *name;
    int nbShip;
    int nbShip_alive;
    Case **grid;
    Ship *tab_ship;
} *Player;

Case **initGrid() {
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
    Player p = malloc(sizeof(Player));
    p->name = name;
    p->nbShip = p->nbShip_alive = nbShip;
    p->grid = initGrid();
    p->tab_ship = NULL;
    return p;
}

void printGrid(Case **grid) {

    for(int i = 0; i < SIZE_GRID; i++) {
        printf("\t%c", 'a' + i);
    } printf("\n");

    for(int i = 0; i < SIZE_GRID; i++) {
        printf("%d\t", i+1);
        for(int j = 0; j < SIZE_GRID;j++) {
            if(grid[i][j]->state == TOUCHED) {
                if(grid[i][j]->type == SHIP) {
                    printf("#");
                } else {
                    printf("X");
                }
            } else {
                if(grid[i][j]->type == SHIP) {
                    printf("O");
                } else {
                    printf(".");
                }
            }
            printf("\t");
        } printf("\n");

    }
}

int playerShoot(Case **grid, int x, int y) {
    /** 0 = eau | 1 = deja touche | 2 = touche */

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

void main() {
    Player p = initPlayer("Edward", 10);
    playerShoot(p->grid, 2, 3);
    printGrid(p->grid);
}