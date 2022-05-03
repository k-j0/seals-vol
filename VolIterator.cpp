
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

	// Check params
	if (params.loadedNum < params.downscaleZ) {
		printf(RED "params.loadedNum (%zu) should be greater than params.downscaleZ (%zu)!\n" WHITE, params.loadedNum, params.downscaleZ);
		return nullptr;
	}

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

	// Sample downscaleX x downscaleY x downscaleZ pixels to return an average
	float total = 0.0f;
	size_t count = 0;
	for (size_t fullZ = z * params.downscaleZ; fullZ < (z + 1) * params.downscaleZ && fullZ < depth; ++fullZ) {

		// Ensure all slices corresponding to downscaled coord z are loaded in
		if (!loadSlice(fullZ)) return NAN;

		for (size_t fullY = y * params.downscaleY; fullY < (y + 1) * params.downscaleY && fullY < height; ++fullY) {
			for (size_t fullX = x * params.downscaleX; fullX < (x + 1) * params.downscaleX && fullX < width; ++fullX) {
				total += slices[fullZ - currentZ][fullY * width + fullX];
				++count;
			}
		}
	}

	// Average the sampled values
	return total / count;
}

bool VolIterator::exportSlicePng(size_t z, std::string filename, float minThreshold, float maxThreshold) {

	if (z >= getDownscaledDepth()) {
		printf(RED "Invalid slice %zu on volume of size %zu x %zu x %zu.\n" WHITE, z, getDownscaledWidth(), getDownscaledHeight(), getDownscaledDepth());
		return false;
	}

	if (!loadSlice(z)) {
		return false;
	}

	// Convert slice to 8-bit greyscale image
	size_t dWidth = getDownscaledWidth();
	size_t dHeight = getDownscaledHeight();
	unsigned char* pixels = new unsigned char[dWidth * dHeight];
	for (size_t y = 0; y < dHeight; ++y) {
		for (size_t x = 0; x < dWidth; ++x) {
			float val = getVoxel(x, y, z);
			val = (val - minThreshold) / (maxThreshold - minThreshold); // 0..1 remap
			pixels[y * dWidth + x] = val < 0.0f ? 0 : val > 1.0f ? 255 : int(val * 255); // clamp & write
		}
	}

	// Write out png file
	bool success = stbi_write_png(filename.c_str(), (int)dWidth, (int)dHeight, 1 /* greyscale */, pixels, 0);
	delete[] pixels;
	pixels = nullptr;
	if (!success) {
		printf(RED "Error writing to %zu x %zu png file %s.\n" WHITE, dWidth, dHeight, filename.c_str());
		return false;
	}

	return true;
}
