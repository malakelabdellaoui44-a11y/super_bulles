#ifndef GAME_H
#define GAME_H


#define WIN_W           1024
#define WIN_H           768
#define GAME_ZONE_H     680   /* hauteur de la zone de jeu */
#define INFO_ZONE_H     (WIN_H - GAME_ZONE_H)

#define NB_LEVELS       4
#define LEVEL_TIME      90    /* secondes par niveau */
#define PLAYER_SPEED    4
#define BULLET_SPEED    8

#define MAX_BUBBLES     32
#define MAX_PROJECTILES 8
#define MAX_WEAPONS     4
#define MAX_PSEUDO      32

/* Tailles de bulles */
#define BUBBLE_SIZE_BIG    40
#define BUBBLE_SIZE_MEDIUM 25
#define BUBBLE_SIZE_SMALL  12

/* Gravité et rebond */
#define GRAVITY         0.3f
#define BOUNCE_DAMPING  0.85f

/* --- Etats du jeu --- */
typedef enum {
    STATE_MENU,
    STATE_RULES,
    STATE_PLAYING,
    STATE_LEVEL_INTRO,
    STATE_LEVEL_WIN,
    STATE_LEVEL_FAIL,
    STATE_BOSS,
    STATE_GAME_WIN,
    STATE_GAME_OVER,
    STATE_SAVE_LOAD,
    STATE_QUIT
} GameState;

/* --- Types d'armes --- */
typedef enum {
    WEAPON_SIMPLE,    /* arme de base : 1 tir à la fois */
    WEAPON_FAST,      /* tirs rapides */
    WEAPON_DOUBLE,    /* 2 tirs simultanés */
    WEAPON_BOMB       /* explosion */
} WeaponType;

/*  Taille des bulles */
typedef enum {
    BUBBLE_BIG    = 0,
    BUBBLE_MEDIUM = 1,
    BUBBLE_SMALL  = 2
} BubbleSize;

/* --- Structure : Joueur --- */
typedef struct {
    float x, y;           /* position (centre bas du perso) */
    int   width, height;  /* dimensions du sprite */
    int   speed;
    int   alive;          /* 1 = vivant, 0 = mort */
    WeaponType weapon;
    int   weapon_timer;   /* frames restantes pour l'arme spéciale */
    int   direction;      /* -1 = gauche, 1 = droite */
    char  pseudo[MAX_PSEUDO];
} Player;

/* --- Structure : Projectile --- */
typedef struct {
    float x, y;
    float vx, vy;
    int   active;
    WeaponType type;
} Projectile;

/* --- Structure : Bulle --- */
typedef struct {
    float      x, y;       /* centre de la bulle */
    float      vx, vy;     /* vitesse */
    BubbleSize size;
    int        radius;     /* rayon en pixels selon la taille */
    int        active;
    int        has_weapon; /* 1 = contient une arme cachée */
    WeaponType weapon_type;
    int        shoots_lightning; /* niveau 3+ : lance des éclairs */
    int        lightning_timer;
} Bubble;

/* --- Structure : Arme au sol --- */
typedef struct {
    float      x, y;
    WeaponType type;
    int        active;
    int        lifetime; /* frames avant disparition */
} WeaponPickup;

/* --- Structure : Éclair --- */
typedef struct {
    float x;           /* position X de l'éclair */
    float y;           /* position Y courante (tombe) */
    float speed;
    int   active;
} Lightning;

/* --- Structure : Boss --- */
typedef struct {
    float x, y;
    int   width, height;
    float vx;
    int   hp;          /* points de vie restants */
    int   max_hp;
    int   active;
    int   bubble_timer; /* frames avant de lancer une bulle */
} Boss;

/* --- Structure : Niveau --- */
typedef struct {
    int       number;
    int       time_left;    /* secondes restantes */
    int       timer_frames; /* compteur interne de frames */
    int       nb_bubbles_init;
    Bubble    bubbles[MAX_BUBBLES];
    int       nb_projectiles;
    Projectile projectiles[MAX_PROJECTILES];
    WeaponPickup weapons[MAX_WEAPONS];
    Lightning lightnings[MAX_WEAPONS];
    Boss      boss;
    int       countdown;    /* décompte 3..0 au démarrage */
    int       countdown_frames;
} Level;

/* --- Structure : Partie --- */
typedef struct {
    Player player;
    Level  current_level;
    int    level_number;  /* 1..NB_LEVELS */
    int    score;
    int    nb_bubbles_destroyed;
} Game;

/* --- Structure : Sauvegarde --- */
typedef struct {
    char pseudo[MAX_PSEUDO];
    int  last_level;
    int  score;
} SaveEntry;

#endif /* GAME_H */
