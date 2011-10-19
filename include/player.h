#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_SPEED (TILE_SIZE * 4) /* Movement speed for our player in pixels per second. */

#define PLAYER_X (game->player.x / TILE_SIZE) /* Current player position in */
#define PLAYER_Y (game->player.y / TILE_SIZE) /* relation to the 'level' array. */

/* move_player: Moves player and handles collision.
 */
void move_player(game_data *game);

/* detect_collision: Finds if 'player' intersects 'wall' by checking if
 *                   'wall.x <= player.x <= wall.x + wall.w' and so on
 *                   for each of the four points in the 'player' rect.
 */
int detect_collision(SDL_Rect player, SDL_Rect wall);

/* set_camera: Sets camera position in relation to our player and calls
 *             'draw_level()' to redraw the level around the player.
 */
void set_camera(game_data *game);

#endif
