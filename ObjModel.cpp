
#include "ObjModel.h"

#include <fstream>
#include <cassert>


size_t ObjModel::addPosition(float x, float y, float z) {
	float3 key = { x, y, z };
	auto found = knownPositions.find(key);
	if (found != knownPositions.end()) {
		return found->second;
	}
	const size_t idx = positions.size() / 3;
	positions.push_back(x);
	positions.push_back(y);
	positions.push_back(z);
	knownPositions.insert(std::make_pair(key, idx));
	return idx;
}

size_t ObjModel::addNormal(float x, float y, float z) {
	float3 key = { x, y, z };
	auto found = knownNormals.find(key);
	if (found != knownNormals.end()) {
		return found->second;
	}
	size_t idx = normals.size() / 3;
	normals.push_back(x);
	normals.push_back(y);
	normals.push_back(z);
	knownNormals.insert(std::make_pair(key, idx));
	return idx;
}

void ObjModel::addTri(size_t a, size_t b, size_t c, size_t na, size_t nb, size_t nc) {
	positionIndices.push_back(a);
	positionIndices.push_back(b);
	positionIndices.push_back(c);
	normalIndices.push_back(na);
	normalIndices.push_back(nb);
	normalIndices.push_back(nc);
}

void ObjModel::addAASquare(float x, float y, float z, ObjModel::Direction direction, float hsize) {
	
	// infer normal and tangent/bitangent
	float nx, ny, nz, tx, ty, tz, bx, by, bz;
	nx = ny = nz = tx = ty = tz = bx = by = bz = 0.0f;
	switch (direction) {
	case ObjModel::Direction::POS_X:
		nx = 1.0f;
		ty = 1.0f;
		break;
	case ObjModel::Direction::NEG_X:
		nx = -1.0f;
		ty = 1.0f;
		break;
	case ObjModel::Direction::POS_Y:
		ny = 1.0f;
		tx = 1.0f;
		break;
	case ObjModel::Direction::NEG_Y:
		ny = -1.0f;
		tx = 1.0f;
		break;
	case ObjModel::Direction::POS_Z:
		nz = 1.0f;
		ty = 1.0f;
		break;
	case ObjModel::Direction::NEG_Z:
		nz = -1.0f;
		ty = 1.0f;
		break;
	default:
		assert(false);
		break;
	}

	// cross-product to find bitangent
	bx = ny * tz - nz * ty;
	by = nz * tx - nx * tz;
	bz = nx * ty - ny * tx;

	// add 4 new vertices
	//
	//	  B .______. D
	//		| \	   |
	//		|	\  |
	//	  A ._____\. C
	//
	size_t a = addPosition(x - hsize * tx - hsize * bx, y - hsize * ty - hsize * by, z - hsize * tz - hsize * bz);
	size_t b = addPosition(x + hsize * tx - hsize * bx, y + hsize * ty - hsize * by, z + hsize * tz - hsize * bz);
	size_t c = addPosition(x - hsize * tx + hsize * bx, y - hsize * ty + hsize * by, z - hsize * tz + hsize * bz);
	size_t d = addPosition(x + hsize * tx + hsize * bx, y + hsize * ty + hsize * by, z + hsize * tz + hsize * bz);
	size_t n = addNormal(nx, ny, nz);

	addTri(a, b, c, n, n, n);
	addTri(c, b, d, n, n, n);

}

bool ObjModel::writeToFile(std::string filename) {

	std::ofstream file(filename);

	// Write positions
	for (size_t i = 0, s = positions.size(); i < s; i += 3) {
		file << "v " << positions[i] << ' ' << positions[i + 1] << ' ' << positions[i + 2] << "\n";
	}

	// Write normals
	for (size_t i = 0, s = normals.size(); i < s; i += 3) {
		file << "vn " << normals[i] << ' ' << normals[i + 1] << ' ' << normals[i + 2] << "\n";
	}

	// Write faces
	assert(positionIndices.size() == normalIndices.size());
	for (size_t i = 0, s = positionIndices.size(); i < s; i += 3) {
		file << "f " << positionIndices[i]+1 << "//" << normalIndices[i]+1 << ' '
					 << positionIndices[i + 1]+1 << "//" << normalIndices[i + 1]+1 << ' '
					 << positionIndices[i + 2]+1 << "//" << normalIndices[i + 2]+1 << "\n";
	}

	file.close();

	return true;
}
