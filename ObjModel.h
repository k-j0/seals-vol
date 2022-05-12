#pragma once

#include <vector>
#include <string>
#include <unordered_map>

// @todo: should allow flushing contents of indices & vertices into the file every now and then to prevent keeping too much in RAM
struct ObjModel {

	enum class Direction {
		POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z
	};

	// Stride = 3 on all 4 arrays below
	std::vector<size_t> positionIndices;
	std::vector<float> positions;
	std::vector<size_t> normalIndices;
	std::vector<float> normals;

	struct float3 {
		float x, y, z;
		inline bool operator==(const float3& other) const {
			return x == other.x && y == other.y && z == other.z;
		}
	};
	struct float3_hash {
		inline std::size_t operator()(const float3& k) const {
			auto hash = std::hash<float>();
			return ((hash(k.x) ^ (hash(k.y) << 1)) >> 1) ^ (hash(k.z) << 1);
		}
	};

	// Maps position -> index in positions
	std::unordered_map<float3, size_t, float3_hash> knownPositions;
	// Maps normal -> index in normals
	std::unordered_map<float3, size_t, float3_hash> knownNormals;

	/// Adds a vertex to the mesh with a position and normal and return the position and normal indices
	size_t addPosition(float x, float y, float z);
	size_t addNormal(float x, float y, float z);
	void addTri(size_t a, size_t b, size_t c, size_t na, size_t nb, size_t nc);

	/// Adds an axis-aligned square face to the model, centred at {x, y, z} with normal along direction, and with half-side-length hsize
	void addAASquare(float x, float y, float z, Direction direction, float hsize);

	/// Writes the obj model out to a file
	bool writeToFile(std::string filename);
};
