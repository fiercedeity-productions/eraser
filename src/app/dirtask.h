#pragma once
#include "task.h"

class DirTask : public Task {
  protected:
	std::vector<std::string> errorMessages_;

  public:
	virtual const bool                                             isIncluded(const std::string &path) const;
	virtual const size_t                                           getSize() const;
	virtual void                                                   execute();
	virtual const std::pair<std::string, std::vector<std::string>> getError() const;
	virtual void                                                   reset();

	// void updateContents();
	DirTask(const std::string &path, const standards::standard &mode);
};