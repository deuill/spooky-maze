#include <stdio.h>
#include <SDL.h>

#include "game.h"
#include "graphics.h"
#include "levels.h"

void clear_level(struct game_data *game)
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

void generate_level(struct game_data *game)
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
		terminate(0);
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

void open_exit(struct game_data *game)
{
	int y;
	//~ SDL_Rect tmp;

	for (y = 0; y < LEVEL_H; y++) {
		if (game->level[y][LEVEL_W - 1] == TILE_DOOR) {
			game->level[y][LEVEL_W - 1] = TILE_EXIT;

			//~ TODO: Draw for tiles.
			//~ tmp = game->wall[y][LEVEL_W - 1];
			//~ convert_iso(tmp.x, tmp.y, (int *) &(tmp.x), (int *) &(tmp.y));
			//~ draw_tile(game, TILE_FLOOR, tmp);
			break;
		}
	}
}

void set_goodies(struct game_data *game)
{
	int x, y, i;

	for (i = 0; i < game->num_goodies; i++) {
		x = rand() % LEVEL_W, y = rand() % LEVEL_H;
		if (game->level[y][x] == TILE_FLOOR) {
			game->level[y][x] = TILE_GOODIE;
			game->goodie[i].rect.x = (TILE_SIZE * x) + (rand() % TILE_SIZE);
			game->goodie[i].rect.y = (TILE_SIZE * y) + (rand() % TILE_SIZE);
			game->goodie[i].rect.w = GOODIE_W;
			game->goodie[i].rect.h = GOODIE_H;

			convert_iso((struct pc *) &(game->goodie[i]));

			game->goodie[i].bg = init_surface(GOODIE_W, GOODIE_H);
		} else
			--i;
	}
}

void set_player(struct game_data *game)
{
	int y;

	/* Place our player in the level entrance. */
	for (y = 0; y < LEVEL_H; y++) {
		if (game->level[y][0] == TILE_DOOR) {
			game->level[y][0] = TILE_UNWALKABLE;
			game->player.rect.y = TILE_SIZE * y;
			game->player.rect.x = 0;
			game->player.rect.w = ENTITY_W;
			game->player.rect.h = ENTITY_H;
			game->player.dir_x = 0, game->player.dir_y = 0;

			convert_iso(&(game->player));

			game->player.bg = init_surface(ENTITY_W, ENTITY_H);

			break;
		}
	}
}

void set_zombies(struct game_data *game)
{
	int x, y, i;
	
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

			convert_iso((struct pc *) &(game->zombie[i]));

			game->zombie[i].bg = init_surface(ENTITY_W, ENTITY_H);
		} else {
			--i;
		}
	}
}
