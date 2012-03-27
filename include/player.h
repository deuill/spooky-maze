#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_SPEED 260 /* Movement speed for our player in pixels per second. */

#define PLAYER_X (game->player.rect.x / TILE_SIZE) /* Current player position in */
#define PLAYER_Y (game->player.rect.y / TILE_SIZE) /* relation to the 'level' array. */

/* 
 * Moves player and checks for collision using 'level_collision()'.
 */
void player_move(struct game_data *game);

/* 
 * Centers 'game.camera' over our player with respect to the level boundaries.
 */
void player_camera_follow(struct game_data *game);

#endif
