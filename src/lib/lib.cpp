#pragma once
#include "lib.h"

#include <fstream>
#include <thread>

// get size of a file at a path
size_t Eraser::getSize(const std::string &path) {
	// open the file
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	// return the position of the cursor, and therefore the size of the file in bytes
	return file.tellg();
}

void Eraser::overwriteBytes(const std::string &path, const size_t &chunkSize, const std::string &pattern,
                            std::function<void(size_t, size_t)> callback) {
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
		char *buffer = createBuffer(pattern, chunkSize, patternOverhang);
		patternOverhang += chunkSize % pattern.length();
		file.write(buffer, chunkSize);

		delete[] buffer;
		std::thread(callback, progress += chunkSize, fileSize).detach();
	}

	// finish it off

	char *buffer = createBuffer(pattern, chunkSize, patternOverhang);
	file.write(buffer, overhangSize);
	std::thread(callback, progress += overhangSize, fileSize).detach();

	delete[] buffer;
}

char *Eraser::createBuffer(const std::string &pattern, const size_t &bufferSize, const size_t &patternOverhang) {
	// pattern cannot be empty!
	if (pattern.empty())
		throw "Pattern cannot be empty!";
	// create an array on the heap with the size of chunkSize
	char *buffer = new char[bufferSize];
	// fill the buffer up
	unsigned long long p = patternOverhang % pattern.length();
	for (size_t c = 0; c < bufferSize; ++c) {
		// create a byte and fill it up
		buffer[c] = pattern[p++];
		p %= pattern.length();
	}

	return buffer;
}