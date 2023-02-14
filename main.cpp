
#include <iostream>
#include <memory>
#include "VolIterator.h"
#include "Arguments.h"
#include "filesystem.h"
#include "colours.h"


int main(int argc, char** argv) {
	printf("\n");

	// Read command-line arguments
	std::string filename;
	size_t width, height, depth, skipZ = 0;
    float threshold;
    bool generate3DModel;
	VolIteratorParams params;
    {
        Arguments args(argc, argv);
        if (args.read<bool>("harp-adult", false)) {
            filename = "../seals-scans/phoca_groenlandica_7495 [2022-04-20 11.02.48]/phoca_groenlandica_7495b/phoca_groenlandica_7495.vol-parts";
            width = 1920;
            height = 1855;
            depth = 1535;
        } else {
            filename = args.read<std::string>("file", "../seals-scans/phoca_groenlandica_juvenil [2022-04-05 13.35.00]/phoca_groenlandica_juvenil/phoca_groenlandica_juvenil.vol-parts");
            width = args.read<size_t>("width", 1893);
            height = args.read<size_t>("height", 1919);
            depth = args.read<size_t>("depth", 1535);
        }
        threshold = args.read<float>("threshold", 7.5f);
        params.downscaleX = params.downscaleY = args.read<size_t>("downscaleXY", 1);
        params.downscaleZ = args.read<size_t>("downscaleZ", 1);
        generate3DModel = args.read<bool>("3d", false);
        if (!generate3DModel) {
            skipZ = args.read<size_t>("skipZ", 10);
        }
    }
	params.loadedNum = params.downscaleZ * 3;
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
    
    if (generate3DModel) {
        // Export entire volume as polygon mesh (simple cubes)
        printf(BLUE "Generating 3D obj, target file: out/%s.obj\n" WHITE, name.c_str());
        if (!vol->exportObj("out/" + name + ".obj", threshold, 0.01f)) {
            return 1;
        }
    } else {
        // Export cross-sections from the volume
        printf(BLUE "Generating cross sections, target directory: out/%s/\n" WHITE, name.c_str());
        for (size_t z = 0, depth = vol->getDownscaledDepth(); z < depth; z += skipZ + 1) {
            printf("%zu / %zu\n", z+1, depth);
            if (!vol->exportSlicePng(z, "out/" + name + "/" + std::to_string(z) + ".png", threshold, threshold)) {
                return 1;
            }
        }
    }
    
    printf(BLUE "Done.\n" WHITE);

	return 0;
}
