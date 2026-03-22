/* ============================================================
 * render.c - Couche graphique avec Allegro 4
 * ============================================================ */

#include <allegro.h>
#include <string.h>
#include <stdio.h>
#include "render.h"

static BITMAP *buffer = NULL; /* double buffer */

/* Couleurs (initialisées dans render_init) */
static int COL_BG, COL_WHITE, COL_BLACK, COL_RED, COL_BLUE,
           COL_GREEN, COL_YELLOW, COL_ORANGE, COL_CYAN,
           COL_GRAY, COL_DARKGRAY, COL_HUD_BG;

/* ============================================================
 * INITIALISATION
 * ============================================================ */

int render_init(void)
{
    allegro_init();
    install_keyboard();
    install_mouse();

    set_color_depth(32);
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIN_W, WIN_H, 0, 0) != 0) {
        allegro_message("Erreur graphique : %s", allegro_error);
        return 0;
    }

    set_window_title("Super Bulles - ECE Paris ING1");

    buffer = create_bitmap(WIN_W, WIN_H);
    if (!buffer) return 0;

    /* Palette de couleurs */
    /* Palette pastel douce */
    COL_BG       = makecol(238, 232, 248); /* violet très pâle */
    COL_WHITE    = makecol(250, 250, 252); /* blanc légèrement teinté */
    COL_BLACK    = makecol(60,  50,  80);  /* quasi-noir violet */
    COL_RED      = makecol(255, 179, 179); /* rose pastel — grosses bulles */
    COL_BLUE     = makecol(179, 217, 255); /* bleu ciel — joueur */
    COL_GREEN    = makecol(200, 240, 220); /* menthe — victoire / armes */
    COL_YELLOW   = makecol(247, 214, 240); /* lavande rosée — titres / score */
    COL_ORANGE   = makecol(255, 221, 170); /* pêche — bulles moyennes */
    COL_CYAN     = makecol(184, 240, 244); /* cyan pâle — temps */
    COL_GRAY     = makecol(200, 192, 220); /* gris violet — texte secondaire */
    COL_DARKGRAY = makecol(140, 130, 165); /* gris foncé — sélection menu */
    COL_HUD_BG   = makecol(42,  31,  74);  /* violet sombre — fond HUD */

    return 1;
}

void render_cleanup(void)
{
    if (buffer) {
        destroy_bitmap(buffer);
        buffer = NULL;
    }
    allegro_exit();
}


/* ============================================================
 * TEXTE AGRANDI via stretch_blit
 * ============================================================ */

static void draw_text_scaled(const char *txt, int cx, int cy, int col, int scale)
{
    int tw = text_length(font, txt);
    int th = text_height(font);
    BITMAP *tmp = create_bitmap(tw + 2, th + 2);
    if (!tmp) { textout_centre_ex(buffer, font, txt, cx, cy, col, -1); return; }
    clear_to_color(tmp, makecol(1, 1, 1));
    textout_ex(tmp, font, txt, 1, 1, col, -1);
    int dw = (tw + 2) * scale;
    int dh = (th + 2) * scale;
    int dx = cx - dw / 2;
    int dy = cy - dh / 2;
    masked_stretch_blit(tmp, buffer, 0, 0, tw + 2, th + 2, dx, dy, dw, dh);
    destroy_bitmap(tmp);
}

static void draw_text_scaled_left(const char *txt, int x, int y, int col, int scale)
{
    int tw = text_length(font, txt);
    int th = text_height(font);
    BITMAP *tmp = create_bitmap(tw + 2, th + 2);
    if (!tmp) { textout_ex(buffer, font, txt, x, y, col, -1); return; }
    clear_to_color(tmp, makecol(1, 1, 1));
    textout_ex(tmp, font, txt, 1, 1, col, -1);
    int dw = (tw + 2) * scale;
    int dh = (th + 2) * scale;
    masked_stretch_blit(tmp, buffer, 0, 0, tw + 2, th + 2, x, y, dw, dh);
    destroy_bitmap(tmp);
}

/* ============================================================
 * MENU
 * ============================================================ */

void render_menu(int selected_item)
{
    const char *items[] = {
        "1. Lire les regles du jeu",
        "2. Nouvelle partie",
        "3. Reprendre une partie",
        "4. Quitter"
    };
    int nb = 4;
    int i;

    clear_to_color(buffer, COL_BG);

    /* Titre gros (scale 3) */
    draw_text_scaled("SUPER BULLES", WIN_W/2, 100, COL_YELLOW, 3);
    draw_text_scaled("ECE Paris - ING1 2025-2026", WIN_W/2, 155, COL_GRAY, 2);

    /* Items du menu (scale 2) */
    for (i = 0; i < nb; i++) {
        int y   = 240 + i * 70;
        int col = (i == selected_item) ? COL_YELLOW : COL_WHITE;
        if (i == selected_item)
            rectfill(buffer, WIN_W/2 - 220, y - 8,
                     WIN_W/2 + 220, y + 30, COL_DARKGRAY);
        draw_text_scaled(items[i], WIN_W/2, y, col, 2);
    }

    draw_text_scaled("Fleches haut/bas : naviguer   Entree : valider",
        WIN_W/2, WIN_H - 40, COL_GRAY, 2);
}

/* ============================================================
 * REGLES
 * ============================================================ */

void render_rules(void)
{
    const char *lines[] = {
        "REGLES DU JEU",
        "",
        "- Eliminez toutes les bulles de chaque niveau",
        "- Tirez avec ESPACE pour diviser les bulles",
        "- Deplacez-vous avec les fleches gauche/droite",
        "- Evitez d'etre touche par une bulle ou un eclair",
        "- A partir du niveau 3, des eclairs tombent des bulles",
        "- Le niveau 4 se termine par un Boss final",
        "- Ramassez les armes au sol pour ameliorer vos tirs",
        "",
        "Appuyez sur ECHAP pour revenir au menu"
    };
    int n = 11, i;
    clear_to_color(buffer, COL_BG);
    for (i = 0; i < n; i++)
        draw_text_scaled(lines[i], WIN_W/2, 100 + i * 45,
                         i == 0 ? COL_YELLOW : COL_WHITE, 2);
}

/* ============================================================
 * INTRO NIVEAU
 * ============================================================ */

void render_level_intro(int level_number, int countdown)
{
    char buf[64];
    clear_to_color(buffer, COL_BG);

    snprintf(buf, sizeof(buf), "NIVEAU %d", level_number);
    draw_text_scaled(buf, WIN_W/2, 250, COL_YELLOW, 3);

    if (countdown > 0) {
        snprintf(buf, sizeof(buf), "%d", countdown);
        draw_text_scaled(buf, WIN_W/2, 360, COL_WHITE, 4);
    } else {
        draw_text_scaled("PRET !", WIN_W/2, 360, COL_GREEN, 3);
    }
}

/* ============================================================
 * RENDU DES ENTITES
 * ============================================================ */

void render_player(const Player *p)
{
    if (!p->alive) return;
    int x = (int)p->x;
    int y = (int)p->y;
    int w = p->width;
    int h = p->height;

    /* Corps (rectangle bleu pastel) */
    rectfill(buffer, x - w/2, y - h, x + w/2, y, COL_BLUE);
    rect(buffer,     x - w/2, y - h, x + w/2, y, COL_DARKGRAY);
    /* Tete */
    circlefill(buffer, x, y - h - 8, 10, COL_CYAN);
    circle(buffer,     x, y - h - 8, 10, COL_DARKGRAY);

    /* Indicateur d'arme */
    if (p->weapon != WEAPON_SIMPLE) {
        int col = (p->weapon == WEAPON_FAST)   ? COL_GREEN  :
                  (p->weapon == WEAPON_DOUBLE)  ? COL_ORANGE :
                                                   COL_RED;
        textprintf_ex(buffer, font, x - 10, y - h - 25, col, -1, "W");
    }
}

void render_bubble(const Bubble *b)
{
    if (!b->active) return;
    int col = (b->size == BUBBLE_BIG)    ? COL_RED    :
              (b->size == BUBBLE_MEDIUM) ? COL_ORANGE :
                                           COL_YELLOW;
    circlefill(buffer, (int)b->x, (int)b->y, b->radius, col);
    circle(buffer, (int)b->x, (int)b->y, b->radius, COL_GRAY);

    /* Indicateur d'arme cachée */
    if (b->has_weapon)
        textout_centre_ex(buffer, font, "W", (int)b->x, (int)b->y - 5,
                          COL_BLACK, -1);
    /* Indicateur éclair */
    if (b->shoots_lightning)
        circle(buffer, (int)b->x, (int)b->y, b->radius + 3, COL_YELLOW);
}

void render_projectile(const Projectile *pr)
{
    if (!pr->active) return;
    int col = (pr->type == WEAPON_BOMB) ? COL_ORANGE : COL_WHITE;
    rectfill(buffer, (int)pr->x - 2, (int)pr->y - 8,
             (int)pr->x + 2, (int)pr->y, col);
}

void render_weapon_pickup(const WeaponPickup *w)
{
    if (!w->active) return;
    int col = (w->type == WEAPON_FAST)   ? COL_GREEN  :
              (w->type == WEAPON_DOUBLE) ? COL_ORANGE :
                                           COL_RED;
    rectfill(buffer, (int)w->x - 12, (int)w->y - 12,
             (int)w->x + 12, (int)w->y + 12, col);
    textout_centre_ex(buffer, font, "ARM",
                      (int)w->x, (int)w->y - 5, COL_WHITE, -1);
}

void render_lightning(const Lightning *l)
{
    if (!l->active) return;
    /* Eclair : ligne jaune verticale */
    vline(buffer, (int)l->x, (int)l->y - 20, (int)l->y, COL_YELLOW);
    rectfill(buffer, (int)l->x - 2, (int)l->y - 5,
             (int)l->x + 2, (int)l->y + 5, COL_WHITE);
}

void render_boss(const Boss *boss)
{
    if (!boss->active) return;
    int x = (int)boss->x, y = (int)boss->y;
    int w = boss->width,   h = boss->height;

    /* Corps du boss */
    rectfill(buffer, x - w/2, y - h/2, x + w/2, y + h/2, makecol(232, 180, 204));
    rect(buffer,   x - w/2, y - h/2, x + w/2, y + h/2, makecol(122, 26, 58));
    circlefill(buffer, x - w/2, y - h/2, 15, makecol(255, 200, 178));
    circlefill(buffer, x + w/2, y - h/2, 15, makecol(255, 200, 178));

    /* Barre de vie */
    int bar_w = 100;
    int hp_w  = (boss->hp * bar_w) / boss->max_hp;
    rect(buffer, x - bar_w/2, y - h/2 - 20,
         x + bar_w/2, y - h/2 - 10, COL_WHITE);
    rectfill(buffer, x - bar_w/2, y - h/2 - 20,
             x - bar_w/2 + hp_w, y - h/2 - 10, COL_RED);
}

/* ============================================================
 * HUD
 * ============================================================ */

void render_hud(const Game *g)
{
    char buf[64];
    int y0 = GAME_ZONE_H;

    /* Fond HUD */
    rectfill(buffer, 0, y0, WIN_W, WIN_H, COL_HUD_BG);
    hline(buffer, 0, y0, WIN_W, COL_GRAY);

    /* Pseudo */
    snprintf(buf, sizeof(buf), "Joueur : %s", g->player.pseudo);
    draw_text_scaled_left(buf, 10, y0 + 8, COL_WHITE, 2);

    /* Score */
    snprintf(buf, sizeof(buf), "Score : %d", g->score);
    draw_text_scaled(buf, WIN_W/2, y0 + 8, COL_YELLOW, 2);

    /* Temps */
    snprintf(buf, sizeof(buf), "Temps : %d", g->current_level.time_left);
    draw_text_scaled_left(buf, WIN_W - 180, y0 + 8, COL_CYAN, 2);

    /* Niveau */
    snprintf(buf, sizeof(buf), "Niv. %d", g->level_number);
    draw_text_scaled_left(buf, WIN_W - 120, y0 + 35, COL_GRAY, 2);
}

/* ============================================================
 * RENDU COMPLET DU JEU
 * ============================================================ */

void render_game(const Game *g)
{
    int i;
    const Level *lv = &g->current_level;

    /* Fond de la zone de jeu */
    rectfill(buffer, 0, 0, WIN_W, GAME_ZONE_H,
             makecol(220, 210, 240));

    /* Bordures */
    rect(buffer, 0, 0, WIN_W - 1, GAME_ZONE_H, COL_GRAY);

    /* Entités */
    for (i = 0; i < MAX_BUBBLES;     i++) render_bubble(&lv->bubbles[i]);
    for (i = 0; i < MAX_PROJECTILES; i++) render_projectile(&lv->projectiles[i]);
    for (i = 0; i < MAX_WEAPONS;     i++) render_weapon_pickup(&lv->weapons[i]);
    for (i = 0; i < MAX_WEAPONS;     i++) render_lightning(&lv->lightnings[i]);

    render_boss(&lv->boss);
    render_player(&g->player);
    render_hud(g);
}

/* ============================================================
 * ECRANS DE FIN
 * ============================================================ */

void render_level_win(int score)
{
    char buf[64];
    clear_to_color(buffer, COL_BG);
    draw_text_scaled("NIVEAU REUSSI !", WIN_W/2, 260, COL_GREEN, 3);
    snprintf(buf, sizeof(buf), "Score : %d", score);
    draw_text_scaled(buf, WIN_W/2, 330, COL_YELLOW, 2);
    draw_text_scaled("ENTREE : continuer   S : sauvegarder   ECHAP : menu",
        WIN_W/2, 410, COL_GRAY, 2);
}

void render_level_fail(void)
{
    clear_to_color(buffer, makecol(255, 220, 225));
    draw_text_scaled("NIVEAU ECHOUE", WIN_W/2, 280, COL_RED, 3);
    draw_text_scaled("ENTREE : recommencer   ECHAP : menu",
        WIN_W/2, 370, COL_GRAY, 2);
}

void render_game_win(int score)
{
    char buf[64];
    clear_to_color(buffer, makecol(210, 245, 225));
    draw_text_scaled("VICTOIRE !", WIN_W/2, 220, COL_GREEN, 4);
    draw_text_scaled("Vous avez vaincu le Boss et sauve le monde !", WIN_W/2, 310, COL_WHITE, 2);
    snprintf(buf, sizeof(buf), "Score final : %d", score);
    draw_text_scaled(buf, WIN_W/2, 370, COL_YELLOW, 2);
    draw_text_scaled("Appuyez sur ENTREE pour revenir au menu", WIN_W/2, 440, COL_GRAY, 2);
}

void render_game_over(int score)
{
    char buf[64];
    clear_to_color(buffer, makecol(255, 215, 220));
    draw_text_scaled("GAME OVER", WIN_W/2, 260, COL_RED, 4);
    snprintf(buf, sizeof(buf), "Score final : %d", score);
    draw_text_scaled(buf, WIN_W/2, 360, COL_YELLOW, 2);
    draw_text_scaled("Appuyez sur ENTREE pour revenir au menu", WIN_W/2, 430, COL_GRAY, 2);
}

void render_save_screen(void)
{
    clear_to_color(buffer, COL_BG);
    draw_text_scaled("Partie sauvegardee !", WIN_W/2, 320, COL_GREEN, 2);
    draw_text_scaled("Appuyez sur une touche...", WIN_W/2, 390, COL_GRAY, 2);
}

/* ============================================================
 * ANIMATIONS
 * ============================================================ */

void render_explosion(float x, float y, int frame)
{
    int r = frame * 5;
    if (r > 60) return;
    circle(buffer, (int)x, (int)y, r,   makecol(255, 179, 179));
    circle(buffer, (int)x, (int)y, r/2, makecol(255, 221, 170));
}

void render_victory_animation(int frame)
{
    int i;
    int confetti_cols[5];
    confetti_cols[0] = makecol(247, 214, 240);
    confetti_cols[1] = makecol(200, 240, 220);
    confetti_cols[2] = makecol(184, 240, 244);
    confetti_cols[3] = makecol(255, 221, 170);
    confetti_cols[4] = makecol(255, 179, 179);
    for (i = 0; i < 10; i++) {
        int x = (frame * 13 + i * 80) % WIN_W;
        int y = (frame * 7  + i * 60) % GAME_ZONE_H;
        circlefill(buffer, x, y, 10, confetti_cols[i % 5]);
    }
}

/* ============================================================
 * DOUBLE BUFFER
 * ============================================================ */

void render_flip(void)
{
    blit(buffer, screen, 0, 0, 0, 0, WIN_W, WIN_H);
}
