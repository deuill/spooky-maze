#include <stdio.h>
#include <SDL.h>

#include "game.h"
#include "graphics.h"
#include "levels.h"

void level_clear(struct game_data *game)
{
	int x, y;

	/* Clear goodies off the level. */
	for (y = 0; y < LEVEL_H; y++)
		for (x = 0; x < LEVEL_W; x++) {
			if (game->level[y][x] == TILE_GOODIE)
				game->level[y][x] = TILE_FLOOR;
		}

	/* Set entrance into place. */
	for (y = 0; y < LEVEL_H; y++) {
		if ((game->level[y][0] == TILE_UNWALKABLE) || 
		    (game->level[y][0] == TILE_FLOOR)) {
			game->level[y][0] = TILE_DOOR;
			break;
		}
	}

	/* Close exit if open. */
	for (y = 0; y < LEVEL_H; y++) {
		if (game->level[y][LEVEL_W - 1] == TILE_EXIT) {
			game->level[y][LEVEL_W - 1] = TILE_DOOR;
			break;
		}
	}
}

bool level_collision(SDL_Rect entity, SDL_Rect wall)
{
	if (entity.x < wall.x) {
		if (entity.x + entity.w > wall.x) {
			if (entity.y + entity.h < wall.y)
				return false;
			else if (entity.y > wall.y + wall.h)
				return false;
			else
				return true;
		}
	} else if (entity.x < wall.x + wall.w) {
		if (entity.y + entity.h < wall.y)
			return false;
		else if (entity.y > wall.y + wall.h)
			return false;
		else
			return true;
	}

	return false;
}


void level_generate(struct game_data *game)
{
	int x, y, i;
	FILE *level;
	char tmp, filename[256];

	/* Choose a random text file. */
	snprintf(filename, 256, "%s%s-%d.txt", game->datadir, "/levels/level",  rand() % game->num_levels);

	/* Load the text file. */
	level = fopen(filename, "r");
	if (level == NULL) {
		printf("Error: Level file '%s' not found!\nExiting...\n", filename);
		game_terminate(0);
	}

	/* Parse text file into 'level' array. */
	for (y = 0; y <= LEVEL_H; y++) {
		for (x = 0; x <= LEVEL_W; x++) {
			fscanf(level, "%c", &tmp);
			if (tmp == ' ')
				x--;
			else if (tmp == '\n')
				break;
			else
				game->level[y][x] = tmp;
		}

		if (feof(level))
			break;
	}

	/* Should we mirror the level? */
	if (rand() % 2) {
		for (y = 0; y < LEVEL_H; y++) {
			for (x = 0, i = LEVEL_W - 1; x < LEVEL_W / 2; x++, i--) {
				tmp = game->level[y][i];
				game->level[y][i] = game->level[y][x];
				game->level[y][x] = tmp;
			}
		}
	}

	/* Should we flip the level? */
	if (rand() % 2) {
		for (x = 0; x < LEVEL_W; x++) {
			for (y = 0, i = LEVEL_H - 1; y < LEVEL_H / 2; y++, i--) {
				tmp = game->level[i][x];
				game->level[i][x] = game->level[y][x];
				game->level[y][x] = tmp;
			}
		}
	}

	fclose(level);
}

int level_tile_visible(int src_x, int src_y, int dest_x, int dest_y, char level[LEVEL_H][LEVEL_W])
{
	int x = src_x, y = src_y;

	/* Move towards the destination element. */
	while (x != dest_x || y != dest_y) {
		if (x < dest_x)
			x++;
		else if (x > dest_x)
			x--;

		if (y < dest_y)
			y++;
		else if (y > dest_y)
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

void level_unlock(struct game_data *game)
{
	int y;
	//~ SDL_Rect tmp;

	for (y = 0; y < LEVEL_H; y++) {
		if (game->level[y][LEVEL_W - 1] == TILE_DOOR) {
			game->level[y][LEVEL_W - 1] = TILE_EXIT;

			//~ TODO: Draw for tiles.
			//~ tmp = game->wall[y][LEVEL_W - 1];
			//~ graphics_iso_convert(tmp.x, tmp.y, (int *) &(tmp.x), (int *) &(tmp.y));
			//~ graphics_tile_draw(game, TILE_FLOOR, tmp);
			break;
		}
	}
}

void level_entities_set(struct game_data *game)
{
	int x, y, i;

	/* Place our player in the level entrance. */
	for (y = 0; y < LEVEL_H; y++) {
		if (game->level[y][0] == TILE_DOOR) {
			game->level[y][0] = TILE_UNWALKABLE;
			game->player.rect.y = TILE_SIZE * y;
			game->player.rect.x = 0;
			game->player.rect.w = ENTITY_W;
			game->player.rect.h = ENTITY_H;
			game->player.dir_x = 0, game->player.dir_y = 0;

			graphics_iso_convert(&(game->player));

			game->player.bg = graphics_surface_init(ENTITY_W, ENTITY_H);

			break;
		}
	}

	/* Place zombies in random locations in the level. */
	for (i = 0; i < game->num_zombies; i++) {
		x = rand() % LEVEL_W, y = rand() % LEVEL_H;
		if (game->level[y][x] == TILE_FLOOR) {
			game->zombie[i].rect.x = x * TILE_SIZE;
			game->zombie[i].rect.y = y * TILE_SIZE;
			game->zombie[i].rect.w = ENTITY_W;
			game->zombie[i].rect.h = ENTITY_H;
			game->zombie[i].num_nodes = 0;
			game->zombie[i].dest_x = 0;
			game->zombie[i].dest_y = 0;

			graphics_iso_convert((struct pc *) &(game->zombie[i]));

			game->zombie[i].bg = graphics_surface_init(ENTITY_W, ENTITY_H);
		} else {
			--i;
		}
	}

	/* Place goodies in random locations in the level. */
	for (i = 0; i < game->num_goodies; i++) {
		x = rand() % LEVEL_W, y = rand() % LEVEL_H;
		if (game->level[y][x] == TILE_FLOOR) {
			game->level[y][x] = TILE_GOODIE;
			game->goodie[i].rect.x = (TILE_SIZE * x) + (rand() % TILE_SIZE);
			game->goodie[i].rect.y = (TILE_SIZE * y) + (rand() % TILE_SIZE);
			game->goodie[i].rect.w = GOODIE_W;
			game->goodie[i].rect.h = GOODIE_H;

			graphics_iso_convert((struct pc *) &(game->goodie[i]));

			game->goodie[i].bg = graphics_surface_init(GOODIE_W, GOODIE_H);
		} else {
			--i;
		}
	}
}
