#pragma once
#include "task.h"

class FileTask : public Task {
  protected:
  public:
	virtual const bool                                             isIncluded(const std::string &path) const;
	virtual const size_t                                           getSize() const;
	virtual void                                                   execute();
	virtual const std::pair<std::string, std::vector<std::string>> getError() const;
	FileTask(const std::string &path, const standards::standard &mode);
};
