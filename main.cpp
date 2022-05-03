
#include <iostream>
#include <memory>
#include "VolIterator.h"
#include "filesystem.h"
#include "colours.h"


int main(int argc, char** argv) {
	printf("\n");

	// Parameters
	std::string filename;
	size_t width, height, depth;
	VolIteratorParams params;
	params.downscaleX = 2;
	params.downscaleY = 2;
	params.downscaleZ = 2;
	params.loadedNum = params.downscaleZ * 2;

	// Read command-line arguments
	if (argc < 5) {
		printf(YELLOW "Usage: ./seals-vol <filename> <width> <height> <depth> <threshold>\n" WHITE);
		return 0;
	}
	filename = argv[1];
	width = std::stoi(argv[2]);
	height = std::stoi(argv[3]);
	depth = std::stoi(argv[4]);
	float threshold = (float)std::atof(argv[5]);
	printf(BLUE "Opening volume %s at size %zu x %zu x %zu (threshold: %f).\n\n" WHITE, filename.c_str(), width, height, depth, threshold);

	// Create volume iterator object
	std::unique_ptr<VolIterator> vol = std::unique_ptr<VolIterator>(VolIterator::Open(filename, width, height, depth, params));
	if (!vol) return 1;

	// Get file name without extension
	long long int lastSlash = filename.find_last_of('/');
	long long int lastBlackslash = filename.find_last_of('\\');
	if (lastSlash > (signed)filename.length()) lastSlash = -1;
	if (lastBlackslash > (signed)filename.length()) lastBlackslash = -1;
	if (lastBlackslash > lastSlash) lastSlash = lastBlackslash;
	long long int lastPeriod = filename.find_last_of('.');
	std::string name = filename.substr(lastSlash + 1, lastPeriod - lastSlash - 1);
	if (!fs::createDirectory("out") || !fs::createDirectory("out/" + name)) {
		printf(RED "Cannot create output directory out/%s, aborting operation.\n" WHITE, name.c_str());
		return 1;
	}

	// Export some of the slices in the volume
	for (size_t z = 0, depth = vol->getDownscaledDepth(); z < depth; z += 10) {
		if (!vol->exportSlicePng(z, "out/" + name + "/" + std::to_string(z) + ".png", threshold, threshold)) {
			return 1;
		}
		printf("%zu / %zu\n", z, depth);
	}

	return 0;
}
