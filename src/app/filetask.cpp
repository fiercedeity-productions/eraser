#pragma once
#include "filetask.h"

FileTask::FileTask(const std::string &path)
    : Task(path) {
	wxPuts("is file path");
	wxPuts(std::to_string(Task::getTasks().size()));
}

bool FileTask::isIncluded(const std::string &path) const {
#if defined(unix) || defined(__unix__) || defined(__unix)
	// case sensitive comparison
	return path == path_;
#else
	// case insensitive comparison; requires copies

	std::string testPath   = path;
	std::string storedPath = path_;

	std::transform(testPath.begin(), testPath.end(), testPath.begin(), ::tolower);
	std::transform(storedPath.begin(), storedPath.end(), storedPath.begin(), ::tolower);

	return testPath == storedPath;
#endif
}