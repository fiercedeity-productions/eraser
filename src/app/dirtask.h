#pragma once
#include "task.h"

class DirTask : public Task {
  protected:
  public:
	virtual const bool   isIncluded(const std::string &path) const;
	virtual const size_t getSize() const;
	virtual void         execute();

	// void updateContents();
	DirTask(const std::string &path, const standards::standard &mode);
};