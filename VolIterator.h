#pragma once

#include <string>
#include <fstream>
#include <vector>


struct VolIteratorParams {

	/// Maximum of slices to extract simultaneously before discarding old slices
	size_t loadedNum = 5;

	/// Downscale factors - if downscaleX == 2, each sampled voxel at coord x will be the result of the average of (x * 2) and (x * 2 + 1)
	size_t downscaleX = 1;
	size_t downscaleY = 1;
	size_t downscaleZ = 1;

};


class VolIterator {

	/// Width, height, depth of the file
	size_t width, height, depth;

	/// Parameters to drive the file reading
	VolIteratorParams params;

	/// File we're reading from
	std::ifstream file;

	/// Array of slices of the file currently loaded in; each slice is width x height floats
	/// Maximum length at any one time is params.loadedNum
	std::vector<float*> slices;

	/// Z coordinate of the first slice currently loaded into the slices vector; if more than one slice is loaded, they're assumed to be neighbours
	size_t currentZ = 0;

protected:

	/// Creates a volume iterator to read a .vol file, assumed large
	VolIterator(std::string filename, size_t width, size_t height, size_t depth, const VolIteratorParams& params);

	/// Loads the given slice from the original file (padded with neighbours as needed)
	bool loadSlice(size_t z);

public:
	virtual ~VolIterator();

	/// Attempts to find the given file, and checks the file size; if valid, returns a new VolIterator; if not, returns nullptr
	static VolIterator* Open(std::string filename, size_t width, size_t height, size_t depth, const VolIteratorParams& params);

	/// Getters
	inline size_t getDownscaledWidth()	const { return width / params.downscaleX  + (width % params.downscaleX  ? 1 : 0); }
	inline size_t getDownscaledHeight() const { return height / params.downscaleY + (height % params.downscaleY ? 1 : 0); }
	inline size_t getDownscaledDepth()	const { return depth / params.downscaleZ  + (depth % params.downscaleZ  ? 1 : 0); }

	/// Clears the internal buffer of slices
	void clearSlices();

	/// Obtains the float value for a single voxel
	float getVoxel(size_t x, size_t y, size_t z);

	/// Exports a png image of a slice
	bool exportSlicePng(size_t z, std::string filename, float minThreshold, float maxThreshold);

	/// Converts the entire volume to cubified polygon mesh
	bool exportObj(std::string filename, float threshold, float scale);
};
