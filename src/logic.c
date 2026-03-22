/* ============================================================
 * logic.c - Logique métier du jeu Super Bulles
 * Aucune dépendance à Allegro.
 * ============================================================ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "logic.h"

/* ============================================================
 * INITIALISATION
 * ============================================================ */

void logic_init_player(Player *p, const char *pseudo)
{
    strncpy(p->pseudo, pseudo, MAX_PSEUDO - 1);
    p->pseudo[MAX_PSEUDO - 1] = '\0';
    p->x       = WIN_W / 2.0f;
    p->y       = GAME_ZONE_H - 30;
    p->width   = 30;
    p->height  = 50;
    p->speed   = PLAYER_SPEED;
    p->alive   = 1;
    p->weapon  = WEAPON_SIMPLE;
    p->weapon_timer = 0;
    p->direction    = 1;
}

static int bubble_radius(BubbleSize size)
{
    switch (size) {
        case BUBBLE_BIG:    return BUBBLE_SIZE_BIG;
        case BUBBLE_MEDIUM: return BUBBLE_SIZE_MEDIUM;
        case BUBBLE_SMALL:  return BUBBLE_SIZE_SMALL;
    }
    return BUBBLE_SIZE_SMALL;
}

void logic_init_bubble(Bubble *b, float x, float y, BubbleSize size,
                       float vx, float vy, int shoots_lightning)
{
    b->x      = x;
    b->y      = y;
    b->vx     = vx;
    b->vy     = vy;
    b->size   = size;
    b->radius = bubble_radius(size);
    b->active = 1;
    b->has_weapon       = 0;
    b->weapon_type      = WEAPON_SIMPLE;
    b->shoots_lightning = shoots_lightning;
    b->lightning_timer  = 120 + rand() % 180; /* 2-5 secondes à 60fps */
}

void logic_init_boss(Boss *boss)
{
    boss->x      = WIN_W / 2.0f;
    boss->y      = 80.0f;
    boss->width  = 80;
    boss->height = 80;
    boss->vx     = 2.0f;
    boss->hp     = 10;
    boss->max_hp = 10;
    boss->active = 1;
    boss->bubble_timer = 180; /* lance une bulle toutes les 3 secondes */
}

/* Configuration des niveaux */
static void setup_level(Level *lv, int number)
{
    int i;
    lv->number       = number;
    lv->time_left    = LEVEL_TIME;
    lv->timer_frames = 0;
    lv->countdown    = 3;
    lv->countdown_frames = 0;
    lv->nb_projectiles   = 0;

    /* Désactiver tous les projectiles, armes, éclairs */
    for (i = 0; i < MAX_PROJECTILES; i++) lv->projectiles[i].active = 0;
    for (i = 0; i < MAX_WEAPONS;     i++) lv->weapons[i].active     = 0;
    for (i = 0; i < MAX_WEAPONS;     i++) lv->lightnings[i].active  = 0;

    /* Désactiver toutes les bulles */
    for (i = 0; i < MAX_BUBBLES; i++) lv->bubbles[i].active = 0;

    lv->boss.active = 0;

    /* Placer les bulles selon le niveau */
    switch (number) {
        case 1:
            /* Niveau 1 : 2 grosses bulles simples */
            logic_init_bubble(&lv->bubbles[0], 200, 150, BUBBLE_BIG,  2.0f, 0.0f, 0);
            logic_init_bubble(&lv->bubbles[1], 600, 200, BUBBLE_BIG, -2.0f, 0.0f, 0);
            lv->nb_bubbles_init = 2;
            break;

        case 2:
            /* Niveau 2 : 2 grosses + 1 moyenne, armes cachées */
            logic_init_bubble(&lv->bubbles[0], 150, 120, BUBBLE_BIG,   2.5f, 0.0f, 0);
            logic_init_bubble(&lv->bubbles[1], 650, 120, BUBBLE_BIG,  -2.5f, 0.0f, 0);
            logic_init_bubble(&lv->bubbles[2], 400, 200, BUBBLE_MEDIUM, 1.5f, 0.0f, 0);
            lv->bubbles[1].has_weapon  = 1;
            lv->bubbles[1].weapon_type = WEAPON_FAST;
            lv->nb_bubbles_init = 3;
            break;

        case 3:
            /* Niveau 3 : éclairs actifs */
            logic_init_bubble(&lv->bubbles[0], 200, 100, BUBBLE_BIG,   3.0f, 0.0f, 1);
            logic_init_bubble(&lv->bubbles[1], 600, 100, BUBBLE_BIG,  -3.0f, 0.0f, 1);
            logic_init_bubble(&lv->bubbles[2], 400, 150, BUBBLE_MEDIUM, 2.0f, 0.0f, 0);
            logic_init_bubble(&lv->bubbles[3], 300, 200, BUBBLE_SMALL,  1.0f, 2.0f, 0);
            lv->bubbles[0].has_weapon  = 1;
            lv->bubbles[0].weapon_type = WEAPON_DOUBLE;
            lv->nb_bubbles_init = 4;
            break;

        case 4:
            /* Niveau 4 : Boss */
            logic_init_bubble(&lv->bubbles[0], 200, 180, BUBBLE_MEDIUM, 2.0f, 0.0f, 1);
            logic_init_bubble(&lv->bubbles[1], 600, 180, BUBBLE_MEDIUM,-2.0f, 0.0f, 1);
            lv->nb_bubbles_init = 2;
            logic_init_boss(&lv->boss);
            break;

        default:
            break;
    }
}

void logic_init_level(Game *g, int level_number)
{
    g->level_number = level_number;
    setup_level(&g->current_level, level_number);
    /* Réinitialiser le joueur en position de départ */
    g->current_level.countdown = 3;
    g->player.x = WIN_W / 2.0f;
    g->player.y = GAME_ZONE_H - 30;
    g->player.alive  = 1;
    g->player.weapon = WEAPON_SIMPLE;
    g->player.weapon_timer = 0;
}

void logic_init_game(Game *g, const char *pseudo)
{
    g->score = 0;
    g->nb_bubbles_destroyed = 0;
    logic_init_player(&g->player, pseudo);
    logic_init_level(g, 1);
}

/* ============================================================
 * MISES À JOUR
 * ============================================================ */

void logic_update_player(Game *g, int move_left, int move_right)
{
    Player *p = &g->player;
    if (!p->alive) return;

    if (move_left) {
        p->x -= p->speed;
        p->direction = -1;
        if (p->x - p->width/2 < 0) p->x = p->width/2;
    }
    if (move_right) {
        p->x += p->speed;
        p->direction = 1;
        if (p->x + p->width/2 > WIN_W) p->x = WIN_W - p->width/2;
    }

    /* Décompte arme spéciale */
    if (p->weapon != WEAPON_SIMPLE && p->weapon_timer > 0) {
        p->weapon_timer--;
        if (p->weapon_timer == 0) p->weapon = WEAPON_SIMPLE;
    }
}

void logic_update_bubbles(Level *lv)
{
    int i;
    for (i = 0; i < MAX_BUBBLES; i++) {
        Bubble *b = &lv->bubbles[i];
        if (!b->active) continue;

        /* Gravité */
        b->vy += GRAVITY;

        b->x += b->vx;
        b->y += b->vy;

        /* Rebond sur les murs latéraux */
        if (b->x - b->radius < 0) {
            b->x  = b->radius;
            b->vx = fabsf(b->vx);
        }
        if (b->x + b->radius > WIN_W) {
            b->x  = WIN_W - b->radius;
            b->vx = -fabsf(b->vx);
        }

        /* Rebond sur le sol */
        if (b->y + b->radius >= GAME_ZONE_H) {
            b->y  = GAME_ZONE_H - b->radius;
            b->vy = -fabsf(b->vy) * BOUNCE_DAMPING;
            /* Vitesse minimale pour ne pas s'immobiliser */
            if (fabsf(b->vy) < 3.0f) b->vy = -4.0f - b->size * 1.5f;
        }

        /* Rebond sur le plafond */
        if (b->y - b->radius < 0) {
            b->y  = b->radius;
            b->vy = fabsf(b->vy);
        }

        /* Gestion éclairs (niveau 3+) */
        if (b->shoots_lightning && b->lightning_timer > 0) {
            b->lightning_timer--;
            if (b->lightning_timer == 0) {
                /* Chercher un slot libre pour l'éclair */
                int j;
                for (j = 0; j < MAX_WEAPONS; j++) {
                    if (!lv->lightnings[j].active) {
                        lv->lightnings[j].x      = b->x;
                        lv->lightnings[j].y      = b->y;
                        lv->lightnings[j].speed  = 6.0f;
                        lv->lightnings[j].active = 1;
                        break;
                    }
                }
                b->lightning_timer = 120 + rand() % 180;
            }
        }
    }
}

void logic_update_projectiles(Level *lv)
{
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *pr = &lv->projectiles[i];
        if (!pr->active) continue;

        pr->y += pr->vy;

        /* Sortie de l'écran par le haut */
        if (pr->y < 0) {
            pr->active = 0;
            if (lv->nb_projectiles > 0) lv->nb_projectiles--;
        }
    }
}

void logic_update_weapons_on_ground(Level *lv)
{
    int i;
    for (i = 0; i < MAX_WEAPONS; i++) {
        WeaponPickup *w = &lv->weapons[i];
        if (!w->active) continue;
        w->lifetime--;
        if (w->lifetime <= 0) w->active = 0;
    }
}

void logic_update_lightnings(Level *lv, Player *p)
{
    int i;
    for (i = 0; i < MAX_WEAPONS; i++) {
        Lightning *l = &lv->lightnings[i];
        if (!l->active) continue;
        l->y += l->speed;
        if (l->y > GAME_ZONE_H) {
            l->active = 0;
            continue;
        }
        /* Collision avec le joueur */
        if (p->alive) {
            float dx = p->x - l->x;
            float dy = (p->y - p->height/2) - l->y;
            if (fabsf(dx) < p->width/2 && fabsf(dy) < p->height) {
                p->alive  = 0;
                l->active = 0;
            }
        }
    }
}

void logic_update_boss(Game *g)
{
    Level *lv  = &g->current_level;
    Boss  *boss = &lv->boss;
    if (!boss->active) return;

    /* Déplacement horizontal, accélération proportionnelle aux dégâts */
    float speed_mult = 1.0f + (float)(boss->max_hp - boss->hp) * 0.2f;
    boss->x += boss->vx * speed_mult;

    if (boss->x - boss->width/2 < 0) {
        boss->x  = boss->width/2;
        boss->vx = fabsf(boss->vx);
    }
    if (boss->x + boss->width/2 > WIN_W) {
        boss->x  = WIN_W - boss->width/2;
        boss->vx = -fabsf(boss->vx);
    }

    /* Lance une bulle périodiquement */
    boss->bubble_timer--;
    if (boss->bubble_timer <= 0) {
        int i;
        for (i = 0; i < MAX_BUBBLES; i++) {
            if (!lv->bubbles[i].active) {
                logic_init_bubble(&lv->bubbles[i],
                    boss->x, boss->y + boss->height/2,
                    BUBBLE_SMALL,
                    (rand() % 3 - 1) * 2.0f, 0.0f, 0);
                break;
            }
        }
        boss->bubble_timer = 180;
    }

    /* Collision boss / joueur */
    Player *p = &g->player;
    if (p->alive) {
        float dx = fabsf(p->x - boss->x);
        float dy = fabsf(p->y - boss->y);
        if (dx < (p->width/2 + boss->width/2) &&
            dy < (p->height/2 + boss->height/2)) {
            p->alive = 0;
        }
    }
}

void logic_update_timer(Level *lv)
{
    if (lv->time_left <= 0) return;
    lv->timer_frames++;
    if (lv->timer_frames >= 60) { /* 60 fps */
        lv->timer_frames = 0;
        lv->time_left--;
    }
}

void logic_update(Game *g)
{
    if (!g->player.alive) return;

    logic_update_bubbles(&g->current_level);
    logic_update_projectiles(&g->current_level);
    logic_update_weapons_on_ground(&g->current_level);
    logic_update_lightnings(&g->current_level, &g->player);
    if (g->current_level.boss.active) logic_update_boss(g);
    logic_check_collisions(g);
    logic_update_timer(&g->current_level);
}

/* ============================================================
 * ACTIONS JOUEUR
 * ============================================================ */

void logic_player_shoot(Game *g)
{
    Level  *lv = &g->current_level;
    Player *p  = &g->player;
    if (!p->alive) return;

    /* Arme simple : 1 seul projectile à la fois */
    if (p->weapon == WEAPON_SIMPLE) {
        int i;
        int count = 0;
        for (i = 0; i < MAX_PROJECTILES; i++)
            if (lv->projectiles[i].active) count++;
        if (count >= 1) return; /* déjà un tir en cours */
    }

    /* Chercher un slot libre */
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *pr = &lv->projectiles[i];
        if (!pr->active) {
            pr->x      = p->x;
            pr->y      = p->y - p->height;
            pr->vx     = 0.0f;
            pr->vy     = -BULLET_SPEED;
            pr->active = 1;
            pr->type   = p->weapon;
            lv->nb_projectiles++;

            /* Arme double : 2ème projectile légèrement décalé */
            if (p->weapon == WEAPON_DOUBLE && i + 1 < MAX_PROJECTILES) {
                Projectile *pr2 = &lv->projectiles[i + 1];
                if (!pr2->active) {
                    pr2->x      = p->x + 10;
                    pr2->y      = p->y - p->height;
                    pr2->vx     = 0.0f;
                    pr2->vy     = -BULLET_SPEED;
                    pr2->active = 1;
                    pr2->type   = p->weapon;
                    lv->nb_projectiles++;
                }
            }
            break;
        }
    }
}

void logic_player_pick_weapon(Game *g)
{
    Level  *lv = &g->current_level;
    Player *p  = &g->player;
    int i;
    for (i = 0; i < MAX_WEAPONS; i++) {
        WeaponPickup *w = &lv->weapons[i];
        if (!w->active) continue;
        float dx = fabsf(p->x - w->x);
        float dy = fabsf(p->y - w->y);
        if (dx < p->width && dy < p->height) {
            p->weapon       = w->type;
            p->weapon_timer = 600; /* 10 secondes à 60fps */
            w->active       = 0;
        }
    }
}

/* ============================================================
 * COLLISIONS
 * ============================================================ */

/* Collision cercle / rectangle */
static int circle_rect_collision(float cx, float cy, float r,
                                  float rx, float ry, float rw, float rh)
{
    float nearX = cx < rx ? rx : (cx > rx + rw ? rx + rw : cx);
    float nearY = cy < ry ? ry : (cy > ry + rh ? ry + rh : cy);
    float dx = cx - nearX;
    float dy = cy - nearY;
    return (dx*dx + dy*dy) < (r*r);
}

void logic_bubble_hit(Game *g, int bubble_idx, int proj_idx)
{
    Level  *lv = &g->current_level;
    Bubble *b  = &lv->bubbles[bubble_idx];
    Projectile *pr = &lv->projectiles[proj_idx];

    /* Score */
    logic_add_score(g, logic_bubble_points(b->size));
    g->nb_bubbles_destroyed++;

    /* Faire tomber l'arme si la bulle en contient une */
    if (b->has_weapon) {
        int i;
        for (i = 0; i < MAX_WEAPONS; i++) {
            if (!lv->weapons[i].active) {
                lv->weapons[i].x        = b->x;
                lv->weapons[i].y        = GAME_ZONE_H - 20;
                lv->weapons[i].type     = b->weapon_type;
                lv->weapons[i].active   = 1;
                lv->weapons[i].lifetime = 600;
                break;
            }
        }
    }

    /* Division */
    logic_split_bubble(lv, b);

    /* Désactiver bulle et projectile */
    b->active  = 0;
    pr->active = 0;
    if (lv->nb_projectiles > 0) lv->nb_projectiles--;
}

void logic_split_bubble(Level *lv, Bubble *b)
{
    if (b->size == BUBBLE_SMALL) return; /* taille minimale : disparaît */

    BubbleSize next_size = (b->size == BUBBLE_BIG) ? BUBBLE_MEDIUM : BUBBLE_SMALL;
    int found = 0, i;

    for (i = 0; i < MAX_BUBBLES && found < 2; i++) {
        if (!lv->bubbles[i].active) {
            float new_vx = (found == 0) ? fabsf(b->vx) + 1.0f : -(fabsf(b->vx) + 1.0f);
            float new_vy = -fabsf(b->vy) - 1.0f;
            logic_init_bubble(&lv->bubbles[i], b->x, b->y,
                              next_size, new_vx, new_vy, b->shoots_lightning);
            found++;
        }
    }
}

void logic_check_collisions(Game *g)
{
    Level  *lv = &g->current_level;
    Player *p  = &g->player;
    int i, j;

    /* Projectile / bulle */
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *pr = &lv->projectiles[i];
        if (!pr->active) continue;
        for (j = 0; j < MAX_BUBBLES; j++) {
            Bubble *b = &lv->bubbles[j];
            if (!b->active) continue;
            float dx = pr->x - b->x;
            float dy = pr->y - b->y;
            if (dx*dx + dy*dy < (float)(b->radius * b->radius)) {
                logic_bubble_hit(g, j, i);
                break;
            }
        }
    }

    /* Bulle / joueur */
    if (p->alive) {
        for (j = 0; j < MAX_BUBBLES; j++) {
            Bubble *b = &lv->bubbles[j];
            if (!b->active) continue;
            if (circle_rect_collision(b->x, b->y, b->radius,
                    p->x - p->width/2, p->y - p->height,
                    p->width, p->height)) {
                p->alive = 0;
                break;
            }
        }
    }

    /* Projectile / boss */
    if (lv->boss.active) {
        Boss *boss = &lv->boss;
        for (i = 0; i < MAX_PROJECTILES; i++) {
            Projectile *pr = &lv->projectiles[i];
            if (!pr->active) continue;
            if (pr->x > boss->x - boss->width/2 &&
                pr->x < boss->x + boss->width/2 &&
                pr->y > boss->y - boss->height/2 &&
                pr->y < boss->y + boss->height/2) {
                boss->hp--;
                pr->active = 0;
                if (lv->nb_projectiles > 0) lv->nb_projectiles--;
                logic_add_score(g, 50);
                if (boss->hp <= 0) boss->active = 0;
                break;
            }
        }
    }

    /* Ramasser arme */
    logic_player_pick_weapon(g);
}

/* ============================================================
 * CONDITIONS DE FIN
 * ============================================================ */

int logic_all_bubbles_dead(const Level *lv)
{
    int i;
    for (i = 0; i < MAX_BUBBLES; i++)
        if (lv->bubbles[i].active) return 0;
    return 1;
}

int logic_is_level_won(const Game *g)
{
    const Level *lv = &g->current_level;
    if (!logic_all_bubbles_dead(lv)) return 0;
    if (lv->number == NB_LEVELS && lv->boss.active) return 0;
    return 1;
}

int logic_is_level_lost(const Game *g)
{
    if (!g->player.alive)                    return 1;
    if (g->current_level.time_left <= 0)     return 1;
    return 0;
}

int logic_is_game_won(const Game *g)
{
    return (g->level_number == NB_LEVELS && logic_is_level_won(g));
}

/* ============================================================
 * SCORE
 * ============================================================ */

int logic_bubble_points(BubbleSize size)
{
    switch (size) {
        case BUBBLE_BIG:    return 100;
        case BUBBLE_MEDIUM: return 200;
        case BUBBLE_SMALL:  return 400;
    }
    return 0;
}

void logic_add_score(Game *g, int points)
{
    g->score += points;
}

/* ============================================================
 * SAUVEGARDE / CHARGEMENT
 * ============================================================ */

int logic_save_game(const Game *g, const char *filename)
{
    FILE *f = fopen(filename, "a");
    if (!f) return 0;
    fprintf(f, "%s %d %d\n", g->player.pseudo, g->level_number, g->score);
    fclose(f);
    return 1;
}

int logic_load_game(Game *g, const char *pseudo, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) return 0;

    char   saved_pseudo[MAX_PSEUDO];
    int    saved_level, saved_score;
    int    found = 0;

    while (fscanf(f, "%31s %d %d", saved_pseudo, &saved_level, &saved_score) == 3) {
        if (strcmp(saved_pseudo, pseudo) == 0) {
            strncpy(g->player.pseudo, pseudo, MAX_PSEUDO - 1);
            g->level_number = saved_level;
            g->score        = saved_score;
            found = 1;
            /* on garde la dernière entrée pour ce pseudo */
        }
    }
    fclose(f);
    return found;
}
