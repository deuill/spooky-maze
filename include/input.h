#ifndef INPUT_H
#define INPUT_H

/* These mappings may or may not work with other */
/* joysticks or when ported to other devices.*/
#define JOYSTICK_UP    12
#define JOYSTICK_DOWN  14
#define JOYSTICK_LEFT  15
#define JOYSTICK_RIGHT 13

/* 
 * Reads events from the keyboard and modifies data in 'game' accordingly.
 */
void input_handle(struct game_data *game);

#endif
