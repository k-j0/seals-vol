
#include "VolIterator.h"

#include <fstream>
#include <cassert>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "filesystem.h"
#include "colours.h"
#include "ObjModel.h"


VolIterator::VolIterator(std::string filename, size_t width, size_t height, size_t depth, const VolIteratorParams& params) : width(width), height(height), depth(depth), params(params) {
	
	if (fs::isDirectory(filename)) {
		std::vector<std::string> filenames;
		fs::listDirectoryFiles(filename, filenames);
		for (int i = 0, sz = filenames.size(); i < sz; ++i) {
			if (i == 0) {
				commonFileSize = fs::fileSize(filenames[i]);
			} else if (i < sz - 1 && fs::fileSize(filenames[i]) != commonFileSize) {
				printf(RED "Cannot load volume parts that do not share the exact same file size (part %d is %lu, expecting %lu)\n" WHITE, i, fs::fileSize(filenames[i]), commonFileSize);
				exit(1);
			}
			files.emplace_back(filenames[i], std::ios::binary);
		}
	} else {
		commonFileSize = fs::fileSize(filename);
		files.emplace_back(filename, std::ios::binary);
	}
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
		std::size_t remaining = width * height * sizeof(float); // number of bytes to read
		std::streamoff pos = (std::streamoff)(currentZ + slices.size()) * width * height * sizeof(float); // global offset
		std::size_t writeOffset = 0; // offset into the slice being written out
		while (remaining > 0) {
			std::size_t fileIdx = pos % commonFileSize; // grab the file that contains the beginning of the range of bytes to read
			std::streamoff localPos = pos - fileIdx * commonFileSize; // position of the range start inside the file
			files[fileIdx].seekg(localPos);
			if (files[fileIdx].tellg() == -1) {
				printf(RED "Error reading volume file; it might be too large to read currently, make sure to use a 64-bit architecture if possible.\n" WHITE);
				return false;
			}
			std::size_t readSize = localPos + remaining >= commonFileSize ? commonFileSize - localPos : remaining; // amount of bytes to read from this file specifically
			files[fileIdx].read(&((char*)slice)[writeOffset], (std::streamsize)readSize);
			remaining -= readSize;
			writeOffset += readSize;
			pos += readSize;
		}
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
	for (auto& file : files) {
		file.close();
	}
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
	if (fs::isDirectory(filename)) {
		printf(YELLOW "Warning: the given filename is a directory, assuming volume part files without size check.\n" WHITE);
	} else {
		size_t filesize = fs::fileSize(filename);
		size_t expected = width * height * depth * sizeof(float);
		if (filesize < expected) {
			printf(RED "File %s was not found to be the advertised size (should be %zu bytes, found %zu bytes).\n" WHITE, filename.c_str(), expected, filesize);
			return nullptr;
		}
	}

	// Create iterator
	return new VolIterator(filename, width, height, depth, params);
}

float VolIterator::getVoxel(size_t x, size_t y, size_t z) {

	// Sample downscaleX x downscaleY x downscaleZ pixels to return an average
	float total = 0.0f;
	size_t count = 0;
	for (size_t fullZ = z * params.downscaleZ; fullZ < (z + 1) * params.downscaleZ && fullZ < depth; ++fullZ) {
		
		if (fullZ < currentZ || fullZ >= currentZ + slices.size()) {
			printf(RED "Z is out of range when fetching voxels: %zu: full = %zu, current = %zu, slices.length = %zu, depth = %zu...\n" WHITE, z, fullZ, currentZ, slices.size(), depth);
			exit(1);
		}
		assert(fullZ >= currentZ && fullZ < currentZ + slices.size());

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
		printf(RED "Cannot load slice %zu out of %zu, aborting.\n" WHITE, z, getDownscaledDepth());
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

bool VolIterator::exportObj(std::string filename, float threshold, float scale) {

	ObjModel model;

	size_t dDepth = getDownscaledDepth();
	size_t dWidth = getDownscaledWidth();
	size_t dHeight = getDownscaledHeight();

	if (params.loadedNum < 3 * params.downscaleZ) {
		printf(RED "params.loadedNum needs to be at least %zu for simple obj export to function!\n" WHITE, 3 * params.downscaleZ);
		return false;
	}

	// load first slice(s)
	for (size_t fullZ = 0; fullZ < params.downscaleZ && fullZ < depth; ++fullZ) {
		if (!loadSlice(fullZ)) {
			printf(RED "Cannot load slice %zu out of %zu, aborting.\n" WHITE, fullZ, depth);
			return false;
		}
	}

	// Iterate over voxels, slice by slice
	for (size_t z = 0; z < dDepth; ++z) {

		// Ensure the next slice(s) is/are loaded in - the current and previous slices should already be available.
		if (z < dDepth - 1) {
			for (size_t fullZ = (z + 1) * params.downscaleZ; fullZ < (z + 2) * params.downscaleZ && fullZ < depth; ++fullZ) {
				if (!loadSlice(fullZ)) {
					printf(RED "Cannot load slice %zu (at step %zu) out of %zu (full = %zu, depth = %zu), aborting.\n" WHITE, z + 1, z, getDownscaledDepth(), fullZ, depth);
					return false;
				}
			}
		}
		
		for (size_t y = 0; y < dHeight; ++y) {
			for (size_t x = 0; x < dWidth; ++x) {

				bool vox = getVoxel(x, y, z) >= threshold;
				if (!vox) continue;

				// 6 sides
				if (x == dWidth - 1 || getVoxel(x + 1, y, z) < threshold) {
					model.addAASquare((x + 0.5f) * scale, y * scale, z * scale, ObjModel::Direction::POS_X, 0.5f * scale);
				}
				if (x == 0 || getVoxel(x - 1, y, z) < threshold) {
					model.addAASquare((x - 0.5f) * scale, y * scale, z * scale, ObjModel::Direction::NEG_X, 0.5f * scale);
				}
				if (y == dHeight - 1 || getVoxel(x, y + 1, z) < threshold) {
					model.addAASquare(x * scale, (y + 0.5f) * scale, z * scale, ObjModel::Direction::POS_Y, 0.5f * scale);
				}
				if (y == 0 || getVoxel(x, y - 1, z) < threshold) {
					model.addAASquare(x * scale, (y - 0.5f) * scale, z * scale, ObjModel::Direction::NEG_Y, 0.5f * scale);
				}
				if (z == dDepth - 1 || getVoxel(x, y, z + 1) < threshold) {
					model.addAASquare(x * scale, y * scale, (z + 0.5f) * scale, ObjModel::Direction::POS_Z, 0.5f * scale);
				}
				if (z == 0 || getVoxel(x, y, z - 1) < threshold) {
					model.addAASquare(x * scale, y * scale, (z - 0.5f) * scale, ObjModel::Direction::NEG_Z, 0.5f * scale);
				}

			}
		}

		printf("%zu of %zu\n", z + 1, dDepth);
	}

	// Write out to wavefront file
	if (!model.writeToFile(filename)) {
		printf(RED "Cannot write obj model to file %s, aborting.\n" WHITE, filename.c_str());
		return false;
	}

	return true;
}
