#pragma once
#include "task.h"

class FileTask : public Task {
  protected:
  public:
	virtual const bool   isIncluded(const std::string &path) const;
	virtual const size_t getSize() const;
	virtual void          execute();
	FileTask(const std::string &path, const standards::standard &mode);
};
