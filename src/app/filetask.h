#pragma once
#include "task.h"

class FileTask : public Task {
  public:
	FileTask(const std::string &path);
	virtual bool isIncluded(const std::string &path) const;
};
