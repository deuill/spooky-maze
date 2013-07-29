#ifndef LEVELS_H
#define LEVELS_H

/* Map tile definitions. These are used in the levels/level-*.txt files
 * to make up the levels for the game. */
#define TILE_FLOOR		'.'
#define TILE_WALL		'w'
#define TILE_DOOR		'd'
#define TILE_EXIT		'e'
#define TILE_GOODIE		'g'
#define TILE_UNWALKABLE	'x'

/* Entity type definitions. */
#define ENTITY_PLAYER	'p'
#define ENTITY_ZOMBIE	'z'
#define ENTITY_GOODIE	'g'

/* 
 * Clear level array from data without regenerating.
 */
void level_clear(struct game_data *game);

/* 
 * Finds if 'entity' intersects 'wall' and returns true or false if it 
 * does or doesn't, respectively.
 */
bool level_collision(SDL_Rect entity, SDL_Rect wall);

/* 
 * Generate random level.
 */
void level_generate(struct game_data *game);

/* 
 * Determine if element in position 'src_x', 'src_y' can see element in position
 * 'dst_x', 'dst_y' and vice versa, using 'level' to determine obstructions.
 * Positions are relative to indices in 'level'. Returns 'true' or 'false',
 * if elements were found to be visible to each other or not, respectively.
 */
int level_tile_visible(int src_x, int src_y, int dest_x, int dest_y, char level[LEVEL_H][LEVEL_W]);

/* 
 * Clears the exit door on the right of the level once certain conditions have been met.
 */
void level_unlock(struct game_data *game);

/* 
 * Place entities (player, zombies, goodies) within the level.
 */
void level_entities_set(struct game_data *game);


#endif
