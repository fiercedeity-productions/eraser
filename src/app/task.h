#pragma once
#include "frame.h"
#include <deque>

class Task {
  protected:
	static Frame *            frameInstance;
	static std::deque<Task *> tasks;

	wxDataViewItem queueRow_;
	bool           locked_; // is true when the task is being processed
	std::string    path_;

  public:
	static void                      setFrame(Frame *const frameInstance);
	static void                      test();
	static void                      add(const std::string &path);
	static const std::deque<Task *> &getTasks(); // to execute isIncluded on all items in the queue
	static bool                      inQueue(const std::string &path);
	static void                      empty();
	static void                      removeByTaskPtr(Task *task);

	const std::string &getPath();
	const bool &       isLocked();

	Task(const std::string &path);
	~Task();
	virtual bool isIncluded(const std::string &path) const; // to prevent duplicate values
};
