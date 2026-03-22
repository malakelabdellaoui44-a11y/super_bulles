/* ============================================================
 * main.c - Point d'entrée et boucle principale
 * Super Bulles - Projet Informatique ING1 ECE Paris 2025-2026
 * ============================================================ */

#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "logic.h"
#include "render.h"
#include "ihm.h"

#define SAVE_FILE   "savegame.txt"
#define TARGET_FPS  60
#define FRAME_MS    (1000 / TARGET_FPS)

/* ============================================================
 * GESTION DU MENU PRINCIPAL
 * ============================================================ */

static GameState run_menu(Game *g)
{
    int selected = 0;
    InputState in;

    while (1) {
        ihm_poll(&in);
        selected = ihm_menu_navigate(selected, 4, &in);

        if (ihm_menu_confirm(&in)) {
            switch (selected) {
                case 0: return STATE_RULES;
                case 1:
                    ihm_read_pseudo(g->player.pseudo, MAX_PSEUDO);
                    logic_init_game(g, g->player.pseudo);
                    return STATE_LEVEL_INTRO;
                case 2:
                    ihm_read_pseudo(g->player.pseudo, MAX_PSEUDO);
                    if (logic_load_game(g, g->player.pseudo, SAVE_FILE)) {
                        logic_init_level(g, g->level_number);
                        return STATE_LEVEL_INTRO;
                    }
                    /* pas de sauvegarde trouvée : afficher erreur brève */
                    break;
                case 3: return STATE_QUIT;
            }
        }
        if (in.escape) return STATE_QUIT;

        render_menu(selected);
        render_flip();
        rest(FRAME_MS);
    }
}

/* ============================================================
 * BOUCLE DE JEU D'UN NIVEAU
 * ============================================================ */

static GameState run_level(Game *g)
{
    Level     *lv = &g->current_level;
    InputState in;
    int        shoot_prev = 0;

    /* Décompte initial */
    lv->countdown        = 3;
    lv->countdown_frames = 0;

    while (lv->countdown > 0) {
        lv->countdown_frames++;
        if (lv->countdown_frames >= TARGET_FPS) {
            lv->countdown_frames = 0;
            lv->countdown--;
        }
        render_level_intro(g->level_number, lv->countdown);
        render_flip();
        rest(FRAME_MS);
    }

    /* Boucle de jeu principale */
    while (1) {
        long t0 = (long)retrace_count;

        ihm_poll(&in);

        /* Tir sur front montant */
        if (in.shoot && !shoot_prev) logic_player_shoot(g);
        shoot_prev = in.shoot;

        /* Déplacement */
        logic_update_player(g, in.move_left, in.move_right);

        /* Mise à jour logique */
        logic_update(g);

        /* Rendu */
        render_game(g);
        render_flip();

        /* Conditions de fin */
        if (logic_is_level_won(g))  return STATE_LEVEL_WIN;
        if (logic_is_level_lost(g)) return STATE_LEVEL_FAIL;
        if (in.escape)              return STATE_MENU;

        /* Cadence 60fps */
        while ((long)retrace_count - t0 < 1) rest(1);
    }
}

/* ============================================================
 * ÉCRAN FIN DE NIVEAU
 * ============================================================ */

static GameState run_level_win(Game *g)
{
    InputState in;

    /* Bonus de temps */
    logic_add_score(g, g->current_level.time_left * 10);

    while (1) {
        ihm_poll(&in);
        render_level_win(g->score);
        render_flip();

        if (ihm_menu_confirm(&in)) {
            /* Niveau suivant ou victoire finale */
            if (g->level_number >= NB_LEVELS)
                return STATE_GAME_WIN;
            g->level_number++;
            logic_init_level(g, g->level_number);
            return STATE_LEVEL_INTRO;
        }
        /* Sauvegarde */
        if (key[KEY_S]) {
            logic_save_game(g, SAVE_FILE);
            render_save_screen();
            render_flip();
            rest(1500);
        }
        if (in.escape) return STATE_MENU;
        rest(FRAME_MS);
    }
}

static GameState run_level_fail(Game *g)
{
    InputState in;
    while (1) {
        ihm_poll(&in);
        render_level_fail();
        render_flip();

        if (ihm_menu_confirm(&in)) {
            /* Recommencer le niveau */
            logic_init_level(g, g->level_number);
            return STATE_LEVEL_INTRO;
        }
        if (in.escape) return STATE_MENU;
        rest(FRAME_MS);
    }
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main(void)
{
    Game      g;
    GameState state;

    if (!render_init()) return 1;
    ihm_init();

    memset(&g, 0, sizeof(Game));
    state = STATE_MENU;

    while (state != STATE_QUIT) {
        switch (state) {
            case STATE_MENU:
                state = run_menu(&g);
                break;

            case STATE_RULES:
                {
                    InputState in;
                    while (1) {
                        ihm_poll(&in);
                        render_rules();
                        render_flip();
                        if (in.escape || ihm_menu_confirm(&in)) { state = STATE_MENU; break; }
                        rest(FRAME_MS);
                    }
                }
                break;

            case STATE_LEVEL_INTRO:
                state = run_level(&g);
                break;

            case STATE_LEVEL_WIN:
                state = run_level_win(&g);
                break;

            case STATE_LEVEL_FAIL:
                state = run_level_fail(&g);
                break;

            case STATE_GAME_WIN:
                {
                    InputState in;
                    int frame = 0;
                    while (1) {
                        ihm_poll(&in);
                        render_game_win(g.score);
                        render_victory_animation(frame++);
                        render_flip();
                        if (ihm_menu_confirm(&in) || in.escape) { state = STATE_MENU; break; }
                        rest(FRAME_MS);
                    }
                }
                break;

            case STATE_GAME_OVER:
                {
                    InputState in;
                    while (1) {
                        ihm_poll(&in);
                        render_game_over(g.score);
                        render_flip();
                        if (ihm_menu_confirm(&in) || in.escape) { state = STATE_MENU; break; }
                        rest(FRAME_MS);
                    }
                }
                break;

            default:
                state = STATE_MENU;
                break;
        }
    }

    render_cleanup();
    return 0;
}
END_OF_MAIN()
