// Macros stantards.
#define isalnum(a) (isalpha(a) || isdigit(a))
#define isalpha(a) (isupper(a) || islower(a))
#define islower(a) (((a) >= 'a') && ((a) <= 'z'))
#define isupper(a) (((a) >= 'A') && ((a) <= 'Z'))
#define isdigit(a) (((a) >= '0') && ((a) <= '9'))

/** ----- Enumérations ----- */
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
    /** On définit l'énumation de l'orientation d'un bateau dans une grille.
    Soit on s'oriente verticalement, soit horizontalement. */

    VERTICAL, HORIZONTAL
} OrientationShip;

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

typedef enum {
    /** On définit l'énumation de tous les états de L'ORDI.
    RESEARCH : recherche de navire ennemi, ORIENTATION : détection de l'orientation à prendre
    et DESTRUCTION : la destruction du navire.*/

    RESEARCH, ORIENTATION, DESTRUCTION
} stateOrdi;

typedef enum {
    /** On définit l'énumation de tous les type de bateau.
    CARRIER = 5 cases (porte-avion), CRUISER = 4 cases (croiseur), 
    DESTROYER = 3 cases (destroyer), SUBMARINE = 3 cases (sous-marin) et TORPEDO = 2 cases (torpilleur) */

    TOP, LEFT, RIGHT, BOTTOM, UNDEFINED
} OrientationShoot;

/** ----- Structures ----- */
typedef struct _Case {
    /** On définit la structure d'une case composé des ses positions (x, y), 
    de son type de case (si il contient un bateau ou de l'eau) et de l'état de cette case
    (si elle a été touché ou non). */

    int x;
    int y;
    typeCase type;
    stateCase state;
} *Case;

typedef struct _Grid {
    Case **cases;
    int length;
} *Grid;

typedef struct _Ship {
    /** On définit la structure d'un bateau composé d'un tableau de case (contenant les cases qui compose ce bateau), 
    de sa longueur, et de son orientation (verticale, ou horizontale). */

    Case *tabCase;
    int length;
    OrientationShip oriented;
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
    Grid grid;
    Ship *tab_ship;
    // Le nombre de tirs spéciaux restants.
    int *shoot;
} *Player;

typedef struct _Ordi {
    /** On définit la structure d'une IA, qui est une extention d'un joueur,
    mais qui possède également l'orientation de ses tirs, ses cases à ciblées
    son état dans lequel il se trouve et l'historique de ses coups. */

    Player ordi;
    OrientationShoot shootOriented;
    stateOrdi state;
    Case lastCase;
    int **history;
} *Ordi;

/** ----- Fonctions ----- */
// Fonctions d'initialisations.
Grid initGrid(int);
Player initPlayer(char *, int);
Ordi initOrdi(int);
Ship initShip(int, OrientationShip, typeShip);

// Fonctions utilitaires.
int addShip(Grid, Ship, int, int);
void fillGrid(Player, typeShip *, int);
void printGrid(Player, Player);

int isDestroyed(Ship);
int isAlive(Player, typeShip);
int shipsDestroyed(Player);

// Fonctions de tirs.
Case *standardShoot(Grid, int, int);
Case *lineShootH(Grid, int, int);
Case *lineShootV(Grid, int, int);
Case *crossShoot(Grid, int, int);
Case *plusShoot(Grid, int, int);
Case *squareShoot(Grid, int, int);
void shootPlayer(Grid, int, int, Case* (*)(Grid, int, int));
Case shootOrdi(Ordi, Grid, int, int, Case* (*)(Grid, int, int));

// Fonctions pour la partie.
void initGame(Player *, Ordi *);
int *askCords(int);
void placeShips(Player, typeShip *, int);
void manageShoot(Player, Player, int *);
void startGame(Player, Player);

void roundPlayer(Player, Player);
void roundOrdi(Ordi, Player);
void playGame(Player, Ordi);

// Fonctions qui nettoye la mémoire utilisée.
void cleanPlayer(Player);
void cleanIA(Ordi o);

// Fonction principale qui exécute le programme.
void battleShip(void);
