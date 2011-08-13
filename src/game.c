#include <stdio.h>
#include <stdbool.h>
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

int main(void)
{
	game_data game;
	SDL_Surface *tmp;
	Uint32 level_time;
	Uint32 start_time, end_time;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0) {
		printf("Fatal error: %s!\nExiting...\n", SDL_GetError());
		exit(2);
	}

	game.screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, SCREEN_DEPTH,
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

	game.camera.w = SCREEN_W;
	game.camera.h = SCREEN_H;
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
		printf("Game over, biatch!\n");
	}

	SDL_Quit();
	return 0;
}
