#ifndef IHM_H
#define IHM_H

/* ============================================================
 * ihm.h - Interface de gestion des entrées utilisateur
 * Séparée de la logique métier et du rendu.
 * ============================================================ */

#include "game.h"

/* --- Structure d'état des entrées --- */
typedef struct {
    int move_left;
    int move_right;
    int shoot;
    int pause;
    int up;
    int down;
    int enter;
    int escape;
    int key_any;       /* une touche quelconque a été pressée */
    char text_input;   /* caractère saisi (pour pseudo) */
} InputState;

/* --- Initialisation --- */
void ihm_init(void);

/* --- Lecture des entrées (appelée chaque frame) --- */
void ihm_poll(InputState *state);

/* --- Saisie du pseudo (bloquant) --- */
void ihm_read_pseudo(char *buf, int max_len);

/* --- Navigation menu --- */
int ihm_menu_navigate(int current, int nb_items, const InputState *state);
int ihm_menu_confirm(const InputState *state);

#endif /* IHM_H */
