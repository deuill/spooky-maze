#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

#include "game.h"
#include "graphics.h"

void clear_entity(game_data *game, SDL_Rect entity)
{
	SDL_FillRect(game->world, &entity, game->black);
}

void draw_level(game_data *game)
{
	int x, y;

	for (y = 0; y < LEVEL_H; y++) {
		for (x = 0; x < LEVEL_W; x++) {
			switch (game->level[y][x]) {
			case 'w': /* Wall tile. */
				game->wall[y][x].w = TILE_SIZE;
				game->wall[y][x].h = TILE_SIZE;
				game->wall[y][x].x = x * TILE_SIZE;
				game->wall[y][x].y = y * TILE_SIZE;

				SDL_FillRect(game->world, &(game->wall[y][x]), game->white);
				break;
			case 'd': /* Closed door. Behaves like a wall for collision purposes. */
				game->wall[y][x].w = TILE_SIZE;
				game->wall[y][x].h = TILE_SIZE;
				game->wall[y][x].x = x * TILE_SIZE;
				game->wall[y][x].y = y * TILE_SIZE;

				/* Set floor tile for the one half. */
				SDL_FillRect(game->world, &(game->wall[y][x]), game->black);

				/* The other half is a door. */
				game->wall[y][x].w = TILE_SIZE / 2;
				game->wall[y][x].x = x * TILE_SIZE + (TILE_SIZE / 2);

				SDL_FillRect(game->world, &(game->wall[y][x]), game->brown);
				break;
			case 'g': /* Goodies should always have floor tiles under them. */
			case 'x': /* This tile is unwalkable by zombies. */
			case '.': /* Floor tile. */
				game->wall[y][x].w = TILE_SIZE;
				game->wall[y][x].h = TILE_SIZE;
				game->wall[y][x].x = x * TILE_SIZE;
				game->wall[y][x].y = y * TILE_SIZE;

				SDL_FillRect(game->world, &(game->wall[y][x]), game->black);
				break;
			default:
				break;
			}
		}
	}
}

void draw_text(game_data *game, const char *text, int pos_x, int pos_y)
{
	int x, y, i;
	SDL_Rect font;
	SDL_Rect offset;

	offset.x = pos_x, offset.y = pos_y;
	font.w = game->graphics.font->w / 10;
	font.h = game->graphics.font->h / 10;

	for (i = 0; text[i] != '\0'; i++) {
		x = (text[i] % 10) * font.w;
		y = ((text[i] / 10) - 3) * font.h;

		font.x = x, font.y = y;

		SDL_BlitSurface(game->graphics.font, &font, game->screen, &offset);
		offset.x += font.w;
	}
}

void load_font(game_data *game)
{
	char font_file[64];

	/* Load font for menus etc. */
	if (SCREEN_H <= 320) {
		sprintf(font_file, "%s%s", DATADIR, "data/graphics/font-320.png");
		game->graphics.font = load_image(font_file);
	} else {
		sprintf(font_file, "%s%s", DATADIR, "data/graphics/font-640.png");
		game->graphics.font = load_image(font_file);
	}
}

SDL_Surface *load_image(const char *filename)
{
	SDL_Surface *tmp, *image;

	/* Load image file in a temporary unoptimized surface. */
	tmp = IMG_Load(filename);
	if (tmp == NULL) {
		printf("Error: Image file '%s' not found!\nExiting...\n", filename);
		terminate(0);
	}

	/* Optimize image and set alpha channel. */
	image = SDL_DisplayFormatAlpha(tmp);
	if (image == NULL) {
		printf("Error: Conversion of image file failed!\nExiting...\n");
		terminate(0);
	} else {
		SDL_SetColorKey(image, SDL_RLEACCEL, image->format->colorkey);
		SDL_FreeSurface(tmp);
	}

	return image;
}

void update_text(game_data *game)
{
	static int goodies, lives, score = 1, time;
	static char goodies_text[16], lives_text[16], score_text[32], time_text[16];

	/* Bottom center: Number of goodies remaining. */
	if (goodies != game->num_goodies) {
		goodies = game->num_goodies;
		if (goodies == 0)
			sprintf(goodies_text, "%s", "Door open!");
		else
			sprintf(goodies_text, "%s%d", "Goodies:", game->num_goodies);
	}

	draw_text(&(*game), goodies_text, (SCREEN_W / 2) - (SCREEN_W / 6), SCREEN_H - TILE_SIZE);

	/* Top left: Number of lives remaining. */
	if (lives != game->lives) {
		lives = game->lives;
		sprintf(lives_text, "%s%d", "Lives:", game->lives);
	}

	draw_text(&(*game), lives_text, SCREEN_W - (SCREEN_W / 4) - 5 , 5);

	/* Top Right: Current score. */
	if (score != game->score) {
		score = game->score;
		sprintf(score_text, "%s%d", "Score:", game->score);
	}

	draw_text(&(*game), score_text, 5 , 5);

	/* Top center: Time remaining. */
	if (time != game->time) {
		time = game->time;
		sprintf(time_text, "%s%d", "Time:", game->time);
	}

	draw_text(&(*game), time_text, (SCREEN_W / 2) - (SCREEN_W / 8), 5);
}

void update_screen(game_data *game)
{
	int i;

	/* Update/animate goodies. */
	for (i = 0; i < game->num_goodies; i++)
		SDL_FillRect(game->world, &(game->goodie[i]), game->blue);

	/* Update/animate zombies. */
	for (i = 0; i < game->num_zombies; i++)
		SDL_FillRect(game->world, &(game->zombie[i].rect), game->yellow);

	/* Update/animate player. */
	SDL_FillRect(game->world, &(game->player), game->red);

	/* Copy from 'world' to 'screen' using 'camera' as a viewport. */
	SDL_BlitSurface(game->world, &game->camera, game->screen, NULL);

	/* Update on-screen info text. */
	update_text(&(*game));

	SDL_Flip(game->screen);
}
