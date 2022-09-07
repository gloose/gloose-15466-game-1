#include "read_write_chunk.hpp"
#include <iostream>
//#include "../nest-libs/windows/glm/include/glm/glm.hpp"
#include <glm/glm.hpp>
#include "load_save_png.hpp"
#include "PPU466.hpp"
#include <algorithm>
#include <fstream>
#include <set>
#include "PlayMode.hpp"

int main(int argc, char** argv) {
	{ // Load tile and palette data
		// Array of tile names
		std::vector<std::string> pngs = {
			"blank",
			"ship_up",
			"ship_down",
			"ship_left",
			"ship_right",
			"wall",
			"star",
			"ufo",
			"ufo_dead",
			"flame_up",
			"flame_down",
			"flame_left",
			"flame_right",
			"0", "1", "2", "3", "4",
			"5", "6", "7", "8", "9"
		};

		// Running vectors of palettes and tiles
		std::vector<PPU466::Palette> palettes;
		std::vector<PPU466::Tile> tiles;

		// Generate tiles for all pngs
		for (auto const& png : pngs) {
			// Load png file
			glm::uvec2 size;
			std::vector<glm::u8vec4> data;
			load_png("pngs/" + png + ".png", &size, &data, LowerLeftOrigin);

			// Verify that tiles is 8x8
			if (size[0] != 8 || size[1] != 8) {
				std::cerr << "Tiles should be 8x8" << std::endl;
				exit(1);
			}

			// Initialize an empty palette
			PPU466::Palette pal;
			for (size_t i = 0; i < 4; i++) {
				pal[i] = glm::u8vec4(0, 0, 0, 0);
			}

			// Convert the raw data into a Tile object and a color palette
			// Defined as a lambda so we can repeat it later
			PPU466::Tile tile;
			size_t color_index = 1;  // Index of the next color to insert into the palette
			auto fill_tile = [&]() {
				// Initialize an empty tile
				for (size_t i = 0; i < 8; i++) {
					tile.bit0[i] = 0;
					tile.bit1[i] = 0;
				}

				for (size_t i = 0; i < 8; i++) {
					for (size_t j = 0; j < 8; j++) {
						glm::u8vec4 color = data[i * 8 + j];

						// Get index into palette
						size_t index = std::find(pal.begin(), pal.end(), color) - pal.begin();

						// If color not in palette, add it
						if (index == pal.size()) {
							if (color_index >= 4) {
								std::cerr << "Tiles should not have more than 3 non-transparent colors" << std::endl;
								exit(1);
							}
							pal[color_index] = color;
							index = color_index;
							color_index++;
						}

						// Set tile bits
						if (index & 1) {
							tile.bit0[i] |= 1 << j;
						}
						if (index & 2) {
							tile.bit1[i] |= 1 << j;
						}
					}
				}
			};
			fill_tile();

			// Compares two u8vec4s in lexicographic order
			auto comp = [](const glm::u8vec4& a, const glm::u8vec4& b) -> bool {
				for (glm::length_t i = 0; i < 4; i++) {
					if (a[i] < b[i]) {
						return true;
					}
					if (a[i] > b[i]) {
						return false;
					}
				}
				return false;
			};

			// Sort palette to aid in duplicate detection
			std::sort(pal.begin(), pal.end(), comp);

			// Returns true if the palette  is equal to pal when both are sorted
			auto unordered_eq = [&](const PPU466::Palette& a) -> bool {
				PPU466::Palette sorted_a = a;
				std::sort(sorted_a.begin(), sorted_a.end(), comp);
				return sorted_a == pal;
			};

			// Fill the tile again with full information of the sorted palette
			fill_tile();

			// Get the palette index from palettes
			size_t palette_index = std::find_if(palettes.begin(), palettes.end(), unordered_eq) - palettes.begin();

			// If this palette is not already in the set, insert it
			if (palette_index == palettes.size()) {
				if (palettes.size() < 8) {
					palette_index = palettes.size();
					palettes.push_back(pal);
				}
				else {
					std::cerr << "Only 8 unique palettes are allowed" << std::endl;
					exit(1);
				}
			}

			// Add tile to tile vector
			if (tiles.size() < 256) {
				tiles.push_back(tile);
			}
			else {
				std::cerr << "Only 256 tiles are allowed" << std::endl;
				exit(1);
			}

			// Write sprite object to file
			std::ofstream ofile("dist/assets/" + png + ".spr", std::ios::binary);
			std::vector<PPU466::Sprite> sprite;
			sprite.emplace_back();
			sprite[0].index = (uint8_t)tiles.size() - 1;
			sprite[0].attributes = (uint8_t)palette_index;
			write_chunk("sprt", sprite, &ofile);
			ofile.close();
		}

		// Write file containing palette data
		std::ofstream pfile("dist/assets/palettes.pal", std::ios::binary);
		write_chunk("palt", palettes, &pfile);
		pfile.close();

		// Write file containing tile data
		std::ofstream tfile("dist/assets/tiles.til", std::ios::binary);
		write_chunk("tile", tiles, &tfile);
		tfile.close();
	}

	{ // Load map data
		// Load the png file
		glm::uvec2 size;
		std::vector<glm::u8vec4> data;
		load_png("pngs/map.png", &size, &data, LowerLeftOrigin);
		
		// Verify map size
		uint8_t map_width = 8;
		uint8_t map_height = 7;
		if (size[0] != map_width * 3u || size[1] != map_height * 3u) {
			std::cerr << "map.png should be an 8x7 grid of 3x3 blocks" << std::endl;
		}

		// Populate a vector of map nodes
		std::vector<PlayMode::MapNode> nodes;
		for (uint8_t i = 0; i < map_height; i++) {
			for (uint8_t j = 0; j < map_width; j++) {
				// Coordinates of center of 3x3 block
				uint8_t x = j * 3u + 1;
				uint8_t y = i * 3u + 1;

				// Lambda to check if x,y coordinate on map image is filled in
				auto is_filled = [&data](uint8_t x, uint8_t y) -> size_t {
					return data[y * 24 + x][3] > 0;
				};

				// Construct node
				nodes.emplace_back();
				nodes.back().up = is_filled(x, y + 1);
				nodes.back().down = is_filled(x, y - 1);
				nodes.back().left = is_filled(x - 1, y);
				nodes.back().right = is_filled(x + 1, y);
				nodes.back().star = nodes.back().up || nodes.back().down || nodes.back().left || nodes.back().right;
				PlayMode::MapNode node = nodes.back();
			}
		}

		// Write vector to file
		std::ofstream file("dist/assets/map.map", std::ios::binary);
		write_chunk("map_", nodes, &file);
		file.close();
	}

	return 0;
}