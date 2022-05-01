#pragma once

#include <string>
#ifdef __MINGW32__
	// On MinGW, cannot use std::filesystem...
	#include <cstdio>
#else
	#include <filesystem>
#endif

bool fileExists(std::string filename) {
#ifdef __MINGW32__
	if (FILE* file = fopen(filename.c_str(), "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}
#else
	return std::filesystem::exists(filename);
#endif
}

unsigned long long int fileSize(std::string filename) {
#ifdef __MINGW32__
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
#else
	return std::filesystem::file_size(filename);
#endif
}
