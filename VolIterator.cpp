
#include "VolIterator.h"

#include <fstream>
#include <cassert>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "filesystem.h"
#include "colours.h"


VolIterator::VolIterator(std::string filename, size_t width, size_t height, size_t depth, const VolIteratorParams& params) : width(width), height(height), depth(depth), params(params) {
	
	file.open(filename, std::ios::binary);
}

bool VolIterator::loadSlice(size_t z) {

	// Check whether the slice is already loaded in
	if (z >= currentZ && z < currentZ + slices.size()) {
		return true; // already loaded in, no need to progress any further
	}

	assert(z < depth);

	// Stepping backwards or forward by a substantial amount clears the buffers fully and loads just the slice requested
	// The assumption is that this will rarely ever be needed
	if (z < currentZ || z > currentZ + params.loadedNum * 2) {
		clearSlices();
		currentZ = z;
	}

	// Fetch the slices required to fill the gap between currentZ and z
	while (z >= currentZ + slices.size()) {
		float* slice = new float[width * height];
		std::streamoff pos = (std::streamoff)(currentZ + slices.size()) * width * height * sizeof(float);
		file.seekg(pos);
		if (file.tellg() == -1) {
			printf(RED "Error reading volume file; it might be too large to read currently, make sure to use a 64-bit architecture if possible.\n" WHITE);
			return false;
		}
		file.read((char*)slice, (std::streamsize)width * height * sizeof(float));
		slices.push_back(slice);
	}

	// Free up trailing slices that aren't needed anymore
	while (slices.size() > params.loadedNum) {
		++currentZ;
		delete[] slices.front();
		slices.erase(slices.begin());
	}

	return true;
}

VolIterator::~VolIterator() {
	clearSlices();
	file.close();
}

void VolIterator::clearSlices() {
	for (auto& slice : slices) {
		delete[] slice;
		slice = nullptr;
	}
	slices.clear();
}

VolIterator* VolIterator::Open(std::string filename, size_t width, size_t height, size_t depth, const VolIteratorParams& params) {

	// Check file path
	if (!fs::fileExists(filename)) {
		printf(RED "File %s cannot be found.\n" WHITE, filename.c_str());
		return nullptr;
	}

	// Check file size
	size_t filesize = fs::fileSize(filename);
	size_t expected = width * height * depth * sizeof(float);
	if (filesize < expected) {
		printf(RED "File %s was not found to be the advertised size (should be %zu bytes, found %zu bytes).\n" WHITE, filename.c_str(), expected, filesize);
		return nullptr;
	}

	// Create iterator
	return new VolIterator(filename, width, height, depth, params);
}

float VolIterator::getVoxel(size_t x, size_t y, size_t z) {

	if (!loadSlice(z)) {
		return NAN;
	}

	return slices[z - currentZ][y * width + x];
}

bool VolIterator::exportSlicePng(size_t z, std::string filename, float minThreshold, float maxThreshold) {

	if (z >= depth) {
		printf(RED "Invalid slice %zu on volume of size %zu x %zu x %zu.\n" WHITE, z, width, height, depth);
		return false;
	}

	// Convert slice to 8-bit greyscale image
	unsigned char* pixels = new unsigned char[width * height];
	for (size_t y = 0; y < height; ++y) {
		for (size_t x = 0; x < width; ++x) {
			float val = getVoxel(x, y, z);
			if (std::isnan(val)) {
				return false; // could not fetch voxel
			}
			val = (val - minThreshold) / (maxThreshold - minThreshold); // 0..1 remap
			pixels[y * width + x] = val < 0.0f ? 0 : val > 1.0f ? 255 : int(val * 255); // clamp & write
		}
	}

	// Write out png file
	bool success = stbi_write_png(filename.c_str(), (int)width, (int)height, 1 /* greyscale */, pixels, 0);
	delete[] pixels;
	pixels = nullptr;
	if (!success) {
		printf(RED "Error writing to %zu x %zu png file %s.\n" WHITE, width, height, filename.c_str());
		return false;
	}

	return true;
}
