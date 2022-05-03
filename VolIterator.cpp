
#include "VolIterator.h"

#include <fstream>
#include <cassert>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "filesystem.h"
#include "colours.h"


VolIterator::VolIterator(std::string filename, size_t width, size_t height, size_t depth) : width(width), height(height), depth(depth) {
	
	file.open(filename, std::ios::binary);
}

bool VolIterator::loadSlice(size_t z) {
	assert(z < depth);

	// For now, throw out all slices currently loaded
	// @todo: keep slices that are still relevant loaded in
	clearSlices();

	// Load single slice
	// @todo: allow loading more than one slice at once
	float* slice = new float[width * height];
	std::streamoff pos = z * width * height * sizeof(float);
	file.seekg(pos);
	if (file.tellg() == -1) {
		printf(RED "Error reading volume file; it might be too large to read currently, make sure to use a 64-bit architecture if possible.\n" WHITE);
		return false;
	}
	currentZ = z;
	file.read((char*)slice, width * height * sizeof(float));
	slices.push_back(slice);

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

VolIterator* VolIterator::Open(std::string filename, size_t width, size_t height, size_t depth) {

	// Check file path
	if (!fs::fileExists(filename)) {
		printf(RED "File %s cannot be found.\n" WHITE, filename.c_str());
		return nullptr;
	}

	// Check file size
	size_t filesize = fs::fileSize(filename);
	size_t expected = width * height * depth * sizeof(float);
	if (filesize < expected) {
		printf(RED "File %s was not found to be the advertised size (should be %llu bytes, found %llu bytes).\n" WHITE, filename.c_str(), expected, filesize);
		return nullptr;
	}

	// Create iterator
	return new VolIterator(filename, width, height, depth);
}

bool VolIterator::exportSlicePng(size_t z, std::string filename, float minThreshold, float maxThreshold) {

	if (z >= depth) {
		printf(RED "Invalid slice %llu on volume of size %llu x %llu x %llu.\n" WHITE, z, width, height, depth);
		return false;
	}

	if (!loadSlice(z)) {
		return false;
	}

	// Convert slice to 8-bit greyscale image
	unsigned char* pixels = new unsigned char[width * height];
	for (size_t y = 0; y < height; ++y) {
		for (size_t x = 0; x < width; ++x) {
			float val = slices[0][y * width + x];
			val = (val - minThreshold) / (maxThreshold - minThreshold); // 0..1 remap
			pixels[y * width + x] = val < 0.0f ? 0 : val > 1.0f ? 255 : int(val * 255); // clamp & write
		}
	}

	// Write out png file
	bool success = stbi_write_png(filename.c_str(), (int)width, (int)height, 1 /* greyscale */, pixels, 0);
	delete[] pixels;
	pixels = nullptr;
	if (!success) {
		printf(RED "Error writing to %llu x %llu png file %s.\n" WHITE, width, height, filename.c_str());
		return false;
	}

	return true;
}
