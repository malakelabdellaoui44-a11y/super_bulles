#ifndef RENDER_H
#define RENDER_H

/* ============================================================
 * render.h - Interface de la couche graphique (Allegro 4)
 * ============================================================ */

#include "game.h"

/* --- Initialisation / nettoyage Allegro --- */
int  render_init(void);
void render_cleanup(void);

/* --- Rendu par état --- */
void render_menu(int selected_item);
void render_rules(void);
void render_level_intro(int level_number, int countdown);
void render_game(const Game *g);
void render_level_win(int score);
void render_level_fail(void);
void render_game_win(int score);
void render_game_over(int score);
void render_save_screen(void);

/* --- Rendu des entités --- */
void render_player(const Player *p);
void render_bubble(const Bubble *b);
void render_projectile(const Projectile *pr);
void render_weapon_pickup(const WeaponPickup *w);
void render_lightning(const Lightning *l);
void render_boss(const Boss *boss);

/* --- HUD (zone d'informations) --- */
void render_hud(const Game *g);

/* --- Animations --- */
void render_explosion(float x, float y, int frame);
void render_victory_animation(int frame);

/* --- Double buffering --- */
void render_flip(void);

#endif /* RENDER_H */
