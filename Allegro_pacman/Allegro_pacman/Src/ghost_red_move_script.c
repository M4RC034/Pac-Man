
#include "ghost.h"
#include "pacman_obj.h"
#include "map.h"
#include "shared.h"
/* Shared variables */
#define GO_OUT_TIME 256
extern uint32_t GAME_TICK_CD;
extern uint32_t GAME_TICK;
extern ALLEGRO_TIMER* game_tick_timer;
extern const int cage_grid_x, cage_grid_y;

/* Declare static function prototypes */
static void ghost_red_move_script_FREEDOM(Ghost* ghost, Map* M);
static void ghost_chase_FREEDOM(Ghost* ghost, Map* M, Pacman* pacman);
static void ghost_red_move_script_BLOCKED(Ghost* ghost, Map* M);

static void ghost_chase_FREEDOM(Ghost* ghost, Map* M, Pacman* pacman) {
	// Chase mode: step along the BFS shortest path straight toward pacman.
	Directions toward = shortest_path_direc(M,
		ghost->objData.Coord.x, ghost->objData.Coord.y,
		pacman->objData.Coord.x, pacman->objData.Coord.y);
	if (toward != NONE && ghost_movable(ghost, M, toward, true))
		ghost_NextMove(ghost, toward);
	// Otherwise keep the current heading (the caller falls back to preMove).
}

static void ghost_red_move_script_FREEDOM(Ghost* ghost, Map* M) {
	// [HACKATHON 2-4]
	// Random walk, but avoid reversing so the ghost doesn't pace back and forth.
	Directions back = NONE;
	switch (ghost->objData.preMove) {
	case UP:    back = DOWN;  break;
	case DOWN:  back = UP;    break;
	case LEFT:  back = RIGHT; break;
	case RIGHT: back = LEFT;  break;
	default:    back = NONE;  break;
	}

	Directions proba[4]; // possible movements
	int cnt = 0;
	for (Directions i = 1; i <= 4; i++) {
		if (i == back)
			continue;
		// room = true: a freed ghost must not walk back into the room.
		if (ghost_movable(ghost, M, i, true))
			proba[cnt++] = i;
	}

	if (cnt > 0)
		ghost_NextMove(ghost, proba[generateRandomNumber(0, cnt - 1)]);
	else if (back != NONE)
		ghost_NextMove(ghost, back); // dead-end: the only way out is to turn around
}

static void ghost_red_move_script_BLOCKED(Ghost* ghost, Map* M) {

	switch (ghost->objData.preMove)
	{
	case UP:
		if (ghost->objData.Coord.y == 10)
			ghost_NextMove(ghost, DOWN);
		else
			ghost_NextMove(ghost, UP);
		break;
	case DOWN:
		if (ghost->objData.Coord.y == 12)
			ghost_NextMove(ghost, UP);
		else
			ghost_NextMove(ghost, DOWN);
		break;
	default:
		ghost_NextMove(ghost, UP);
		break;
	}
}

void ghost_red_move_script(Ghost* ghost, Map* M, Pacman* pacman) {
	if (!movetime(ghost->speed))
		return;
		switch (ghost->status)
		{
		case BLOCKED:
			ghost_red_move_script_BLOCKED(ghost, M);
			if (al_get_timer_count(game_tick_timer) > GO_OUT_TIME)
				ghost->status = GO_OUT;
			break;
		case FREEDOM:
			if (ghost->ai_mode == AI_CHASE)
				ghost_chase_FREEDOM(ghost, M, pacman);
			else
				ghost_red_move_script_FREEDOM(ghost, M);
			break;
		case GO_OUT:
			ghost_move_script_GO_OUT(ghost, M);
			break;
		case GO_IN:
			ghost_move_script_GO_IN(ghost, M);
			if (M->map[ghost->objData.Coord.y][ghost->objData.Coord.x] == 'B') {
				ghost->status = GO_OUT;
				ghost->speed = ghost_base_speed;
			}
			break;
		case FLEE:
			ghost_move_script_FLEE(ghost, M, pacman);
			break;
		default:
			break;
		}

		if(ghost_movable(ghost, M, ghost->objData.nextTryMove, false)){
			ghost->objData.preMove = ghost->objData.nextTryMove;
			ghost->objData.nextTryMove = NONE;
		}
		else if (!ghost_movable(ghost, M, ghost->objData.preMove, false))
			return;

		switch (ghost->objData.preMove) {
		case RIGHT:
			ghost->objData.Coord.x += 1;
			break;
		case LEFT:
			ghost->objData.Coord.x -= 1;
			break;
		case UP:
			ghost->objData.Coord.y -= 1;
			break;
		case DOWN:
			ghost->objData.Coord.y += 1;
			break;
		default:
			break;
		}
		ghost->objData.facing = ghost->objData.preMove;
		ghost->objData.moveCD = GAME_TICK_CD;
}