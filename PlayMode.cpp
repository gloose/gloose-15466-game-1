#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <fstream>
#include "read_write_chunk.hpp"
#include "../nest-libs/windows/glm/include/glm/glm.hpp"
#include "Load.hpp"
#include <string>
#include "../nest-libs/windows/glm/include/glm/gtx/color_space.hpp"

PPU466::Sprite* load_sprite(std::string name) {
	// Load sprite data from file
	std::ifstream ifile("assets/" + name + ".spr", std::ios::binary);
	std::vector<PPU466::Sprite> sprite;
	read_chunk(ifile, "sprt", &sprite);
	ifile.close();
	return new PPU466::Sprite(sprite[0]);
}

// Load all sprite assets
Load<PPU466::Sprite> blank_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("blank"); });
Load<PPU466::Sprite> ship_up_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("ship_up"); });
Load<PPU466::Sprite> ship_down_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("ship_down"); });
Load<PPU466::Sprite> ship_left_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("ship_left"); });
Load<PPU466::Sprite> ship_right_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("ship_right"); });
Load<PPU466::Sprite> wall_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("wall"); });
Load<PPU466::Sprite> star_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("star"); });
Load<PPU466::Sprite> ufo_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("ufo"); });
Load<PPU466::Sprite> ufo_dead_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("ufo_dead"); });
Load<PPU466::Sprite> flame_up_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("flame_up"); });
Load<PPU466::Sprite> flame_down_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("flame_down"); });
Load<PPU466::Sprite> flame_left_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("flame_left"); });
Load<PPU466::Sprite> flame_right_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("flame_right"); });
Load<PPU466::Sprite> zero_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("0"); });
Load<PPU466::Sprite> one_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("1"); });
Load<PPU466::Sprite> two_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("2"); });
Load<PPU466::Sprite> three_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("3"); });
Load<PPU466::Sprite> four_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("4"); });
Load<PPU466::Sprite> five_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("5"); });
Load<PPU466::Sprite> six_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("6"); });
Load<PPU466::Sprite> seven_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("7"); });
Load<PPU466::Sprite> eight_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("8"); });
Load<PPU466::Sprite> nine_sprite(LoadTagDefault, [&]() -> PPU466::Sprite* { return load_sprite("9"); });

// Load palette table data from assets/palettes.pal
Load<std::vector<PPU466::Palette>> load_palette_table(LoadTagDefault, [&]() -> std::vector<PPU466::Palette>* {
	std::ifstream ifile("assets/palettes.pal", std::ios::binary);
	std::vector<PPU466::Palette>* palettes = new std::vector<PPU466::Palette>();
	read_chunk(ifile, "palt", palettes);
	ifile.close();
	return palettes;
});

// Load tile table data from assets/tiles.til
Load<std::vector<PPU466::Tile>> load_tile_table(LoadTagDefault, [&]() -> std::vector<PPU466::Tile>* {
	std::ifstream ifile("assets/tiles.til", std::ios::binary);
	std::vector<PPU466::Tile>* tiles = new std::vector<PPU466::Tile>();
	read_chunk(ifile, "tile", tiles);
	ifile.close();
	return tiles;
});

Load<std::vector<PlayMode::MapNode>> load_map(LoadTagDefault, [&]() -> std::vector<PlayMode::MapNode>* {
	std::ifstream ifile("assets/map.map", std::ios::binary);
	std::vector<PlayMode::MapNode>* nodes = new std::vector<PlayMode::MapNode>();
	read_chunk(ifile, "map_", nodes);
	ifile.close();
	return nodes;
});

// Get the map node at a given x, y index into map
PlayMode::MapNode* PlayMode::getMapNode(uint8_t x, uint8_t y) {
	return &map[(y % map_height) * map_width + (x % map_width)];
}

// Overload of above with u8vec2 input
PlayMode::MapNode* PlayMode::getMapNode(glm::u8vec2 indices) {
	return getMapNode(indices[0], indices[1]);
}

// Get x,y indices into map for a given map node
glm::u8vec2 PlayMode::getNodeIndices(MapNode* node) {
	uint8_t i = (uint8_t)(node - map.data());
	uint8_t x = i % map_width;
	uint8_t y = i / map_width;
	return glm::u8vec2(x, y);
}

// Get xy pixel coordinates of a map node
glm::u8vec2 PlayMode::getNodeCoords(MapNode* node) {
	glm::u8vec2 i = getNodeIndices(node);
	return glm::u8vec2(i[0] * map_block_size + map_block_size / 2, i[1]* map_block_size + map_block_size / 2);
}

// Get xy pixel coordinates of a position struct
glm::u8vec2 PlayMode::getPositionCoords(const Position& pos) {
	glm::i8vec2 p1 = getNodeCoords(pos.a);
	glm::i8vec2 p2 = getNodeCoords(pos.b);
	return p1 + (glm::i8vec2)((glm::fvec2)(p2 - p1) * pos.dist);
}

// Get the direction enum most aligned with the given vector
PlayMode::Direction PlayMode::getDirection(glm::fvec2 v) {
	std::array<float, 4> dots = {
		glm::dot(v, right_vec),
		glm::dot(v, up_vec),
		glm::dot(v, left_vec),
		glm::dot(v, down_vec)
	};
	size_t maxi = std::max_element(dots.begin(), dots.end()) - dots.begin();
	switch (maxi) {
	case 0:
		return dir_right;
	case 1:
		return dir_up;
	case 2:
		return dir_left;
	case 3:
		return dir_down;
	default:
		return dir_none;
	}
}

// Eat the star on the given node
void PlayMode::collectStar(MapNode* node) {
	if (node->star) {
		node->star = false;
		node->star_timer = min_star_spawn_time + ((float)rand() / (float)RAND_MAX) * (max_star_spawn_time - min_star_spawn_time);
		player.speed += 0.1f;
		score += star_score;
	}
}

// Get the reverse of a direction enum
PlayMode::Direction PlayMode::reverseDir(Direction dir) {
	switch (dir) {
	case dir_up:
		return dir_down;
	case dir_down:
		return dir_up;
	case dir_left:
		return dir_right;
	case dir_right:
		return dir_left;
	default:
		return dir_none;
	}
}

// Change a given position struct by a given input velocity, following map movement rules
PlayMode::Direction PlayMode::movePosition(Position* pos, glm::fvec2 v, bool is_player, bool* bonk) {
	glm::i8vec2 p1 = getNodeIndices(pos->a);
	glm::i8vec2 p2 = getNodeIndices(pos->b);

	// Vector from a to b
	glm::i8vec2 delta = p2 - p1;

	// In case of screen wrap, adjust delta to still represent direction of edge
	if (p1[0] == 0 && p2[0] == map_width - 1) {
		delta = glm::i8vec2(-1, 0);
	} else if (p1[0] == map_width - 1 && p2[0] == 0) {
		delta = glm::i8vec2(1, 0);
	} else if (p1[1] == 0 && p2[1] == map_height - 1) {
		delta = glm::i8vec2(0, -1);
	}
	else if (p1[1] == map_height - 1 && p2[1] == 0) {
		delta = glm::i8vec2(0, 1);
	}

	// Verify that a and b nodes are adjacent
	assert((delta == glm::i8vec2(1, 0) || delta == glm::i8vec2(-1, 0) || delta == glm::i8vec2(0, 1) || delta == glm::i8vec2(0, -1)) && "Position must consist of adjacent map nodes");

	// Set new position, assuming movement along a single edge
	Position new_pos;
	new_pos.a = pos->a;
	new_pos.b = pos->b;
	float move = glm::dot(v, (glm::fvec2)delta);
	new_pos.dist = pos->dist + move;

	// Set final movement direction (may be overwritten in overflow case)
	Direction dir = getDirection((glm::fvec2)delta * move);

	// Until proven otherwise, no bonk occurred
	if (bonk != nullptr) {
		*bonk = false;
	}

	// Handle dist overflow outside of the range [0, 1]
	// In such cases, position should change to an adjacent edge
	while (new_pos.dist < 0 || new_pos.dist > 1) {
		// Compute new starting node (a) and initial guess of ending node (b)
		// Updates dist accordingly
		if (new_pos.dist < 0) {
			// Overflow by crossing pos->a
			new_pos.a = pos->a;
			new_pos.b = pos->b;
			new_pos.dist *= -1;
			if (is_player) {
				collectStar(pos->a);
			}
		}
		else {
			// Overflow by crossing pos->b
			new_pos.a = pos->b;
			new_pos.b = pos->a;
			new_pos.dist -= 1;
			if (is_player) {
				collectStar(pos->b);
			}
		}

		// Choose the new direction based on input velocity
		// Prioritizes changing direction if the input is given
		float e = 1e-4f;
		if (abs(glm::dot((glm::fvec2)delta, right_vec)) > e) {
			// Previous edge was horizontal

			if (glm::dot(v, up_vec) > e && new_pos.a->up) {
				// Turn up
				new_pos.b = getMapNode((glm::i8vec2)getNodeIndices(new_pos.a) + (glm::i8vec2)up_vec);
				dir = dir_up;
			}
			else if (glm::dot(v, up_vec) < -e && new_pos.a->down) {
				// Turn down
				new_pos.b = getMapNode((glm::i8vec2)getNodeIndices(new_pos.a) - (glm::i8vec2)up_vec);
				dir = dir_down;
			}
			else if (glm::dot(v, right_vec) > e && new_pos.a->right) {
				// Turn right
				new_pos.b = getMapNode((glm::i8vec2)getNodeIndices(new_pos.a) + (glm::i8vec2)right_vec);
				dir = dir_right;
			}
			else if (glm::dot(v, right_vec) < -e && new_pos.a->left) {
				// Turn left
				new_pos.b = getMapNode((glm::i8vec2)getNodeIndices(new_pos.a) - (glm::i8vec2)right_vec);
				dir = dir_left;
			}
			else {
				// Nowhere to turn
				dir = reverseDir(dir);

				// Indicate that a bonk occurred
				if (bonk != nullptr) {
					*bonk = true;
				}
			}
		}
		else {
			// Previous edge was vertical

			if (glm::dot(v, right_vec) > e && new_pos.a->right) {
				// Turn right
				new_pos.b = getMapNode((glm::i8vec2)getNodeIndices(new_pos.a) + (glm::i8vec2)right_vec);
				dir = dir_right;
			}
			else if (glm::dot(v, right_vec) < -e && new_pos.a->left) {
				// Turn left
				new_pos.b = getMapNode((glm::i8vec2)getNodeIndices(new_pos.a) - (glm::i8vec2)right_vec);
				dir = dir_left;
			}
			else if (glm::dot(v, up_vec) > e && new_pos.a->up) {
				// Turn up
				new_pos.b = getMapNode((glm::i8vec2)getNodeIndices(new_pos.a) + (glm::i8vec2)up_vec);
				dir = dir_up;
			}
			else if (glm::dot(v, up_vec) < -e && new_pos.a->down) {
				// Turn down
				new_pos.b = getMapNode((glm::i8vec2)getNodeIndices(new_pos.a) - (glm::i8vec2)up_vec);
				dir = dir_down;
			}
			else {
				// Nowhere to turn
				dir = reverseDir(dir);

				// Indicate that a bonk occurred
				if (bonk != nullptr) {
					*bonk = true;
				}
			}
		}
	}

	*pos = new_pos;

	return dir;
}

// Convert a direction enum to a unit vector
glm::fvec2 PlayMode::getDirVector(Direction dir) {
	switch (dir) {
	case dir_up:
		return glm::fvec2(0, 1);
	case dir_down:
		return glm::fvec2(0, -1);
	case dir_right:
		return glm::fvec2(1, 0);
	case dir_left:
		return glm::fvec2(-1, 0);
	default:
		return glm::fvec2(0, 0);
	}
}

void PlayMode::reset() {
	// Populate tile and palette tables
	std::copy(load_tile_table->begin(), load_tile_table->end(), ppu.tile_table.data());
	std::copy(load_palette_table->begin(), load_palette_table->end(), ppu.palette_table.data());

	// Populate map
	std::copy(load_map->begin(), load_map->end(), map.data());

	// Set player sprite
	ppu.sprites[map.size()] = *ship_right_sprite;
	player.sprite = &ppu.sprites[map.size()];

	// Set flame sprite
	ppu.sprites[map.size() + 1] = *flame_up_sprite;
	flame = &ppu.sprites[map.size() + 1];

	// Set the player's initial position
	player.pos.a = getMapNode(3, 0);
	player.pos.b = getMapNode(4, 0);
	player.pos.dist = 0.5;

	// Initialize other player fields
	player.alive = true;
	player.dir = dir_none;
	player.speed = base_speed;

	// Empty the background
	memset(ppu.background.data(), 0, ppu.background.size() * sizeof(uint16_t));

	// Initialize map
	for (uint8_t i = 0; i < map_height; i++) {
		for (uint8_t j = 0; j < map_width; j++) {
			// MapNode object
			MapNode& node = map[i * map_width + j];

			// Array indicating which tiles in the block should have walls
			std::array<bool, 4 * 4> walls = {
				true, !node.down, !node.down, true,
				!node.left, false, false, !node.right,
				!node.left, false, false, !node.right,
				true, !node.up, !node.up, true,
			};

			// Add walls to background
			for (uint8_t y = 0; y < 4; y++) {
				for (uint8_t x = 0; x < 4; x++) {
					uint8_t x_total = j * 4u + x;
					uint8_t y_total = i * 4u + y;
					if (walls[y * 4 + x]) {
						ppu.background[y_total * PPU466::BackgroundWidth + x_total] = wall_sprite->index | (wall_sprite->attributes << 8);
					}
				}
			}
		}
	}

	// Initialize ufos
	for (size_t i = 0; i < ufos.size(); i++) {
		ppu.sprites[map.size() + 2 + i] = *ufo_sprite;
		ufos[i].sprite = &ppu.sprites[map.size() + 2 + i];
		ufos[i].alive = true;
	}

	// Set initial position for first ufo
	ufos[0].pos.a = getMapNode(0, map_height - 1);
	ufos[0].pos.b = getMapNode(1, map_height - 1);
	ufos[0].pos.dist = 0;
	ufos[0].dir = dir_right;

	// Set initial position for second ufo
	ufos[1].pos.a = getMapNode(map_width - 1, map_height - 1);
	ufos[1].pos.b = getMapNode(map_width - 2, map_height - 1);
	ufos[1].pos.dist = 0;
	ufos[1].dir = dir_left;

	// Zero out score (necessary if not the first reset)
	score = 0;
}

PlayMode::PlayMode() {
	reset();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_r) {
			// To prevent accidental resetting on a good run, only allow it if the player is dead or at very low speed
			if (!player.alive || player.speed < base_speed) {
				reset();
			}
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	// Don't update while stopped, and shake screen
	if (stop_timer > 0) {
		stop_timer -= elapsed;
		if (stop_timer > 0) {
			ppu.background_position = (glm::ivec2)(shake_amplitude * sin(stop_timer * shake_frequency) * getDirVector(shake_dir));
			return;
		}
	}
	ppu.background_position[0] = 0;

	if (!player.alive) {
		return;
	}

	// Given a set of inputs, determine the velocity that a ship "wants to move", ignoring walls
	auto getInputVelocity = [&](Ship& ship, bool right, bool up, bool left, bool down) -> glm::fvec2 {
		glm::fvec2 v = { 0, 0 };
		if (ship.dir == dir_left || (left && ship.dir != dir_right)) {
			v[0] -= ship.speed * elapsed;
		}
		if (ship.dir == dir_right || (right && ship.dir != dir_left)) {
			v[0] += ship.speed * elapsed;
		}
		if (ship.dir == dir_down || (down && ship.dir != dir_up)) {
			v[1] -= ship.speed * elapsed;
		}
		if (ship.dir == dir_up || (up && ship.dir != dir_down)) {
			v[1] += ship.speed * elapsed;
		}
		return v;
	};

	// Update player state
	if (player.alive) {
		// Move player and check for bonk
		glm::fvec2 v = getInputVelocity(player, right.pressed, up.pressed, left.pressed, down.pressed);
		if (glm::length(v) > 0) {
			bool bonk = false;
			player.dir = movePosition(&player.pos, v, true, &bonk);
			if (bonk) {
				stop_timer = bonk_stop_time;
				shake_amplitude = player.speed;
				shake_dir = player.dir;
				player.speed *= bonk_penalty;
			}
		}

		// Check for ufo collision
		glm::u8vec2 pxy = getPositionCoords(player.pos);
		for (auto& ufo : ufos) {
			// Only check collisions with living ufos
			if (!ufo.alive) {
				continue;
			}

			// Do the collision test
			glm::u8vec2 uxy = getPositionCoords(ufo.pos);
			if (pxy[0] + player.size > uxy[0] &&
				pxy[0] < uxy[0] + ufo.size &&
				pxy[1] + player.size > uxy[1] &&
				pxy[1] < uxy[1] + ufo.size)
			{
				// Player loses speed on collision
				player.speed -= ufo.speed;

				if (player.speed <= 0) {
					// If player speed drops to 0, they die
					stop_timer = death_shake_duration;
					shake_amplitude = death_shake_amplitude;
					shake_dir = dir_right;
					player.alive = false;
				}
				else {
					// Otherwise, the ufo dies
					stop_timer = collision_stop_time;
					shake_amplitude = player.speed;
					shake_dir = player.dir;
					ufo.alive = false;
					ufo.death_timer = ufo_spawn_time;
					score += kill_score;
				}
			}
		}
	}

	// Update ufos
	for (auto& ufo : ufos) {
		// If ufo is dead, update death timer and skip this iteration if still dead
		if (!ufo.alive) {
			ufo.death_timer -= elapsed;
			if (ufo.death_timer > 0) {
				continue;
			}
			else {
				ufo.alive = true;
			}
		}

		glm::fvec2 v;

		// Coin-flip for chasing or random ai
		if (rand() % 2) {
			// Chase the player (stupidly, with no path-finding or accounting for screen loop)
			v = getInputVelocity(ufo, player.sprite->x > ufo.sprite->x, player.sprite->y > ufo.sprite->y, player.sprite->x < ufo.sprite->x, player.sprite->y < ufo.sprite->y);
		}
		else {
			// Random inputs
			v = getInputVelocity(ufo, rand() % 2, rand() % 2, rand() % 2, rand() % 2);
		}
		
		// Move ufo position
		if (glm::length(v) > 0) {
			ufo.dir = movePosition(&ufo.pos, v, false, nullptr);
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	// Update star respawn timers
	for (auto& node : map) {
		if (node.up || node.down || node.left || node.right) {
			node.star_timer -= elapsed;
			if (node.star_timer <= 0) {
				node.star = true;
			}
		}
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	// Player and flame rotation sprites
	switch (player.dir) {
	case dir_up:
		*player.sprite = *ship_up_sprite;
		*flame = *flame_up_sprite;
		break;
	case dir_down:
		*player.sprite = *ship_down_sprite;
		*flame = *flame_down_sprite;
		break;
	case dir_left:
		*player.sprite = *ship_left_sprite;
		*flame = *flame_left_sprite;
		break;
	case dir_right:
		*player.sprite = *ship_right_sprite;
		*flame = *flame_right_sprite;
		break;
	default:
		*player.sprite = *ship_up_sprite;
		*flame = *flame_up_sprite;
		break;
	}

	{ // Player sprite
		glm::u8vec2 xy = (glm::ivec2)getPositionCoords(player.pos) + ppu.background_position;
		player.sprite->x = xy[0] - 4;
		player.sprite->y = xy[1] - 4;
	}

	// Flame sprite
	if (player.dir == dir_none) {
		flame->x = 240;
	}
	else {
		// Put flame behind player
		glm::u8vec2 flame_offset = getDirVector(player.dir) * -8.f;
		flame->x = player.sprite->x + flame_offset[0];
		flame->y = player.sprite->y + flame_offset[1];

		// Convert hsv triple to u8vec4 rgb(a), with a always 255
		auto hsvToRgb = [](float h, uint8_t s, uint8_t v) -> glm::u8vec4 {
			glm::tvec3<double> rgb = glm::rgbColor(glm::tvec3<double>(std::fmaxf(h, 0), (double)s / 100, (double)v / 100));
			return glm::u8vec4(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255, 255);
		};

		// First palette color is transparent
		ppu.palette_table[flame->attributes][0] = glm::u8vec4(0, 0, 0, 0);

		// Set other colors based on the player's speed
		float base_hue = std::fminf((player.speed - base_speed) * color_mult, 180.f);
		ppu.palette_table[flame->attributes][1] = hsvToRgb(base_hue + 40, 100, 100);
		ppu.palette_table[flame->attributes][2] = hsvToRgb(base_hue + 20, 100, 100);
		ppu.palette_table[flame->attributes][3] = hsvToRgb(base_hue, 100, 100);
	}

	// Star sprites
	for (uint8_t i = 0; i < map.size(); i++) {
		MapNode* node = &map[i];
		if (node->star) {
			ppu.sprites[i] = *star_sprite;
		}
		else {
			ppu.sprites[i] = *blank_sprite;
		}
		glm::u8vec2 xy = (glm::ivec2)getNodeCoords(node) + ppu.background_position;
		ppu.sprites[i].x = xy[0] - 4;
		ppu.sprites[i].y = xy[1] - 4;
	}

	// Ufo sprites
	for (auto& ufo : ufos) {
		if (ufo.alive || (ufo.death_timer < 5 && (int)(ufo.death_timer * 2) % 2)) {
			*ufo.sprite = *ufo_sprite;
		}
		else {
			*ufo.sprite = *ufo_dead_sprite;
		}
		glm::u8vec2 xy = (glm::ivec2)getPositionCoords(ufo.pos) + ppu.background_position;
		ufo.sprite->x = xy[0] - 4;
		ufo.sprite->y = xy[1] - 4;
	}

	// Score sprites
	std::string score_str = std::to_string(score);
	for (size_t i = 0; i < score_str.size(); i ++) {
		char chr = score_str[i];
		size_t bg = (ppu.ScreenHeight / 8 - 1) * ppu.BackgroundWidth + i;
		PPU466::Sprite num_spr = *zero_sprite;
		switch (chr) {
		case '0':
			num_spr = *zero_sprite;
			break;
		case '1':
			num_spr = *one_sprite;
			break;
		case '2':
			num_spr = *two_sprite;
			break;
		case '3':
			num_spr = *three_sprite;
			break;
		case '4':
			num_spr = *four_sprite;
			break;
		case '5':
			num_spr = *five_sprite;
			break;
		case '6':
			num_spr = *six_sprite;
			break;
		case '7':
			num_spr = *seven_sprite;
			break;
		case '8':
			num_spr = *eight_sprite;
			break;
		case '9':
			num_spr = *nine_sprite;
			break;
		default:
			assert(false && "score_str should be a string of numeric characters");
		}
		ppu.background[bg] = num_spr.index | (num_spr.attributes << 8);
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
