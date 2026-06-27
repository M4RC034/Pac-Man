#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include "ghost.h"
#include "map.h"
#include "pacman_obj.h"
#include "shared.h"

/* global variables*/
// [ NOTE ]
// if you change the map .txt to your own design.
// You have to modify cage_grid_{x,y} to corressponding value also.
// Or you can do some change while loading map (reading .txt file)
// Make the start position metadata stored with map.txt.
const int cage_grid_x=22, cage_grid_y=11;

/* shared variables. */
extern uint32_t GAME_TICK;
extern uint32_t GAME_TICK_CD;
extern const int block_width,  block_height;
/* Internal variables */
static const int fix_draw_pixel_offset_x = -3;
static const int fix_draw_pixel_offset_y = -3;
static const int draw_region = 30;
// [ NOTE - speed again ]
// Again, you see this notaficationd. If you still want to implement something 
// fancy with speed, objData->moveCD and GAME_TICK, you can first start on 
// working on animation of ghosts and pacman. // Once you finished the animation 
// part, you will have more understanding on whole mechanism.
// Base ghost speed is chosen per difficulty (shared.h / scene_game init).

Ghost* ghost_create(int flag) {

	// NOTODO
	Ghost* ghost = (Ghost*)malloc(sizeof(Ghost));
	if (!ghost)
		return NULL;

	ghost->typeFlag = flag;
	ghost->objData.Size.x = block_width;
	ghost->objData.Size.y = block_height;

	ghost->objData.nextTryMove = NONE;
	ghost->speed = ghost_base_speed;
	ghost->status = BLOCKED;
	// Two AI modes alternate by type: Blinky(0)/Inky(2) hunt via shortest path,
	// Pinky(1)/Clyde(3) wander randomly.
	ghost->ai_mode = (flag % 2 == 0) ? AI_CHASE : AI_RANDOM;

	ghost->flee_sprite = load_bitmap("Assets/ghost_flee.png");
	ghost->dead_sprite = load_bitmap("Assets/ghost_dead.png");

	// Each type gets its own colour and a distinct spot inside the cage so
	// they don't all overlap. They keep y == cage_grid_y (centre row) so the
	// BLOCKED bobbing stays inside the room. All share the random red script.
	switch (ghost->typeFlag) {
	case Pinky:
		ghost->objData.Coord.x = cage_grid_x - 1;
		ghost->objData.Coord.y = cage_grid_y;
		ghost->move_sprite = load_bitmap("Assets/ghost_move_pink.png");
		break;
	case Inky:
		ghost->objData.Coord.x = cage_grid_x + 1;
		ghost->objData.Coord.y = cage_grid_y;
		ghost->move_sprite = load_bitmap("Assets/ghost_move_blue.png");
		break;
	case Clyde:
		ghost->objData.Coord.x = cage_grid_x;
		ghost->objData.Coord.y = cage_grid_y + 1;
		ghost->move_sprite = load_bitmap("Assets/ghost_move_orange.png");
		break;
	case Blinky:
	default:
		ghost->objData.Coord.x = cage_grid_x;
		ghost->objData.Coord.y = cage_grid_y;
		ghost->move_sprite = load_bitmap("Assets/ghost_move_red.png");
		break;
	}
	ghost->move_script = &ghost_red_move_script;
	return ghost;
}
void ghost_destory(Ghost* ghost) {
	if (!ghost)
		return;
	if (ghost->move_sprite)
		al_destroy_bitmap(ghost->move_sprite);
	if (ghost->flee_sprite)
		al_destroy_bitmap(ghost->flee_sprite);
	if (ghost->dead_sprite)
		al_destroy_bitmap(ghost->dead_sprite);
	free(ghost);
}
void ghost_draw(Ghost* ghost) {
	// getDrawArea return the drawing RecArea defined by objData and GAME_TICK_CD
	RecArea drawArea = getDrawArea(ghost->objData, GAME_TICK_CD);

	const float dx = drawArea.x + fix_draw_pixel_offset_x;
	const float dy = drawArea.y + fix_draw_pixel_offset_y;

	if (ghost->status == FLEE) {
		// Frightened: blue ghost, alternate the two blue frames.
		int frame = (ghost->objData.moveCD % 16) < 8 ? 0 : 1;
		al_draw_scaled_bitmap(ghost->flee_sprite, frame * 16, 0, 16, 16,
			dx, dy, draw_region, draw_region, 0);
		return;
	}

	// Pick the sprite column for the facing direction.
	// move_sprite: two frames per direction -> [base]/[base+1] animate the feet.
	// dead_sprite: one frame per direction (eyes only).
	int move_base, dead_frame;
	switch (ghost->objData.facing) {
	case RIGHT: move_base = 0; dead_frame = 0; break;
	case LEFT:  move_base = 2; dead_frame = 1; break;
	case UP:    move_base = 4; dead_frame = 2; break;
	case DOWN:  move_base = 6; dead_frame = 3; break;
	default:    move_base = 0; dead_frame = 0; break;
	}

	if (ghost->status == GO_IN) {
		// Eaten: only the eyes travel back to the room.
		al_draw_scaled_bitmap(ghost->dead_sprite, dead_frame * 16, 0, 16, 16,
			dx, dy, draw_region, draw_region, 0);
		return;
	}

	// Normal (BLOCKED / GO_OUT / FREEDOM): directional sprite with a walk cycle.
	int frame = move_base + ((ghost->objData.moveCD % 16) < 8 ? 0 : 1);
	al_draw_scaled_bitmap(ghost->move_sprite, frame * 16, 0, 16, 16,
		dx, dy, draw_region, draw_region, 0);
}
void ghost_NextMove(Ghost* ghost, Directions next) {
	ghost->objData.nextTryMove = next;
}
void printGhostStatus(GhostStatus S) {

	switch(S){
	
	case BLOCKED: // stay inside the ghost room
		game_log("BLOCKED");
		break;
	case GO_OUT: // going out the ghost room
		game_log("GO_OUT");
		break;
	case FREEDOM: // free at the map
		game_log("FREEDOM");
		break;
	case GO_IN:
		game_log("GO_IN");
		break;
	case FLEE:
		game_log("FLEE");
		break;
	default:
		game_log("status error");
		break;
	}
}
bool ghost_movable(Ghost* ghost, Map* M, Directions targetDirec, bool room) {
	// [HACKATHON 2-3]
	// Ghost version of `pacman_movable`: a ghost may not move into a wall, and
	// (when `room` is true) it may not move into the ghost room either.
	int target_x = ghost->objData.Coord.x;
	int target_y = ghost->objData.Coord.y;

	switch (targetDirec)
	{
	case UP:
		target_y -= 1;
		break;
	case DOWN:
		target_y += 1;
		break;
	case LEFT:
		target_x -= 1;
		break;
	case RIGHT:
		target_x += 1;
		break;
	default:
		// for none UP, DOWN, LEFT, RIGHT direction u should return false.
		return false;
	}

	if (is_wall_block(M, target_x, target_y) || (room && is_room_block(M, target_x, target_y)))
		return false;
	return true;
}

void ghost_toggle_FLEE(Ghost* ghost, bool setFLEE) {
	// When pacman eats a power bean, only free-roaming ghosts (FREEDOM) become
	// frightened (FLEE). Ghosts in BLOCKED / GO_OUT / GO_IN keep their state.
	if (setFLEE) {
		if (ghost->status == FREEDOM)
			ghost->status = FLEE;
	} else {
		if (ghost->status == FLEE)
			ghost->status = FREEDOM;
	}
}

void ghost_collided(Ghost* ghost) {
	if (ghost->status == FLEE) {
		ghost->status = GO_IN;
		ghost->speed = 4;
	}
}

void ghost_move_script_GO_IN(Ghost* ghost, Map* M) {
	// Description
	// `shortest_path_direc` is a function that returns the direction of shortest path.
	// Check `map.c` for its detail usage.
	// For GO_IN state.
	ghost->objData.nextTryMove = shortest_path_direc(M, ghost->objData.Coord.x, ghost->objData.Coord.y, cage_grid_x, cage_grid_y);
}
void ghost_move_script_GO_OUT(Ghost* ghost, Map* M) {
	// Description
	// Here we always assume the room of ghosts opens upward.
	// And used a greedy method to drag ghosts out of room.
	// You should modify here if you have different implementation/design of room.
	if(M->map[ghost->objData.Coord.y][ghost->objData.Coord.x] == 'B') 
		ghost_NextMove(ghost, UP);
	else
		ghost->status = FREEDOM;
}
void ghost_move_script_FLEE(Ghost* ghost, Map* M, const Pacman * const pacman) {
	// Run away: get the direction of the shortest path toward pacman, then pick
	// any other movable direction so the ghost heads away from him.
	Directions toward = shortest_path_direc(M, ghost->objData.Coord.x, ghost->objData.Coord.y, pacman->objData.Coord.x, pacman->objData.Coord.y);
	Directions proba[4];
	int cnt = 0;
	for (Directions i = 1; i <= 4; i++) {
		if (i == toward)
			continue;
		if (ghost_movable(ghost, M, i, true))
			proba[cnt++] = i;
	}
	if (cnt > 0)
		ghost_NextMove(ghost, proba[generateRandomNumber(0, cnt - 1)]);
	else
		ghost_NextMove(ghost, toward); // cornered: nowhere left to run
}

