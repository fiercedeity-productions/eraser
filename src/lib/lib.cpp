#pragma once
#include "lib.h"

#include <filesystem>
#include <fstream>
#include <thread>

// get size of a file at a path
size_t Eraser::getSize(const std::string &path) {
	// open the file
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	// return the position of the cursor, and therefore the size of the file in bytes
	return file.tellg();
}

// validate settings
void Eraser::validateSettings(const std::string &path, const size_t &chunkSize) {
	std::string errorMessage;

	std::ofstream file(path, std::ios::binary | std::ios::in);
	if (chunkSize < 4096)
		errorMessage = "Chunk size must be 4096 bytes or larger!";
	else if (!std::experimental::filesystem::is_regular_file(path))
		errorMessage = "File does not exist!";
	else if (!file.is_open())
		errorMessage = "Error writing to file!";
	else if (getSize(path) == 0)
		errorMessage = "File must not be empty!";
	else {
		return;
	}

	throw std::runtime_error(errorMessage);
}

// overwrite files
void Eraser::overwriteBytes(const std::string &path, const size_t &chunkSize, const std::vector<unsigned char> &pattern,
                            // arg1: bytes written, arg2: total bytes, arg3: pass number
                            const std::function<void(size_t, size_t, size_t)> &progressCheck, size_t pass) {
	// empty pattern means random bits

	// open the file, without modifying it
	std::ofstream file(path, std::ios::binary | std::ios::in);
	// move to the beginning of the file to overwrite the data
	file.seekp(0);

	const size_t fileSize = getSize(path); // get the file size
	const size_t overhangSize =
	    fileSize % chunkSize; // calculate the amount left when writing to the file in chunks of chunkSize

	// overwrite the part divisible by chunkSize
	size_t patternOverhang = 0;
	size_t progress        = 0;
	for (size_t i = 0; i < (fileSize - overhangSize) / chunkSize; ++i) {
		unsigned char *buffer = pattern.empty() ? createBuffer(chunkSize) : createBuffer(pattern, chunkSize, patternOverhang);
		if (!pattern.empty())
			patternOverhang += chunkSize % pattern.size();
		file.write(reinterpret_cast<char *>(buffer), chunkSize);

		delete[] buffer;
		std::thread(progressCheck, progress += chunkSize, fileSize, pass).detach();
	}

	// finish it off
	unsigned char *buffer = pattern.empty() ? createBuffer(overhangSize) : createBuffer(pattern, overhangSize, patternOverhang);
	file.write(reinterpret_cast<char *>(buffer), overhangSize);
	std::thread(progressCheck, progress += overhangSize, fileSize, pass).detach();

	delete[] buffer;
}

// multiple times
void Eraser::overwriteBytesMultiple(const std::string &path, const size_t &chunkSize,
                                    const std::vector<std::vector<unsigned char>> &patterns,
                                    // arg1: bytes written, arg2: total bytes, arg3: pass number
                                    const std::function<void(size_t, size_t, size_t)> &progressCheck,
                                    // arg1: arg2: total bytes in file, arg2: pass number
                                    const std::function<void(size_t, size_t)> &callback) {
	// do some validation
	validateSettings(path, chunkSize);
	// make sure the patterns vector is not empty
	if (patterns.empty())
		throw std::runtime_error("Patterns must not be empty!");

	// loop through patterns and apply them
	for (auto i = patterns.begin(); i < patterns.end(); ++i) {
		// overwrite bytes based on current pattern (if pattern is empty, it will write random bytes)
		overwriteBytes(path, chunkSize, *i, progressCheck, i - patterns.begin());
	}

	// call the callback
	std::thread(callback, getSize(path), patterns.size() - 1).join();
}

// create buffers to overwrite the files
unsigned char *Eraser::createBuffer(const std::vector<unsigned char> &pattern, const size_t &bufferSize,
                                    const size_t &patternOverhang) {
	// create an array on the heap with the size of chunkSize
	unsigned char *buffer = new unsigned char[bufferSize];
	// fill the buffer up
	unsigned long long p = patternOverhang % pattern.size();
	for (size_t c = 0; c < bufferSize; ++c) {
		// set the byte to the next byte in the pattern
		buffer[c] = pattern[p++];
		p %= pattern.size();
	}

	return buffer;
}

unsigned char *Eraser::createBuffer(const size_t &bufferSize) {
	// seed rand() to time
	srand(static_cast<unsigned int>(time(nullptr)));
	// create an array on the heap with the size of chunkSize
	unsigned char *buffer = new unsigned char[bufferSize];
	// fill the buffer up with random bits
	for (size_t c = 0; c < bufferSize; ++c) {
		// create a random byte
		unsigned char byte = 0;
		for (int bit = 0; bit < 7; ++bit) {
			byte <<= 1;         // shift bits left
			byte |= rand() & 1; // toggle last bit randomly
		}
		// set the byte
		buffer[c] = byte;
	}

	return buffer;
}
