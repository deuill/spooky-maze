#include <stdio.h>
#include <SDL.h>

#include "game.h"
#include "graphics.h"
#include "levels.h"

void clear_level(game_data *game)
{
	int x, y;

	/* Clear goodies off the level. */
	for (y = 0; y < LEVEL_H; y++)
		for (x = 0; x < LEVEL_W; x++) {
			if (game->level[y][x] == 'g')
				game->level[y][x] = '.';
		}

	/* Set entrance into place. */
	for (y = 0; y < LEVEL_H; y++) {
		if ((game->level[y][0] == 'x') || 
		    (game->level[y][0] == '.')) {
			game->level[y][0] = 'd';
			break;
		}
	}

	/* Close exit if open. */
	for (y = 0; y < LEVEL_H; y++) {
		if (game->level[y][LEVEL_W - 1] == 'e') {
			game->level[y][LEVEL_W - 1] = 'd';
			break;
		}
	}
}

void generate_level(game_data *game)
{
	int x, y, i;
	FILE *level;
	char tmp, filename[64];

	/* Choose a random text file. */
	sprintf(filename, "%s%s-%d", DATADIR, "data/levels/level",  rand() % NUM_LEVELS);

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

void open_exit(game_data *game)
{
	int y;

	for (y = 0; y < LEVEL_H; y++) {
		if (game->level[y][LEVEL_W - 1] == 'd') {
			game->level[y][LEVEL_W - 1] = 'e';
			clear_entity(&(*game), game->wall[y][LEVEL_W - 1]);
			break;
		}
	}
}

void set_goodies(game_data *game)
{
	int x, y, i;

	game->num_goodies = 12;

	for (i = 0; i < game->num_goodies; i++) {
		x = rand() % LEVEL_W, y = rand() % LEVEL_H;
		if (game->level[y][x] == '.') {
			game->level[y][x] = 'g';
			game->goodie[i].x = x * TILE_SIZE + (TILE_SIZE / 4);
			game->goodie[i].y = y * TILE_SIZE + (TILE_SIZE / 4);
			game->goodie[i].w = TILE_SIZE / 2;
			game->goodie[i].h = TILE_SIZE / 2;
		} else
			--i;
	}
}

void set_player(game_data *game)
{
	int y;

	/* Place our player in the level entrance. */
	for (y = 0; y < LEVEL_H; y++) {
		if (game->level[y][0] == 'd') {
			game->level[y][0] = 'x';
			game->player.y = y * TILE_SIZE;
			game->player.x = 0;
			break;
		}
	}
}

void set_zombies(game_data *game)
{
	int x, y, i;
	
	game->num_zombies = 6;
	
	for (i = 0; i < game->num_zombies; i++) {
		x = rand() % LEVEL_W, y = rand() % LEVEL_H;
		if (game->level[y][x] == '.') {
			game->zombie[i].rect.x = x * TILE_SIZE;
			game->zombie[i].rect.y = y * TILE_SIZE;
			game->zombie[i].rect.w = ENTITY_SIZE;
			game->zombie[i].rect.h = ENTITY_SIZE;
			game->zombie[i].num_nodes = 0;
			game->zombie[i].dest_x = 0;
			game->zombie[i].dest_y = 0;
		} else
			--i;
	}
}