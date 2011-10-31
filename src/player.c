#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#include "game.h"
#include "graphics.h"
#include "input.h"
#include "levels.h"
#include "player.h"

int detect_collision(SDL_Rect player, SDL_Rect wall)
{
	/* Check top-left corner of the 'player' rectangle. */
	if ((wall.x <= player.x && player.x <= wall.x + wall.w) &&
	    (wall.y <= player.y && player.y <= wall.y + wall.h))
		return true;
	/* Check top-right corner of the 'player' rectangle. */
	if ((wall.x <= player.x + player.w && player.x + player.w <= wall.x + wall.w) &&
	    (wall.y <= player.y && player.y <= wall.y + wall.h))
		return true;
	/* Check bottom-left corner of the 'player' rectangle. */
	if ((wall.x <= player.x && player.x <= wall.x + wall.w) &&
	    (wall.y <= player.y + player.h && player.y + player.h <= wall.y + wall.h))
		return true;
	/* Check bottom-right corner of the 'player' rectangle. */
	if ((wall.x <= player.x + player.w && player.x + player.w <= wall.x + wall.w) &&
	    (wall.y <= player.y + player.h && player.y + player.h <= wall.y + wall.h))
		return true;
	/* Check center of the 'player' rectangle. */
	if ((wall.x <= player.x + (player.w / 2) && player.x + (player.w / 2) <= wall.x + wall.w) &&
	    (wall.y <= player.y + (player.h / 2) && player.y + (player.h / 2) <= wall.y + wall.h))
		return true;
	/* No collision detected. */
	return false;
}

void move_player(game_data *game)
{
	SDL_Rect tmp;			/* Used for collision detection. */
	int move_x, move_y;		/* Used for holding our direction temporarily. */
	int x, y, i, position = 1;	/* Position of wall relative to player. */

	/* Protect against incorrect delta-time readings */
	if (game->delta_time > 100)
		return;

	/* Scale our speed depending on the frame-rate */
	move_x = (int) (game->dir_x * ((float) game->delta_time / 1000.0f));
	move_y = (int) (game->dir_y * ((float) game->delta_time / 1000.0f));

	for (y = PLAYER_Y - 1; y <= PLAYER_Y + 1; y++)
	for (x = PLAYER_X - 1; x <= PLAYER_X + 1; x++, position++)
		switch (game->level[y][x]) {
		case TILE_EXIT:
			/* You have cleared this stage, congratulations! */
			if (detect_collision(game->player, game->wall[y][x])) {
				clear_entity(game, game->player);
				game->level_cleared = true;
			}
			break;
		case TILE_GOODIE:
			/* Find which goodie in the 'goodies' array we're colliding with. */
			for (i = 0; i < game->num_goodies; i++)
				if ((game->goodie[i].x / TILE_SIZE == x) && \
				    (game->goodie[i].y / TILE_SIZE == y))
					break;
			/* Once we collide with the goodie, clear the goodie, rearrange
			 * the goodies array and reduce the number of goodies in the level. */
			if (detect_collision(game->player, game->goodie[i])) {
				clear_entity(game, game->goodie[i]);
				game->goodie[i] = game->goodie[game->num_goodies - 1];
				game->level[y][x] = TILE_FLOOR;
				game->num_goodies--;
				game->score += 100;
				/* Give us 1 life every 10000 score. */
				if (game->score / game->score_scale == 1) {
					game->lives += game->score / game->score_scale;
					game->score_scale += 10000;
				}
				/* If we have removed all goodies, open the door. */
				if (game->num_goodies == 0)
					open_exit(game);
			}
			break;
		case TILE_DOOR: /* Treat the closed door as a wall and fall through. */
		case TILE_WALL:
			tmp = game->player;
			if (move_x < 0) {
				switch (position) {
				case 1: /* Do not move through walls to the top-left diagonally. */
					tmp.x += move_x, tmp.y += move_y;
					if ((detect_collision(tmp, game->wall[y][x])) &&
					    (game->level[y + 1][x] != TILE_WALL) && (move_y < 0))
						move_y = (game->wall[y][x].y + game->wall[y][x].h) - game->player.y;
					break;
				case 4: /* Do not move through walls to the left. */
					tmp.x += move_x;
					if (detect_collision(tmp, game->wall[y][x]))
						move_x = (game->wall[y][x].x + game->wall[y][x].w) - game->player.x;
					break;
				case 7: /* Do not move through walls to the bottom-left from the right. */
					tmp.x += move_x;
					if ((detect_collision(tmp, game->wall[y][x])) &&
					    (game->wall[y][x].y < tmp.y + tmp.h))
						move_x = (game->wall[y][x].x + game->wall[y][x].w) - game->player.x;
					break;
				}
			} else if (move_x > 0) {
				switch (position) {
				case 3: /* Do not move through walls to the top-right diagonally. */
					tmp.x += move_x, tmp.y += move_y;
					if ((detect_collision(tmp, game->wall[y][x])) &&
					    (game->level[y + 1][x] != TILE_WALL) && (move_y < 0))
						move_y = (game->wall[y][x].y + game->wall[y][x].h) - game->player.y;
					break;
				case 6: /* Do not move through walls to the right. */
					tmp.x += move_x;
					if (detect_collision(tmp, game->wall[y][x]))
						move_x = game->wall[y][x].x - (game->player.x + game->player.w);
					break;
				case 9: /* Do not move through walls to the bottom-right from the left. */
					tmp.x += move_x;
					if ((detect_collision(tmp, game->wall[y][x])) &&
					    (game->wall[y][x].y < tmp.y + tmp.h))
						move_x = game->wall[y][x].x - (game->player.x + game->player.w);
					break;
				}
			}

			tmp = game->player;
			if (move_y < 0) {
				switch (position) {
				case 2: /* Do not move through walls to the top. */
					tmp.y += move_y;
					if (detect_collision(tmp, game->wall[y][x]))
						move_y = (game->wall[y][x].y + game->wall[y][x].h) - game->player.y;
					break;
				case 3: /* Do not move through walls to the top-right from the bottom. */
					tmp.y += move_y;
					if ((detect_collision(tmp, game->wall[y][x])) &&
					    (game->wall[y][x].x < tmp.x + tmp.w))
						move_y = (game->wall[y][x].y + game->wall[y][x].h) - game->player.y;
					break;
				}
			} else if (move_y > 0) {
				switch (position) {
				case 7: /* Do not move through walls to the bottom-left diagonally. */
					tmp.x += move_x, tmp.y += move_y;
					if ((detect_collision(tmp, game->wall[y][x])) &&
					    (game->level[y][x + 1] != TILE_WALL) && (move_x < 0))
						move_x = (game->wall[y][x].x + game->wall[y][x].w) - game->player.x;
					break;
				case 8: /* Do not move through walls to the bottom. */
					tmp.y += move_y;
					if (detect_collision(tmp, game->wall[y][x]))
						move_y = game->wall[y][x].y - (game->player.y + game->player.h);
					break;
				case 9: /* Do not move through walls to the bottom-right from the top. */
					tmp.y += move_y;
					if ((detect_collision(tmp, game->wall[y][x])) &&
					    (game->wall[y][x].x < tmp.x + tmp.w)) {
						move_y = game->wall[y][x].y - (game->player.y + game->player.h);
						break;
					}
					/* Do not move through walls to the bottom-right diagonally. */
					tmp.x += move_x;
					if ((detect_collision(tmp, game->wall[y][x])) &&
					    (game->level[y][x - 1] != TILE_WALL) && (move_x > 0))
						move_x = game->wall[y][x].x - (game->player.x + game->player.w);
					break;
				}
			}
			break;
		}

	/* Do not move over screen edges. */
	tmp = game->player, tmp.x += move_x;
	if ((tmp.x <= 0) && (move_x < 0))
		move_x = -(game->player.x);
	else if ((tmp.x + tmp.w >= LEVEL_W * TILE_SIZE) && move_x > 0)
		move_x = (LEVEL_W * TILE_SIZE) - (game->player.x + game->player.w);

	clear_entity(game, game->player);

	game->player.x += move_x;
	game->player.y += move_y;

	set_camera(game);
}

void set_camera(game_data *game)
{
	/* Keep the camera centered over our player. */
	game->camera.x = (game->player.x + ENTITY_SIZE / 2) - game->screen_w / 2;
	game->camera.y = (game->player.y + ENTITY_SIZE / 2) - game->screen_h / 2;

	/* Do not go out of bounds. */
	if (game->camera.x < 0)
		game->camera.x = 0;
	else if (game->camera.x > (LEVEL_W * TILE_SIZE) - game->camera.w)
		game->camera.x = (LEVEL_W * TILE_SIZE) - game->camera.w;

	if (game->camera.y < 0)
		game->camera.y = 0;
	else if (game->camera.y > (LEVEL_H * TILE_SIZE) - game->camera.h)
		game->camera.y = (LEVEL_H * TILE_SIZE) - game->camera.h;
}
