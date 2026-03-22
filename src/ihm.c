/* ============================================================
 * ihm.c - Gestion des entrées utilisateur (Allegro 4)
 * Découplé de la logique métier.
 * ============================================================ */

#include <allegro.h>
#include <string.h>
#include <stdio.h>
#include "ihm.h"

void ihm_init(void)
{
    /* install_keyboard() déjà appelé dans render_init */
    clear_keybuf();
}

void ihm_poll(InputState *state)
{
    state->move_left  = key[KEY_LEFT];
    state->move_right = key[KEY_RIGHT];
    state->shoot      = key[KEY_SPACE];
    state->pause      = key[KEY_P];
    state->up         = key[KEY_UP];
    state->down       = key[KEY_DOWN];
    state->enter      = key[KEY_ENTER];
    state->escape     = key[KEY_ESC];
    state->key_any    = keypressed();
    state->text_input = 0;
}

void ihm_read_pseudo(char *buf, int max_len)
{
    int len = 0;
    int c;
    buf[0] = '\0';
    clear_keybuf();

    while (1) {
        if (keypressed()) {
            c = readkey();
            int ascii = c & 0xFF;
            int scan  = (c >> 8) & 0xFF;

            if (scan == KEY_ENTER && len > 0) {
                buf[len] = '\0';
                break;
            } else if (scan == KEY_BACKSPACE && len > 0) {
                len--;
                buf[len] = '\0';
            } else if (ascii >= 32 && ascii < 127 && len < max_len - 1) {
                buf[len++] = (char)ascii;
                buf[len]   = '\0';
            }
        }
        rest(10);
    }
}

int ihm_menu_navigate(int current, int nb_items, const InputState *state)
{
    static int prev_up   = 0;
    static int prev_down = 0;

    if (state->up && !prev_up) {
        current = (current - 1 + nb_items) % nb_items;
    }
    if (state->down && !prev_down) {
        current = (current + 1) % nb_items;
    }

    prev_up   = state->up;
    prev_down = state->down;
    return current;
}

int ihm_menu_confirm(const InputState *state)
{
    static int prev_enter = 0;
    int result = (state->enter && !prev_enter);
    prev_enter = state->enter;
    return result;
}
