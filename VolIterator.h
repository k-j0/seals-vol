#pragma once

#include <string>
#include <fstream>
#include <vector>

class VolIterator {

	/// Width, height, depth of the file
	unsigned long long int width, height, depth;

	/// File we're reading from
	std::ifstream file;

	/// Array of slices of the file currently loaded in; each slice is width x height floats
	std::vector<float*> slices;

	/// Z coordinate of the first slice currently loaded into the slices vector; if more than one slice is loaded, they're assumed to be neighbours
	unsigned long long int currentZ = 0;

protected:

	/// Creates a volume iterator to read a .vol file, assumed large
	VolIterator(std::string filename, unsigned long long int width, unsigned long long int height, unsigned long long int depth);

	/// Loads the given slice from the original file (padded with neighbours as needed)
	void loadSlice(unsigned long long int z);

public:
	virtual ~VolIterator();

	/// Attempts to find the given file, and checks the file size; if valid, returns a new VolIterator; if not, returns nullptr
	static VolIterator* Open(std::string filename, unsigned long long int width, unsigned long long int height, unsigned long long int depth);

	/// Getters
	inline unsigned long long int getWidth() const { return width; }
	inline unsigned long long int getHeight() const { return height; }
	inline unsigned long long int getDepth() const { return depth; }

	/// Clears the internal buffer of slices
	void clearSlices();

	/// Exports a png image of a slice
	bool exportSlicePng(unsigned long long int z, std::string filename, float minThreshold, float maxThreshold);
};
