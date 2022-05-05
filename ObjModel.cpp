
#include "ObjModel.h"

#include <fstream>
#include <cassert>


#define PUSH_VERT(x, y, z, nx, ny, nz) \
	do { \
		vertices.push_back(x); \
		vertices.push_back(y); \
		vertices.push_back(z); \
		vertices.push_back(nx); \
		vertices.push_back(ny); \
		vertices.push_back(nz); \
	} while (false)

#define PUSH_TRI(a, b, c) \
	do { \
		indices.push_back(a); \
		indices.push_back(b); \
		indices.push_back(c); \
	} while (false)


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
	size_t idx = vertices.size() / 6;
	PUSH_VERT(x - hsize * tx - hsize * bx, y - hsize * ty - hsize * by, z - hsize * tz - hsize * bz, nx, ny, nz);
	PUSH_VERT(x + hsize * tx - hsize * bx, y + hsize * ty - hsize * by, z + hsize * tz - hsize * bz, nx, ny, nz);
	PUSH_VERT(x - hsize * tx + hsize * bx, y - hsize * ty + hsize * by, z - hsize * tz + hsize * bz, nx, ny, nz);
	PUSH_VERT(x + hsize * tx + hsize * bx, y + hsize * ty + hsize * by, z + hsize * tz + hsize * bz, nx, ny, nz);

	PUSH_TRI(idx, idx + 1, idx + 2);
	PUSH_TRI(idx + 2, idx + 1, idx + 3);

}

bool ObjModel::writeToFile(std::string filename) {

	std::ofstream file(filename);

	// Write positions
	for (size_t i = 0, s = vertices.size(); i < s; i += 6) {
		file << "v " << vertices[i] << ' ' << vertices[i + 1] << ' ' << vertices[i + 2] << "\r\n";
	}

	// Write normals
	for (size_t i = 0, s = vertices.size(); i < s; i += 6) {
		file << "vn " << vertices[i + 3] << ' ' << vertices[i + 4] << ' ' << vertices[i + 5] << "\r\n";
	}

	// Write faces
	for (size_t i = 0, s = indices.size(); i < s; i += 3) {
		file << "f " << indices[i]+1 << "//" << indices[i]+1 << ' ' << indices[i + 1]+1 << "//" << indices[i + 1]+1 << ' ' << indices[i + 2]+1 << "//" << indices[i + 2]+1 << "\r\n";
	}

	file.close();

	return true;
}
