#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	// Size of the map, measured in number of map nodes
	static const uint8_t map_width = 8;
	static const uint8_t map_height = 7;

	// Size (width = height) of the block of pixels corresponding to each map node
	static const uint8_t map_block_size = 32;

	// Map is stored as a row-major array of map nodes
	// Map nodes represent intersections of walkable paths through the map
	// Each node keeps track of which directions you can move in from that node
	// Maps also store whether there is a star on that node, and if not, how long until it respawns
	struct MapNode {
		bool up = false;
		bool down = false;
		bool left = false;
		bool right = false;
		bool star = false;
		float star_timer = 0;
	};
	std::array<MapNode, map_width* map_height> map;

	// Time it takes for a star to respawn after being eaten, chosen randomly from the interval
	const float min_star_spawn_time = 10;
	const float max_star_spawn_time = 50;

	// Used to determine position on map
	struct Position {
		MapNode* a;
		MapNode* b;
		float dist;
	};

	// Cardianl direction, used to describe player movement
	enum Direction {
		dir_none,
		dir_up,
		dir_down,
		dir_left,
		dir_right
	};

	// Unit vectors in each cardinal direction
	const glm::fvec2 right_vec = glm::fvec2(1, 0);
	const glm::fvec2 up_vec = glm::fvec2(0, 1);
	const glm::fvec2 left_vec = glm::fvec2(-1, 0);
	const glm::fvec2 down_vec = glm::fvec2(0, -1);

	// Starting speed of ship
	inline static const float base_speed = 2.f;

	// Struct for player data
	struct Ship {
		PPU466::Sprite* sprite;
		Position pos;
		float speed = base_speed;
		Direction dir = dir_none;
		bool alive = true;
		float death_timer = 0;
		uint8_t size = 8;
	};
	
	// Player object
	Ship player;

	// Multiplier on player speed if they "bonk" (run into a wall)
	const float bonk_penalty = 0.5;

	// Timer to stop action for emphasis
	float stop_timer = 0;

	// How long to stop after a bonk
	float bonk_stop_time = 0.2f;
	float collision_stop_time = 0.4f;

	// Screen shake parameters (may be altered depending on bonk velocity)
	float shake_amplitude = 1;
	float shake_frequency = 32;
	Direction shake_dir = dir_right;

	// How hard to shake the screen when you die
	const float death_shake_duration = 2;
	const float death_shake_amplitude = 2;

	// Array of enemy ships
	std::array<Ship, 2> ufos;

	// How long it takes for a ufo to respawn after death
	float ufo_spawn_time = 15;

	// Score earned by the player this game
	size_t score = 0;

	// Score values for various actions
	size_t star_score = 10;
	size_t kill_score = 100;

	// Pointer to flame sprite in sprite table
	PPU466::Sprite* flame;

	// Multiply by player speed - base speed to get hue of flame
	float color_mult = 50;

	// Helper functions
	MapNode* getMapNode(uint8_t x, uint8_t y);
	MapNode* getMapNode(glm::u8vec2 indices);
	glm::u8vec2 getNodeIndices(MapNode* node);
	glm::u8vec2 getNodeCoords(MapNode* node);
	glm::u8vec2 getPositionCoords(const Position& pos);
	Direction movePosition(Position* pos, glm::fvec2 v, bool is_player, bool* bonk = nullptr);
	Direction getDirection(glm::fvec2 v);
	void collectStar(MapNode* node);
	Direction reverseDir(Direction dir);
	glm::fvec2 getDirVector(Direction dir);
	void reset();

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
