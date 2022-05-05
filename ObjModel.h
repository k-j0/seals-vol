#pragma once

#include <vector>
#include <string>

// @todo: should flush contents of indices & vertices into the file every now and then to prevent keeping too much in RAM
struct ObjModel {

	enum class Direction {
		POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z
	};

	std::vector<size_t> indices; // stride = 3
	std::vector<float> vertices; // packed positions and normals -> stride = 6

	/// Adds an axis-aligned square face to the model, centred at {x, y, z} with normal along direction, and with half-side-length hsize
	void addAASquare(float x, float y, float z, Direction direction, float hsize);

	/// Writes the obj model out to a file
	bool writeToFile(std::string filename);
};
