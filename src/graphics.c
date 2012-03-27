#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

#include "game.h"
#include "graphics.h"
#include "levels.h"

void graphics_entity_clear(struct game_data *game, struct pc *entity)
{
	SDL_Rect tmp;

	tmp.x = entity->iso_x, tmp.y = entity->iso_y;
	tmp.w = entity->rect.w, tmp.h = entity->rect.h;

	SDL_BlitSurface(entity->bg, NULL, game->world, &tmp);
}

void graphics_iso_convert(struct pc *entity)
{
	entity->iso_x = ((TILE_SIZE / 2) * (LEVEL_H - 1)) + ((entity->rect.x / 2) - (entity->rect.y / 2)) + ((TILE_SIZE - entity->rect.w) / 2);
	entity->iso_y = (entity->rect.x / 4) + (entity->rect.y / 4) + ((TILE_SIZE - entity->rect.h) / 2);
}

void graphics_entity_draw(struct game_data *game, const int entity_type, struct pc *entity)
{
	SDL_Rect tmp, offset;

	tmp.x = entity->iso_x, tmp.y = entity->iso_y;
	tmp.w = entity->rect.w, tmp.h = entity->rect.h;

	/* Animation offset within the sprite. */
	offset.x = 0, offset.y = 0;
	offset.w = entity->rect.w, offset.h = entity->rect.h;

	/* Draw entity. */
	switch (entity_type) {
		case ENTITY_PLAYER:
			SDL_BlitSurface(game->graphics.player, &offset, game->world, &tmp);
			break;
		case ENTITY_ZOMBIE:
			SDL_BlitSurface(game->graphics.zombie, &offset, game->world, &tmp);
			break;
		case ENTITY_GOODIE:
			SDL_BlitSurface(game->graphics.goodie, &offset, game->world, &tmp);
			break;
	}
}

void graphics_level_draw(struct game_data *game)
{
	int x, y;
	SDL_Rect tile;

	tile.w = tile.h = TILE_SIZE;

	for (y = 0; y < LEVEL_H; y++) {
		tile.x = (tile.w / 2) * (LEVEL_H - (1 + y));
		tile.y = (tile.h / 4) * y;

		for (x = 0; x < LEVEL_W; x++) {
			switch (game->level[y][x]) {
			case TILE_WALL:
				game->wall[y][x].w = TILE_SIZE;
				game->wall[y][x].h = TILE_SIZE;
				game->wall[y][x].x = x * TILE_SIZE;
				game->wall[y][x].y = y * TILE_SIZE;

				graphics_tile_draw(game, TILE_WALL, tile);
				break;
			case TILE_DOOR: /* Behaves like a wall for collision purposes. */
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
			case TILE_GOODIE: /* Goodies should always have floor tiles under them. */
			case TILE_UNWALKABLE: /* This tile is unwalkable by zombies. */
			case TILE_FLOOR:
			default:
				game->wall[y][x].w = TILE_SIZE;
				game->wall[y][x].h = TILE_SIZE;
				game->wall[y][x].x = x * TILE_SIZE;
				game->wall[y][x].y = y * TILE_SIZE;

				graphics_tile_draw(game, TILE_FLOOR, tile);
				break;
			}

			tile.x += tile.w / 2;
			tile.y += tile.h / 4;
		}
	}
}

void graphics_text_draw(struct game_data *game, const char *text, int pos_x, int pos_y)
{
	int i;
	SDL_Rect font, offset;

	offset.x = pos_x, offset.y = pos_y;
	font.w = game->graphics.font->w / 10;
	font.h = game->graphics.font->h / 10;

	for (i = 0; text[i] != '\0'; i++) {
		font.x = (text[i] % 10) * font.w;
		font.y = ((text[i] / 10) - 3) * font.h;

		SDL_BlitSurface(game->graphics.font, &font, game->screen, &offset);
		offset.x += font.w;
	}
}

void graphics_tile_draw(struct game_data *game, const int tile_type, SDL_Rect tile)
{
	SDL_Rect offset;

	offset.y = 0;
	offset.w = tile.w, offset.h = tile.h;

	switch (tile_type) {
		case TILE_FLOOR:
			offset.x = 0;
			break;
		case TILE_WALL:
			offset.x = tile.w;
			break;
	}

	SDL_BlitSurface(game->graphics.level, &offset, game->world, &tile);
}

SDL_Surface *graphics_surface_init(int width, int height)
{
	SDL_Surface *tmp, *optimized;

	/* Initialize temporary surface. */
	tmp = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCCOLORKEY,
			  width, height, SCREEN_DEPTH, 0, 0, 0, 0);

	/* Optimize surface for rendering. */
	optimized = SDL_DisplayFormat(tmp);
	if (optimized == NULL) {
		printf("Error: Initialization of surface failed!\nExiting...\n");
		game_terminate(0);
	} else {
		SDL_FreeSurface(tmp);
	}

	return optimized;
}

void graphics_assets_load(struct game_data *game)
{
	char tmp_file[256];

	/* Load font for menus etc. */
	if (game->screen_h <= 320) {
		snprintf(tmp_file, 256, "%s%s", game->datadir, "/graphics/font-320.png");
		game->graphics.font = graphics_image_load(tmp_file);
	} else {
		snprintf(tmp_file, 256, "%s%s", game->datadir, "/graphics/font-640.png");
		game->graphics.font = graphics_image_load(tmp_file);
	}

	/* Load tileset for use in levels. */
	snprintf(tmp_file, 256, "%s%s", game->datadir, "/graphics/level.png");
	game->graphics.level = graphics_image_load(tmp_file);

	/* Load sprites for player. */
	snprintf(tmp_file, 256, "%s%s", game->datadir, "/graphics/player.png");
	game->graphics.player = graphics_image_load(tmp_file);

	/* Load sprites for zombies. */
	snprintf(tmp_file, 256, "%s%s", game->datadir, "/graphics/zombie.png");
	game->graphics.zombie = graphics_image_load(tmp_file);

	/* Load sprites for goodies. */
	snprintf(tmp_file, 256, "%s%s", game->datadir, "/graphics/goodie.png");
	game->graphics.goodie = graphics_image_load(tmp_file);
}

SDL_Surface *graphics_image_load(const char *filename)
{
	SDL_Surface *tmp, *image;

	/* Load image file in a temporary unoptimized surface. */
	tmp = IMG_Load(filename);
	if (tmp == NULL) {
		printf("Error: Image file '%s' not found!\nExiting...\n", filename);
		game_terminate(0);
	}

	/* Optimize image and set alpha channel. */
	image = SDL_DisplayFormatAlpha(tmp);
	if (image == NULL) {
		printf("Error: Conversion of image file failed!\nExiting...\n");
		game_terminate(0);
	} else {
		SDL_SetColorKey(image, SDL_RLEACCEL, image->format->colorkey);
		SDL_FreeSurface(tmp);
	}

	return image;
}

void graphics_entity_store(struct game_data *game, struct pc *entity)
{
	SDL_Rect tmp;

	tmp.x = entity->iso_x, tmp.y = entity->iso_y;
	tmp.w = entity->rect.w, tmp.h = entity->rect.h;

	SDL_BlitSurface(game->world, &tmp, entity->bg, NULL);
}

void graphics_text_update(struct game_data *game)
{
	static int goodies, lives, score = 1, time;
	static char goodies_text[16], lives_text[16], score_text[32], time_text[16];

	/* Bottom center: Number of goodies remaining. */
	if (goodies != game->num_goodies) {
		goodies = game->num_goodies;
		if (goodies == 0)
			snprintf(goodies_text, 16, "%s", "Door open!");
		else
			snprintf(goodies_text, 16, "%s%d", "Goodies:", game->num_goodies);
	}

	graphics_text_draw(game, goodies_text, (game->screen_w / 2) - (((game->graphics.font->w / 10) * strlen(goodies_text)) / 2), game->screen_h - 40);

	/* Top left: Number of lives remaining. */
	if (lives != game->player.lives) {
		lives = game->player.lives;
		snprintf(lives_text, 16, "%s%d", "Lives:", game->player.lives);
	}

	graphics_text_draw(game, lives_text, game->screen_w - ((game->graphics.font->w / 10) * strlen(lives_text)) - 5 , 5);

	/* Top Right: Current score. */
	if (score != game->score) {
		score = game->score;
		snprintf(score_text, 16, "%s%d", "Score:", game->score);
	}

	graphics_text_draw(game, score_text, 5 , 5);

	/* Top center: Time remaining. */
	if (time != game->time) {
		time = game->time;
		snprintf(time_text, 16, "%s%d", "Time:", game->time);
	}

	graphics_text_draw(game, time_text, (game->screen_w / 2) - (((game->graphics.font->w / 10) * strlen(time_text)) / 2), 5);
}

void graphics_screen_update(struct game_data *game)
{
	int i;

	/* Clear entities from screen. */
	for (i = 0; i < game->num_goodies; i++)
		graphics_entity_clear(game, (struct pc *) &(game->goodie[i]));

	/* Store entity backgrounds for next time we clear. */
	for (i = 0; i < game->num_zombies; i++)
		graphics_entity_store(game, (struct pc *) &(game->zombie[i]));

	if (game->player.dir_x != 0 || game->player.dir_y != 0)
		graphics_entity_store(game, &(game->player));

	/* Draw entities on screen. */
	for (i = 0; i < game->num_goodies; i++)
		graphics_entity_draw(game, ENTITY_GOODIE, (struct pc *) &(game->goodie[i]));

	for (i = 0; i < game->num_zombies; i++)
		graphics_entity_draw(game, ENTITY_ZOMBIE, (struct pc *) &(game->zombie[i]));

	graphics_entity_draw(game, ENTITY_PLAYER, &(game->player));

	/* Copy from 'world' to 'screen' using 'camera' as a viewport. */
	SDL_BlitSurface(game->world, &game->camera, game->screen, NULL);

	/* Update on-screen info text. */
	graphics_text_update(game);

	SDL_Flip(game->screen);
}
