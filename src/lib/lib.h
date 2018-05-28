#pragma once
#include <functional>
#include <string>

#if defined(__GNUC__) && defined(__unix__)
#define EXPORT __attribute__((__visibility__("default")))
#elif defined(WIN32)
#define EXPORT __declspec(dllexport)
#endif

class EXPORT Eraser {
  private:
  public:
	// get size of a file at a path
	static size_t getSize(const std::string &path);
	// write a file
	static void overwriteBytes(const std::string &path, const size_t &chunkSize, const std::string &pattern,
	                           std::function<void(size_t, size_t)> callback);
	// create buffers
	static char *createBuffer(const std::string &pattern, const size_t &bufferSize, const size_t &patternOverhang);
};