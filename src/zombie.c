#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#include "game.h"
#include "graphics.h"
#include "levels.h"
#include "player.h"
#include "zombie.h"

int find_path(struct npc *zombie, char level[LEVEL_H][LEVEL_W])
{
	int x, y;
	int position, i;
	int t = 0, c = 0;
	struct list *tmp;
	struct list *current_node;
	struct list open[SEARCH_DEPTH];
	struct list closed[SEARCH_DEPTH];

	/* First, add our zombie to the closed list. */
	closed[c].x = zombie->rect.x / TILE_SIZE;
	closed[c].y = zombie->rect.y / TILE_SIZE;
	closed[c].g = 0;
	closed[c].parent = NULL;
	current_node = &closed[c];
	c++;

	/* Then, search all adjacent squares and add them to the open list if suitable. */
	for (;;) {
		position = 1;
		for (y = current_node->y - 1; y <= current_node->y + 1; y++)
		for (x = current_node->x - 1; x <= current_node->x + 1; x++, position++) {
			/* Check if the node is a wall. */
			if ((level[y][x] == TILE_WALL) || (level[y][x] == TILE_UNWALKABLE) || (level[y][x] == TILE_DOOR))
				continue;

			/* Don't cut through corners. */
			switch (position) {
				case 1:
					if (level[current_node->y][current_node->x - 1] == TILE_WALL)
						goto next_node; /* Go to the end of the loop. */
					else if (level[current_node->y - 1][current_node->x] == TILE_WALL)
						goto next_node;
					break;
				case 3:
					if (level[current_node->y][current_node->x + 1] == TILE_WALL)
						goto next_node;
					else if (level[current_node->y - 1][current_node->x] == TILE_WALL)
						goto next_node;
					break;
				case 7:
					if (level[current_node->y][current_node->x - 1] == TILE_WALL)
						goto next_node;
					else if (level[current_node->y + 1][current_node->x] == TILE_WALL)
						goto next_node;
					break;
				case 9:
					if (level[current_node->y][current_node->x + 1] == TILE_WALL)
						goto next_node;
					else if (level[current_node->y + 1][current_node->x] == TILE_WALL)
						goto next_node;
					break;
				default:
					break;
			}

			/* Check if we have matched our destination. */
			if (x == zombie->dest_x && y == zombie->dest_y) {
				closed[c].x = x, closed[c].y = y;
				closed[c].parent = current_node;
				goto dest_found; /* Go to the (near) end of the function. */
			}

			/* Check if the node is in the closed list. */
			for (i = c - 1; i >= 0; i--)
				if (x == closed[i].x && y == closed[i].y)
					goto next_node;

			/* Check if the node is in the open list. */
			for (i = t - 1; i >= 0; i--)
				if (x == open[i].x && y == open[i].y && !open[i].dropped) {
					if ((current_node->g + (position % 2) ? 14 : 10) < open[i].g) {
						open[i].g = current_node->g + (position % 2) ? 14 : 10;
						open[i].f = open[i].g + open[i].h;
						open[i].parent = current_node;
						goto next_node;
					} else {
						goto next_node;
					}
				}

			/* Node is not a wall and is on neither list, add to the open list. */
			open[t].x = x, open[t].y = y;
			open[t].g = current_node->g + (position % 2) ? 14 : 10;
			open[t].h = (abs(open[t].x - zombie->dest_x) + abs(open[t].y - zombie->dest_y)) * 10;
			open[t].f = open[t].g + open[t].h;
			open[t].parent = current_node;
			open[t].dropped = false;

			/* If we have exceeded our search depth, set current adjacent
			 * node as destination and move there instead. */
			if (t < SEARCH_DEPTH - 1)
				t++;
			else {
				closed[c].x = x, closed[c].y = y;
				closed[c].parent = current_node;
				goto dest_found;
			}

			/* End of loop. */
			next_node:;
		}

		/* Search open list for node with the lowest 'f' and move to it. */
		tmp = NULL;
		for (i = t - 1; i > 0; i--) {
			if (!open[i].dropped) {
				if (tmp == NULL)
					tmp = &open[i];
				else if (open[i].f < tmp->f)
					tmp = &open[i];
			}
		}

		/* Check for dead end. */
		if (tmp == NULL) 
			return 0;
		else
			current_node = tmp;

		/* Drop current node from the open list and add it to the closed list. */
		current_node->dropped = true;
		closed[c] = *current_node;
		c++;
	}

	/* We have reached our destination! */
	dest_found:

	/* Retrace the path from the end, following parent nodes until we
	 * reach our zombie. Copy X/Y coordinates into the 'path' struct. */
	zombie->path[1].x = closed[c].x;
	zombie->path[1].y = closed[c].y;
	tmp = closed[c].parent;

	for (i = 2; tmp != NULL; i++) {
		zombie->path[i].x = tmp->x;
		zombie->path[i].y = tmp->y;
		tmp = tmp->parent;
	}

	/* Return the number of nodes in the path, starting from the node next to
	 * our current position unless certain conditions are met and we need to
	 * center on our current position first. */
	if ((zombie->path[i - 2].x < zombie->path[i - 1].x) &&
	    (zombie->rect.y > (zombie->path[i - 1].y * TILE_SIZE)) &&
	    (level[zombie->path[i - 1].y + 1][zombie->path[i - 1].x - 1] == TILE_WALL))
		return i - 1;
	if ((zombie->path[i - 2].y < zombie->path[i - 1].y) &&
	    (zombie->rect.x > (zombie->path[i - 1].x * TILE_SIZE)) &&
	    (level[zombie->path[i - 1].y - 1][zombie->path[i - 1].x + 1] == TILE_WALL))
		return i - 1;
	if ((zombie->path[i - 2].x > zombie->path[i - 1].x) &&
	    (zombie->rect.y > (zombie->path[i - 1].y * TILE_SIZE)) &&
	    (level[zombie->path[i - 1].y + 1][zombie->path[i - 1].x + 1] == TILE_WALL))
		return i - 1;
	if ((zombie->path[i - 2].y > zombie->path[i - 1].y) &&
	    (zombie->rect.x > (zombie->path[i - 1].x * TILE_SIZE)) &&
	    (level[zombie->path[i - 1].y + 1][zombie->path[i - 1].x + 1] == TILE_WALL))
		return i - 1;

	return i - 2;
}

int line_of_sight(int src_x, int src_y, int dst_x, int dst_y, char level[LEVEL_H][LEVEL_W])
{
	int x = src_x, y = src_y;

	/* Move towards the destination element. */
	while (x != dst_x || y != dst_y) {
		if (x < dst_x)
			x++;
		else if (x > dst_x)
			x--;

		if (y < dst_y)
			y++;
		else if (y > dst_y)
			y--;

		switch (level[y][x]) {
			case TILE_WALL:
				return false;
			case TILE_UNWALKABLE:
				return false;
		}
	}

	/* Move backwards towards the source element. */
	while (x != src_x || y != src_y) {
		if (x < src_x)
			x++;
		else if (x > src_x)
			x--;

		if (y < src_y)
			y++;
		else if (y > src_y)
			y--;

		switch (level[y][x]) {
			case TILE_WALL:
				return false;
			case TILE_UNWALKABLE:
				return false;
		}
	}

	return true;
}

void move_zombies(game_data *game)
{
	SDL_Rect tmp;
	int x, y, i, n;
	int move_x, move_y;

	/* Protect against incorrect delta-time readings */
	if (game->delta_time > 100)
		return;

	/* Scale the zombie speed depending on the frame-rate */
	move_x = (int) (ZOMBIE_SPEED * ((float) game->delta_time / 1000.0f));
	move_y = (int) (ZOMBIE_SPEED * ((float) game->delta_time / 1000.0f));

	for (i = 0; i < game->num_zombies; i++) {
		/* 
		 * Chase our player if found closer than 5 tiles away.
		 */
		if (abs(PLAYER_X - ZOMBIE_X(i)) <= 5 && abs(PLAYER_Y - ZOMBIE_Y(i)) <= 5) {
			clear_entity(game, ZOMBIE(i).rect);

			/* Check if we have collided with the player. */
			if (abs(PLAYER_X - ZOMBIE_X(i)) <= 1 && abs(PLAYER_Y - ZOMBIE_Y(i)) <= 1)
				if (detect_collision(ZOMBIE(i).rect, game->player)) {
					game->dead = true;
					return;
				}

			/* Recalculate line of sight if the player has moved from the destination node. */
			if (((ZOMBIE(i).dest_x != PLAYER_X) || (ZOMBIE(i).dest_y != PLAYER_Y)) &&
			    (game->level[PLAYER_Y][PLAYER_X] == TILE_FLOOR)) {
				if (line_of_sight(PLAYER_X, PLAYER_Y, ZOMBIE_X(i), ZOMBIE_Y(i), game->level)) {
					ZOMBIE(i).dest_x = PLAYER_X;
					ZOMBIE(i).dest_y = PLAYER_Y;
					ZOMBIE(i).num_nodes = 0;
				/* We reached the player's last known position and found nothing. */
				} else if ((ZOMBIE(i).num_nodes == 0) &&
				            (ZOMBIE_X(i) == ZOMBIE(i).dest_x) &&
				            (ZOMBIE_Y(i) == ZOMBIE(i).dest_y)) {
					ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
					goto random;
				/* Move to last known location if player is out of sight. */
				} else if (ZOMBIE(i).num_nodes == 0) {
					ZOMBIE(i).num_nodes = find_path(&ZOMBIE(i), game->level);

					/* Set a random destination if we can't reach our
					 * player, otherwise move to the chosen destination. */
					if (ZOMBIE(i).num_nodes == 0)
						goto random;
					else
						goto move;
				}
			}

			if ((ZOMBIE(i).num_nodes == 0) && (game->level[PLAYER_Y][PLAYER_X] == TILE_FLOOR)) {
				/* Reset X and Y movement speed if zeroed out */
				if (move_x == 0)
					move_x = (int) (ZOMBIE_SPEED * ((float) game->delta_time / 1000.0f));
				if (move_y == 0)
					move_y = (int) (ZOMBIE_SPEED * ((float) game->delta_time / 1000.0f));

				tmp = ZOMBIE(i).rect;

				/* Check for collision in the X axis. */
				if (ZOMBIE(i).rect.x < ZOMBIE(i).dest_x * TILE_SIZE) {
					tmp.x += move_x;

					/* Do not move through walls to the right. */
					switch (game->level[ZOMBIE_Y(i)][ZOMBIE_X(i) + 1]) {
					case TILE_WALL:
					case TILE_UNWALKABLE:
						if (detect_collision(tmp, game->wall[ZOMBIE_Y(i)][ZOMBIE_X(i) + 1]))
							move_x = game->wall[ZOMBIE_Y(i)][ZOMBIE_X(i) + 1].x - (ZOMBIE(i).rect.x + ENTITY_SIZE);
					}

					/* Do not move through walls to the bottom right. */
					switch (game->level[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) + 1]) {
					case TILE_WALL:
					case TILE_UNWALKABLE:
						if (detect_collision(tmp, game->wall[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) + 1]))
							move_x = game->wall[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) + 1].x - (ZOMBIE(i).rect.x + ENTITY_SIZE);
					}

					if (tmp.x > ZOMBIE(i).dest_x * TILE_SIZE)
						move_x = tmp.x - (ZOMBIE(i).dest_x * TILE_SIZE);
				} else if (ZOMBIE(i).rect.x > ZOMBIE(i).dest_x * TILE_SIZE) {
					tmp.x -= move_x;

					/* Do not move through walls to the left. */
					switch (game->level[ZOMBIE_Y(i)][ZOMBIE_X(i) - 1]) {
					case TILE_WALL:
					case TILE_UNWALKABLE:
						if (detect_collision(tmp, game->wall[ZOMBIE_Y(i)][ZOMBIE_X(i) - 1]))
							move_x = ZOMBIE(i).rect.x - (game->wall[ZOMBIE_Y(i)][ZOMBIE_X(i) - 1].x + TILE_SIZE);
					}

					/* Do not move through walls to the bottom left. */
					switch (game->level[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) - 1]) {
					case TILE_WALL:
					case TILE_UNWALKABLE:
						if (detect_collision(tmp, game->wall[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) - 1]))
							move_x = ZOMBIE(i).rect.x - (game->wall[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) - 1].x + TILE_SIZE);
					}

					if (tmp.x < ZOMBIE(i).dest_x * TILE_SIZE)
						move_x = (ZOMBIE(i).dest_x * TILE_SIZE) - tmp.x;
				}

				/* Check for collision on the Y axis. */
				if (ZOMBIE(i).rect.y < ZOMBIE(i).dest_y * TILE_SIZE) {
					tmp.y += move_y;

					/* Do not move through walls to the bottom. */
					switch (game->level[ZOMBIE_Y(i) + 1][ZOMBIE_X(i)]) {
					case TILE_WALL:
					case TILE_UNWALKABLE:
						if (detect_collision(tmp, game->wall[ZOMBIE_Y(i) + 1][ZOMBIE_X(i)]))
							move_y = game->wall[ZOMBIE_Y(i) + 1][ZOMBIE_X(i)].y - (ZOMBIE(i).rect.y + ENTITY_SIZE);
					}

					/* Do not move through walls to the bottom right. */
					switch (game->level[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) + 1]) {
					case TILE_WALL:
					case TILE_UNWALKABLE:
						if (detect_collision(tmp, game->wall[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) + 1]))
							move_y = game->wall[ZOMBIE_Y(i) + 1][ZOMBIE_X(i) + 1].y - (ZOMBIE(i).rect.y + ENTITY_SIZE);
					}

					if (tmp.y > ZOMBIE(i).dest_y * TILE_SIZE)
						move_y = tmp.y - (ZOMBIE(i).dest_y * TILE_SIZE);
				} else if (ZOMBIE(i).rect.y > ZOMBIE(i).dest_y * TILE_SIZE) {
					tmp.y -= move_y;

					/* Do not move through walls to the top. */
					switch (game->level[ZOMBIE_Y(i) - 1][ZOMBIE_X(i)]) {
					case TILE_WALL:
					case TILE_UNWALKABLE:
						if (detect_collision(tmp, game->wall[ZOMBIE_Y(i) - 1][ZOMBIE_X(i)]))
							move_y = ZOMBIE(i).rect.y - (game->wall[ZOMBIE_Y(i) - 1][ZOMBIE_X(i)].y + TILE_SIZE);
					}

					/* Do not move through walls to the top right. */
					switch (game->level[ZOMBIE_Y(i) - 1][ZOMBIE_X(i) + 1]) {
					case TILE_WALL:
					case TILE_UNWALKABLE:
						if (detect_collision(tmp, game->wall[ZOMBIE_Y(i) - 1][ZOMBIE_X(i) + 1]))
							move_y = ZOMBIE(i).rect.y - (game->wall[ZOMBIE_Y(i) - 1][ZOMBIE_X(i) + 1].y + TILE_SIZE);
					}

					if (tmp.y < ZOMBIE(i).dest_y * TILE_SIZE)
						move_y = (ZOMBIE(i).dest_y * TILE_SIZE) - tmp.y;
				}

				/* Do not move in space occupied by other zombies. */
				for (n = 0; n < game->num_zombies; n++) {
					if (n != i) {
						if (detect_collision(tmp, game->zombie[n].rect)) {
							/* Stop moving if other zombie is in path */
							if ((ZOMBIE(i).rect.x > game->zombie[n].rect.x + game->zombie[n].rect.w) &&
							    (tmp.x < ZOMBIE(i).rect.x))
								move_x = 0;
							else if ((ZOMBIE(i).rect.x + ZOMBIE(i).rect.w < game->zombie[n].rect.x) &&
								  (tmp.x > ZOMBIE(i).rect.x))
								move_x = 0;
							if ((ZOMBIE(i).rect.y > game->zombie[n].rect.y + game->zombie[n].rect.h) &&
							    (tmp.y < ZOMBIE(i).rect.y))
								move_y = 0;
							else if ((ZOMBIE(i).rect.y + ZOMBIE(i).rect.h < game->zombie[n].rect.y) &&
								  (tmp.y > ZOMBIE(i).rect.y))
								move_y = 0;
						}
					}
				}

				/* Move towards destination. */
				if (ZOMBIE(i).rect.x < ZOMBIE(i).dest_x * TILE_SIZE)
					ZOMBIE(i).rect.x += move_x;
				else if (ZOMBIE(i).rect.x > ZOMBIE(i).dest_x * TILE_SIZE)
					ZOMBIE(i).rect.x -= move_x;

				if (ZOMBIE(i).rect.y < ZOMBIE(i).dest_y * TILE_SIZE)
					ZOMBIE(i).rect.y += move_y;
				else if (ZOMBIE(i).rect.y > ZOMBIE(i).dest_y * TILE_SIZE)
					ZOMBIE(i).rect.y -= move_y;
			} else if (ZOMBIE(i).num_nodes == 0) {
				ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
				goto random;
			} else {
				goto move;
			}
		/* 
		 * Start moving if we do have a destination set.
		 */
		} else if (ZOMBIE(i).num_nodes > 0) {
			clear_entity(game, ZOMBIE(i).rect);

			move:

			/* Check if we have reached the next node. */
			if ((ZOMBIE_NODE(i).x * TILE_SIZE == ZOMBIE(i).rect.x) &&
			    (ZOMBIE_NODE(i).y * TILE_SIZE == ZOMBIE(i).rect.y)) {
				ZOMBIE(i).num_nodes--;
				if (ZOMBIE(i).num_nodes == 0) {
					ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
					continue;
				}
			}

			/* Reset X and Y movement speed if zeroed out */
			if (move_x == 0)
				move_x = (int) (ZOMBIE_SPEED * ((float) game->delta_time / 1000.0f));
			if (move_y == 0)
				move_y = (int) (ZOMBIE_SPEED * ((float) game->delta_time / 1000.0f));

			/* Calculate movement direction */
			tmp = ZOMBIE(i).rect;
			if (ZOMBIE_NODE(i).x * TILE_SIZE < ZOMBIE(i).rect.x)
				tmp.x -= move_x;
			else if (ZOMBIE_NODE(i).x * TILE_SIZE > ZOMBIE(i).rect.x)
				tmp.x += move_x;
			if (ZOMBIE_NODE(i).y * TILE_SIZE < ZOMBIE(i).rect.y)
				tmp.y -= move_y;
			else if (ZOMBIE_NODE(i).y * TILE_SIZE > ZOMBIE(i).rect.y)
				tmp.y += move_y;

			/* Do not move in space occupied by other zombies. */
			for (n = 0; n < game->num_zombies; n++) {
				if (n != i) {
					if (detect_collision(tmp, game->zombie[n].rect)) {
						/* Recalculate path if stuck against one another. */
						if ((ZOMBIE(i).rect.x + ZOMBIE(i).rect.w < ZOMBIE(n).rect.x) &&
						    (ZOMBIE(i).rect.x < ZOMBIE_NODE(i).x * TILE_SIZE) &&
						    (ZOMBIE(n).rect.x > ZOMBIE_NODE(n).x * TILE_SIZE)) {
							ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
							goto random;
						} else if ((ZOMBIE(i).rect.x + ZOMBIE(i).rect.w > ZOMBIE(n).rect.x) &&
						    (ZOMBIE(i).rect.x > ZOMBIE_NODE(i).x * TILE_SIZE) &&
						    (ZOMBIE(n).rect.x < ZOMBIE_NODE(n).x * TILE_SIZE)) {
							ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
							goto random;
						}

						if ((ZOMBIE(i).rect.y + ZOMBIE(i).rect.h < ZOMBIE(n).rect.y) &&
						    (ZOMBIE(i).rect.y < ZOMBIE_NODE(i).y * TILE_SIZE) &&
						    (ZOMBIE(n).rect.y > ZOMBIE_NODE(n).y * TILE_SIZE)) {
							ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
							goto random;
						} else if ((ZOMBIE(i).rect.y + ZOMBIE(i).rect.h > ZOMBIE(n).rect.y) &&
						    (ZOMBIE(i).rect.y > ZOMBIE_NODE(i).y * TILE_SIZE) &&
						    (ZOMBIE(n).rect.y < ZOMBIE_NODE(n).y * TILE_SIZE)) {
							ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
							goto random;
						}

						/* Stop moving if other zombie is in path */
						if ((ZOMBIE(i).rect.x > game->zombie[n].rect.x + game->zombie[n].rect.w) &&
						    (tmp.x < ZOMBIE(i).rect.x))
							move_x = 0;
						else if ((ZOMBIE(i).rect.x + ZOMBIE(i).rect.w < game->zombie[n].rect.x) &&
							  (tmp.x > ZOMBIE(i).rect.x))
							move_x = 0;
						if ((ZOMBIE(i).rect.y > game->zombie[n].rect.y + game->zombie[n].rect.h) &&
						    (tmp.y < ZOMBIE(i).rect.y))
							move_y = 0;
						else if ((ZOMBIE(i).rect.y + ZOMBIE(i).rect.h < game->zombie[n].rect.y) &&
							  (tmp.y > ZOMBIE(i).rect.y))
							move_y = 0;
					}
				}
			}

			/* Move our zombie towards the next node in our path */
			if (ZOMBIE_NODE(i).x * TILE_SIZE < ZOMBIE(i).rect.x) {
				if (tmp.x < ZOMBIE_NODE(i).x * TILE_SIZE)
					ZOMBIE(i).rect.x -= ZOMBIE(i).rect.x - (ZOMBIE_NODE(i).x * TILE_SIZE);
				else
					ZOMBIE(i).rect.x -= move_x;
			} else if (ZOMBIE_NODE(i).x * TILE_SIZE > ZOMBIE(i).rect.x) {
				if (tmp.x > ZOMBIE_NODE(i).x * TILE_SIZE)
					ZOMBIE(i).rect.x += (ZOMBIE_NODE(i).x * TILE_SIZE) - ZOMBIE(i).rect.x;
				else
					ZOMBIE(i).rect.x += move_x;
			}

			if (ZOMBIE_NODE(i).y * TILE_SIZE < ZOMBIE(i).rect.y) {
				if (tmp.y < ZOMBIE_NODE(i).y * TILE_SIZE)
					ZOMBIE(i).rect.y -= ZOMBIE(i).rect.y - (ZOMBIE_NODE(i).y * TILE_SIZE);
				else
					ZOMBIE(i).rect.y -= move_y;
			} else if (ZOMBIE_NODE(i).y * TILE_SIZE > ZOMBIE(i).rect.y) {
				if (tmp.y > ZOMBIE_NODE(i).y * TILE_SIZE)
					ZOMBIE(i).rect.y += (ZOMBIE_NODE(i).y * TILE_SIZE) - ZOMBIE(i).rect.y;
				else
					ZOMBIE(i).rect.y += move_y;
			}
		/* 
		 * We don't have a destination set, so let's set one +/- 10 squares away. 
		 */
		} else {
			random:

			/* Be biased toward pre-existing destinations, used for chasing
			 * the player after losing sight. */
			if (ZOMBIE(i).dest_x != 0 && ZOMBIE(i).dest_y != 0) {
				if (ZOMBIE(i).dest_x - ZOMBIE_X(i) > 0)
					x = (rand() % 10);
				else if (ZOMBIE(i).dest_x - ZOMBIE_X(i) < 0)
					x = (rand() % 10) * -1;
				else
					x = (rand() % 20) - 10;

				if (ZOMBIE(i).dest_y - ZOMBIE_Y(i) > 0)
					y = (rand() % 10);
				else if (ZOMBIE(i).dest_y - ZOMBIE_Y(i) < 0)
					y = (rand() % 10) * -1;
				else
					y = (rand() % 20) - 10;
			} else {
				x = (rand() % 20) - 10, y = (rand() % 20) - 10;
			}

			if ((ZOMBIE_X(i) + x < 0) || (ZOMBIE_X(i) + x > LEVEL_W) || 
			    (ZOMBIE_Y(i) + y < 0) || (ZOMBIE_Y(i) + y > LEVEL_W)) {
				ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
				i--;
			} else if (game->level[ZOMBIE_Y(i) + y][ZOMBIE_X(i) + x] == TILE_FLOOR) {
				ZOMBIE(i).dest_x = ZOMBIE_X(i) + x;
				ZOMBIE(i).dest_y = ZOMBIE_Y(i) + y;

				ZOMBIE(i).num_nodes = find_path(&ZOMBIE(i), game->level);
				if (ZOMBIE(i).num_nodes == 0) {
					ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
					i--;
				}
			} else {
				ZOMBIE(i).dest_x = 0, ZOMBIE(i).dest_y = 0;
				i--;
			}
		}
	}
}
