
#include "VolIterator.h"

#include <fstream>
#include <cassert>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "filesystem.h"


VolIterator::VolIterator(std::string filename, unsigned long long int width, unsigned long long int height, unsigned long long int depth) : width(width), height(height), depth(depth) {
	
	file.open(filename, std::ios::binary);
}

void VolIterator::loadSlice(unsigned long long int z) {
	assert(z < depth);

	// For now, throw out all slices currently loaded
	// @todo: keep slices that are still relevant loaded in
	clearSlices();

	// Load single slice
	// @todo: allow loading more than one slice at once
	float* slice = new float[width * height];
	currentZ = z;
	file.seekg(z * width * height * sizeof(float));
	file.read((char*)slice, width * height * sizeof(float));
	slices.push_back(slice);

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

VolIterator* VolIterator::Open(std::string filename, unsigned long long int width, unsigned long long int height, unsigned long long int depth) {

	// Check file path
	if (!fileExists(filename)) {
		printf("File %s cannot be found.\n", filename.c_str());
		return nullptr;
	}

	// Check file size
	unsigned long long int filesize = (unsigned long long int)fileSize(filename);
	unsigned long long int expected = width * height * depth * sizeof(float);
	if (filesize < expected) {
		printf("File %s was not found to be the advertised size (should be %llu bytes, found %llu bytes).\n", filename.c_str(), expected, filesize);
		return nullptr;
	}

	// Create iterator
	return new VolIterator(filename, width, height, depth);
}

bool VolIterator::exportSlicePng(unsigned long long int z, std::string filename, float minThreshold, float maxThreshold) {

	if (z >= depth) {
		printf("Invalid slice %llu on volume of size %llu x %llu x %llu.\n", z, width, height, depth);
		return false;
	}

	loadSlice(z);

	// Convert slice to 8-bit greyscale image
	unsigned char* pixels = new unsigned char[width * height];
	for (unsigned long long int y = 0; y < height; ++y) {
		for (unsigned long long int x = 0; x < width; ++x) {
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
		printf("Error writing to %llu x %llu png file %s.\n", width, height, filename.c_str());
		return false;
	}

	return true;
}