#ifndef LEVELS_H
#define LEVELS_H

/* Map tile definitions. These are used in the levels/level-*.txt files
 * to make up the levels for the game. */
#define TILE_FLOOR	'.'
#define TILE_WALL	'w'
#define TILE_DOOR	'd'
#define TILE_EXIT	'e'
#define TILE_GOODIE	'g'
#define TILE_UNWALKABLE	'x'

/* clear_level: Clear level array from data without regenerating.
 */
void clear_level(game_data *game);

/* generate_level: Generate random level.
 */
void generate_level(game_data *game);

/* open_exit: Clears the exit door on the right of the level once
 *            certain conditions have been met.
 */
void open_exit(game_data *game);

/* set_goodies: Place all goodies in random locations within the level.
 */
void set_goodies(game_data *game);

/* set_player: Set position for our player in the level entrance.
 */
void set_player(game_data *game);

/* set_zombies: Place zombies in random locations within the level.
 */
void set_zombies(game_data *game);

#endif
