#pragma once

#include <string>
#ifdef __MINGW32__
	// On MinGW, cannot use std::filesystem...
	#include <sys/stat.h>
	#include <cstdio>
	#include <direct.h>
#else
	#include <filesystem>
#endif

namespace fs {

	/// Returns whether the given file exists
	inline bool fileExists(std::string filename) {
	#ifdef __MINGW32__
		struct stat buf;
		return stat(filename.c_str(), &buf) == 0;
	#else
		return std::filesystem::exists(filename);
	#endif
	}

	/// Returns the file size in bytes of the given file
	inline size_t fileSize(std::string filename) {
	#ifdef __MINGW32__
		std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
		return in.tellg();
	#else
		return std::filesystem::file_size(filename);
	#endif
	}

	/// Creates the directory pointed to by the path and returns true for success
	inline bool createDirectory(std::string path) {
		if (fileExists(path)) return true;
	#ifdef __MINGW32__
		return _mkdir(path.c_str()) == 0;
	#else
		return std::filesystem::create_directory(path);
	#endif
	}

}
