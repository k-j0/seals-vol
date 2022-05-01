
#include <iostream>

#include "VolIterator.h"


int main(int argc, char** argv) {
	printf("\n");

	// Read command-line arguments
	if (argc < 5) {
		printf("Usage: ./seals-vol <filename> <width> <height> <depth> <threshold>\n");
		return 0;
	}
	std::string filename = argv[1];
	unsigned long long int width = std::stoi(argv[2]), height = std::stoi(argv[3]), depth = std::stoi(argv[4]);
	float threshold = (float)std::atof(argv[5]);
	printf("Opening volume %s at size %llu x %llu x %llu (threshold: %f).\n\n", filename.c_str(), width, height, depth, threshold);

	// Create volume iterator object
	VolIterator* vol = VolIterator::Open(filename, width, height, depth);
	if (!vol) return 1;

	// Export some of the slices in the volume
	for (unsigned long long int z = 0, depth = vol->getDepth(); z < depth; z += 5) {
		vol->exportSlicePng(z, "out/" + std::to_string(z) + ".png", threshold, threshold);
		printf("%llu / %llu\n", z, depth);
	}

	// Cleanup
	delete vol;

	return 0;
}
