#ifndef LOGIC_H
#define LOGIC_H

/* ============================================================
 * logic.h - Interface de la couche logique métier
 * Aucune dépendance à Allegro dans ce module.
 * ============================================================ */

#include "game.h"

/* --- Initialisation --- */
void logic_init_game(Game *g, const char *pseudo);
void logic_init_level(Game *g, int level_number);
void logic_init_player(Player *p, const char *pseudo);
void logic_init_bubble(Bubble *b, float x, float y, BubbleSize size,
                       float vx, float vy, int shoots_lightning);
void logic_init_boss(Boss *boss);

/* --- Mise à jour (appelée chaque frame) --- */
void logic_update(Game *g);
void logic_update_player(Game *g, int move_left, int move_right);
void logic_update_bubbles(Level *lv);
void logic_update_projectiles(Level *lv);
void logic_update_weapons_on_ground(Level *lv);
void logic_update_lightnings(Level *lv, Player *p);
void logic_update_boss(Game *g);
void logic_update_timer(Level *lv);

/* --- Actions joueur --- */
void logic_player_shoot(Game *g);
void logic_player_pick_weapon(Game *g);

/* --- Collisions --- */
void logic_check_collisions(Game *g);
void logic_bubble_hit(Game *g, int bubble_idx, int proj_idx);
void logic_split_bubble(Level *lv, Bubble *b);

/* --- Conditions de victoire / défaite --- */
int  logic_all_bubbles_dead(const Level *lv);
int  logic_is_level_won(const Game *g);
int  logic_is_level_lost(const Game *g);
int  logic_is_game_won(const Game *g);

/* --- Score --- */
void logic_add_score(Game *g, int points);
int  logic_bubble_points(BubbleSize size);

/* --- Sauvegarde --- */
int  logic_save_game(const Game *g, const char *filename);
int  logic_load_game(Game *g, const char *pseudo, const char *filename);

#endif /* LOGIC_H */
