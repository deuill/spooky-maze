#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>
#include <SDL.h>

#include "game.h"
#include "graphics.h"
#include "input.h"
#include "levels.h"
#include "player.h"
#include "zombie.h"

int terminate(int code)
{
	if (code != 0)
		printf("Error: %s\nExiting...\n", SDL_GetError());
	SDL_Quit();
	exit(code);
}

void usage(void)
{
	printf("Usage: spooky-maze -d [datadir]\n");
	printf("\t-d, --datadir:\n\t\tDirectory where data files reside.\n");
	printf("\t-f, --fullscreen:\n\t\tStart game in fullscreen.\n");
	printf("\t-s, --size:\n\t\tSize of game screen (example usage: '-s 800x600').\n");
	printf("\t-h, --help:\n\t\tDisplay this text.\n");
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

	game_data game;
	SDL_Surface *tmp;
	Uint32 level_time;
	Uint32 start_time, end_time;

	/* Process command-line arguments. */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--datadir") == 0 || strcmp(argv[i], "-d") == 0) {
			if (argv[i + 1] == NULL || argv[i + 1][0] == '-') {
				printf("Error: No directory specified after '%s'!\n", argv[i]);
				usage();
			} else if (tmp_dir = opendir(argv[++i])) {
				closedir(tmp_dir);
				game.datadir = argv[i];
			} else {
				closedir(tmp_dir);
				printf("Error: Specified directory does not exist or cannot be accessed!\n");
				usage();
			}
		} else if (strcmp(argv[i], "--fullscreen") == 0 || strcmp(argv[i], "-f") == 0) {
			fullscreen = true;
		} else if (strcmp(argv[i], "--size") == 0 || strcmp(argv[i], "-s") == 0) {
			if ((token = strtok(argv[++i], "x")) != NULL)
				game.screen_w = atoi(token);
			else
				usage();
			
			if ((token = strtok(NULL, "x")) != NULL)
				game.screen_h = atoi(token);
			else
				usage();

			if (game.screen_w == 0 || game.screen_h == 0)
				usage();
		} else {
			usage();
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
	sprintf(dirname, "%s%s", game.datadir, "/levels/");

	tmp_dir = opendir(dirname);
	if (tmp_dir == NULL) {
		printf("Error:\tCould not find data files in '%s'.\n"
		       "\tPlease specify data directory via the '-d' command-line option.\n", dirname);
		exit(1);
	}

	while ((tmp_file = readdir(tmp_dir)) != NULL) {
		if (strncmp("level-", tmp_file->d_name, 6) == 0)
			game.num_levels++;
	}

	closedir(tmp_dir);

	/* Initialize SDL and friends. */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
		printf("Fatal error: %s!\nExiting...\n", SDL_GetError());
		exit(2);
	}

	if (fullscreen)
		game.screen = SDL_SetVideoMode(game.screen_w, game.screen_h, SCREEN_DEPTH,
			      SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
	else
		game.screen = SDL_SetVideoMode(game.screen_w, game.screen_h, SCREEN_DEPTH,
			      SDL_HWSURFACE | SDL_DOUBLEBUF);

	if (game.screen == NULL) {
		printf("Fatal error: %s!\nExiting...\n", SDL_GetError());
		exit(2);
	}

	if (SDL_NumJoysticks() > 0) {
		SDL_JoystickEventState(SDL_ENABLE);
		game.joystick = SDL_JoystickOpen(0);
	}

	tmp = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
				   LEVEL_W * TILE_SIZE, LEVEL_H * TILE_SIZE,
				   SCREEN_DEPTH, 0, 0, 0, 0);

	game.world = SDL_DisplayFormat(tmp);
	SDL_FreeSurface(tmp);

	SDL_WM_SetCaption("Spooky Maze", "spooky-maze");
	SDL_ShowCursor(SDL_DISABLE);

	srand((unsigned int) time(NULL));

	load_font(&game);

	game.camera.w = game.screen_w;
	game.camera.h = game.screen_h;
	game.player.w = ENTITY_SIZE;
	game.player.h = ENTITY_SIZE;
	game.dir_x = 0, game.dir_y = 0;

	/* TEMP */
	game.black = SDL_MapRGB(game.screen->format, 0x00, 0x00, 0x00);
	game.white = SDL_MapRGB(game.screen->format, 0xff, 0xff, 0xff);
	game.red = SDL_MapRGB(game.screen->format, 0xcc, 0x00, 0x00);
	game.blue = SDL_MapRGB(game.screen->format, 0x1f, 0x00, 0xff);
	game.brown = SDL_MapRGB(game.screen->format, 0x77, 0x54, 0x00);
	game.yellow = SDL_MapRGB(game.screen->format, 0xFF, 0xFA, 0x00);

	for (;;) {
		game.lives = 3;
		game.score = 0;
		game.dead = false;
		game.cur_level = 1;
		game.score_scale = 10000;
		game.level_cleared = true;

		game.num_zombies = 6;
		game.num_goodies = 12;

		for (;;) {
			start_time = 0;
			level_time = SDL_GetTicks();

			if (game.level_cleared)
				generate_level(&game);
			else
				clear_level(&game);

			game.level_cleared = false;

			set_goodies(&game);
			set_zombies(&game);
			set_player(&game);
			set_camera(&game);

			draw_level(&game);

			for (;;) {
				end_time = SDL_GetTicks();
				game.delta_time =  end_time - start_time;
				start_time = SDL_GetTicks();

				/* Listen to keyboard events. */
				handle_input(&game);

				/* Do not attempt to move player if no actual input has taken place. */
				if (game.dir_x != 0 || game.dir_y != 0)
					move_player(&game);

				/* Check if we cleared the stage, and start a new level if we did. */
				if (game.level_cleared)
					break;

				/* Calculate paths and move zombies through level. */
				move_zombies(&game);

				/* Update the remaining time since for this stage and check for timeout. */
				game.time = 75 - (end_time - level_time) / 1000;
				if (game.time == 0)
					game.dead = true;

				/* Check if we died (by zombie or timeout). */
				if (game.dead)
					break;

				/* Update and draw screen elements. */
				update_screen(&game);

				/* Try to normalize frame-rate to about 60 fps. */
				if (game.delta_time < 16)
					SDL_Delay(16 - game.delta_time);
			}

			/* Level end. */

			if (game.dead) {
				game.lives--;
				game.dead = false;
				/* Remove 200 score for each level */
				game.score -= game.cur_level * 200;
				if (game.score < 0)
					game.score = 0;

				if (game.time == 0)
					printf("Time out!\n");
				else
					printf("You were eaten! Yum!\n");

				if (game.lives == 0)
					break;
			} else if (game.level_cleared) {
				/* For each second remaining in the timer, give us 50 points. */
				game.score += game.time * 50;
				/* Give us 1 life every 10000 score. */
				if (game.score / game.score_scale == 1) {
					game.lives += game.score / game.score_scale;
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
