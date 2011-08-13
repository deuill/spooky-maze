#ifndef INPUT_H
#define INPUT_H

#define JOYSTICK_UP    12 /* These mappings may or may not work with */
#define JOYSTICK_DOWN  14 /* other joysticks or when ported to other */
#define JOYSTICK_LEFT  15 /* devices. */
#define JOYSTICK_RIGHT 13

/* handle_input: Reads events from the keyboard and modifies data in
 *               'game' accordingly.
 */
void handle_input(game_data *game);

#endif
