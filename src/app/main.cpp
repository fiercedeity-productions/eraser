#pragma once
#include "lib.h"
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

int main(int argc, const char **argv) {
	std::mutex mut;

	Eraser::overwriteBytes("hello", 4096 * 1024, "hello", [&](size_t progress, size_t fileSize) -> void {
		std::lock_guard<std::mutex> lock(mut);
		std::cout << "bytes written:\t" << progress << "\ttotal bytes:\t" << fileSize << "\tprogress:\t"
		          << static_cast<double>(100) * static_cast<double>(progress) / static_cast<double>(fileSize) << "%"
		          << std::endl;
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}