#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include <SDL.h>

#include "game.h"
#include "graphics.h"
#include "input.h"
#include "levels.h"
#include "player.h"
#include "zombie.h"

int game_terminate(int code)
{
	if (code != 0)
		fprintf(stderr, "Error: %s\nExiting...\n", SDL_GetError());
	SDL_Quit();
	exit(code);
}

static void game_usage(void)
{
	printf(	"Usage: spooky-maze [OPTION]...\n"
		"Options:\n"
		" -d, --datadir\t\tDirectory where data files reside.\n"
		" -f, --fullscreen\tStart game in fullscreen.\n"
		" -s, --size\t\tSize of game screen (example usage: '-s 800x600').\n"
		" -h, --help\t\tDisplay this text.\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int i;
	DIR *tmp_dir;
	char *token;
	char dirname[256];
	bool fullscreen = false;
	struct dirent *tmp_file;

	struct game_data game;
	SDL_Surface *tmp;
	Uint32 level_time;
	Uint32 start_time, end_time;

	/* Process command-line arguments. */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--datadir") == 0 || strcmp(argv[i], "-d") == 0) {
			if (argv[i + 1] == NULL || argv[i + 1][0] == '-') {
				fprintf(stderr, "spooky-maze: Error: no directory specified after '%s'!\n", argv[i]);
				exit(1);
			} else if ((tmp_dir = opendir(argv[++i]))) {
				closedir(tmp_dir);
				game.datadir = argv[i];
			} else {
				closedir(tmp_dir);
				fprintf(stderr, "spooky-maze: Error: directory '%s' does not exist or cannot be accessed!\n", argv[i]);
				exit(1);
			}
		} else if (strcmp(argv[i], "--fullscreen") == 0 || strcmp(argv[i], "-f") == 0) {
			fullscreen = true;
		} else if (strcmp(argv[i], "--size") == 0 || strcmp(argv[i], "-s") == 0) {
			if ((token = strtok(argv[++i], "x")) != NULL)
				game.screen_w = atoi(token);
			else
				game_usage();
			
			if ((token = strtok(NULL, "x")) != NULL)
				game.screen_h = atoi(token);
			else
				game_usage();

			if (game.screen_w == 0 || game.screen_h == 0)
				game_usage();
		} else {
			game_usage();
		}
	}

	if (game.datadir == NULL)
		game.datadir = DATADIR;

	if (game.screen_w == 0 || game.screen_h == 0) {
		game.screen_w = SCREEN_W;
		game.screen_h = SCREEN_H;
	}

	/* Count number of levels. */
	game.num_levels = 0;
	snprintf(dirname, 256, "%s%s", game.datadir, "/levels/");

	tmp_dir = opendir(dirname);
	if (tmp_dir == NULL) {
		fprintf(stderr, "spooky-maze: Error: Could not find data files in '%s'.\n", dirname);
		game_usage();
	}

	while ((tmp_file = readdir(tmp_dir)) != NULL) {
		if (strncmp("level-", tmp_file->d_name, 6) == 0)
			game.num_levels++;
	}

	closedir(tmp_dir);

	/* Initialize SDL and friends. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "spooky-maze: Fatal error: %s!\nExiting...\n", SDL_GetError());
		exit(2);
	}

	if (fullscreen)
		game.screen = SDL_SetVideoMode(game.screen_w, game.screen_h, SCREEN_DEPTH,
			      SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
	else
		game.screen = SDL_SetVideoMode(game.screen_w, game.screen_h, SCREEN_DEPTH,
			      SDL_HWSURFACE | SDL_DOUBLEBUF);

	if (game.screen == NULL) {
		fprintf(stderr, "spooky-maze: Fatal error: %s!\nExiting...\n", SDL_GetError());
		exit(2);
	}

	if (SDL_NumJoysticks() > 0) {
		for (i = 0; i < SDL_NumJoysticks(); i++) {
			SDL_JoystickEventState(SDL_ENABLE);
			game.joystick = SDL_JoystickOpen(i);

			/* Some devices are erroneously presented as joysticks,
			 * and we don't like that. */
			if (SDL_JoystickNumAxes(game.joystick) > 20)
				SDL_JoystickClose(game.joystick);
		}
	}

	tmp = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
				   LEVEL_W * TILE_SIZE, LEVEL_H * TILE_SIZE,
				   SCREEN_DEPTH, 0, 0, 0, 0);

	game.world = SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);

	SDL_WM_SetCaption("Spooky Maze", "spooky-maze");
	SDL_ShowCursor(SDL_DISABLE);

	srand((unsigned int) time(NULL));

	graphics_assets_load(&game);

	game.camera.w = game.screen_w;
	game.camera.h = game.screen_h;

	/* TEMP */
	game.black  = SDL_MapRGB(game.screen->format, 0x00, 0x00, 0x00);
	game.white  = SDL_MapRGB(game.screen->format, 0xff, 0xff, 0xff);
	game.red    = SDL_MapRGB(game.screen->format, 0xcc, 0x00, 0x00);
	game.blue   = SDL_MapRGB(game.screen->format, 0x1f, 0x00, 0xff);
	game.brown  = SDL_MapRGB(game.screen->format, 0x77, 0x54, 0x00);
	game.yellow = SDL_MapRGB(game.screen->format, 0xFF, 0xFA, 0x00);

	for (;;) {
		game.score = 0;
		game.cur_level = 1;
		game.score_scale = 10000;
		game.level_cleared = true;

		game.player.lives = 3;
		game.player.dead = false;

		for (;;) {
			if (game.level_cleared)
				level_generate(&game);
			else
				level_clear(&game);

			game.level_cleared = false;

			game.num_zombies = 6;
			game.num_goodies = 12;

			level_entities_set(&game);
			player_camera_follow(&game);

			graphics_level_draw(&game);

			for (i = 0; i < game.num_goodies; i++)
				graphics_entity_store(&(game), (struct pc *) &(game.goodie[i]));

			for (i = 0; i < game.num_zombies; i++)
				graphics_entity_store(&(game), (struct pc *) &(game.zombie[i]));

			graphics_entity_store(&(game), &(game.player));

			start_time = 0;
			level_time = SDL_GetTicks();

			for (;;) {
				end_time = SDL_GetTicks();
				game.delta_time =  end_time - start_time;
				start_time = SDL_GetTicks();

				/* Listen to keyboard events. */
				input_handle(&game);

				/* Do not attempt to move player if no actual input has taken place. */
				if (game.player.dir_x != 0 || game.player.dir_y != 0)
					player_move(&game);

				/* Check if we cleared the stage, and start a new level if we did. */
				if (game.level_cleared)
					break;

				/* Calculate paths and move zombies through level. */
				zombie_move(&game);

				/* Update the remaining time since for this stage and check for timeout. */
				game.time = 75 - (end_time - level_time) / 1000;
				//~ if (game.time == 0)
					//~ game.player.dead = true;

				/* Check if we died (by zombie or timeout). */
				if (game.player.dead)
					break;

				/* Update and draw screen elements. */
				graphics_screen_update(&game);

				/* Try to normalize frame-rate to about 60 fps. */
				if (game.delta_time < 16)
					SDL_Delay(16 - game.delta_time);
			}

			/* Level end. */

			if (game.player.dead) {
				game.player.lives--;
				game.player.dead = false;
				/* Remove 200 score for each level */
				game.score -= game.cur_level * 200;
				if (game.score < 0)
					game.score = 0;

				if (game.time == 0)
					printf("Time out!\n");
				else
					printf("You were eaten! Yum!\n");

				if (game.player.lives == 0)
					break;
			} else if (game.level_cleared) {
				/* For each second remaining in the timer, give us 50 points. */
				game.score += game.time * 50;
				/* Give us 1 life every 10000 score. */
				if (game.score / game.score_scale == 1) {
					game.player.lives += game.score / game.score_scale;
					game.score_scale += 10000;
				}
				game.cur_level++;
				printf("Level: %d\n", game.cur_level);
			}
		}

		/* Game session end. */

		printf("Game over, biatch!\n");
	}

	SDL_Quit();
	exit(0);
}
