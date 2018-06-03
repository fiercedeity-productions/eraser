#pragma once
#include <functional>
#include <string>
#include <vector>

#if defined(__GNUC__) && defined(__unix__)
#define EXPORT __attribute__((__visibility__("default")))
#elif defined(WIN32)
#define EXPORT __declspec(dllexport)
#endif

class EXPORT GoodBye {
  private:
  public:
	// validate settings. will throw errors
	static void validateSettings(const std::string &path, const size_t &chunkSize);

	// get size of a file at a path
	static size_t getSize(const std::string &path);

	// write a file
	static void overwriteBytes(const std::string &path, const size_t &chunkSize, const std::vector<unsigned char> &pattern,
	                           // arg1: bytes written, arg2: total bytes, arg3: pass number
	                           const std::function<void(size_t, size_t, size_t)> &progressCheck, size_t pass = 0);
	// write multiple times. does validation
	static void overwriteBytesMultiple(const std::string &path, const size_t &chunkSize,
	                                   const std::vector<std::vector<unsigned char>> &patterns,
	                                   // arg1: bytes written, arg2: total bytes, arg3: pass number
	                                   const std::function<void(size_t, size_t, size_t)> &progressCheck,
	                                   // arg1: total bytes in file, arg2: pass number
	                                   const std::function<void(size_t, size_t)> &callback);

	// create buffers
	static unsigned char *createBuffer(const std::vector<unsigned char> &pattern, const size_t &bufferSize,
	                                   const size_t &patternOverhang); // buffer based on pattern
	static unsigned char *createBuffer(const size_t &bufferSize);      // buffer based on random bits
};