#pragma once
#include "frame.h"
#include "standards.h"
#include <deque>

class Task {
  protected:
	static Frame *            frameInstance;
	static std::deque<Task *> tasks;

	void   updateProgressBar(const double &proportion) const;
	void   updateStatus(const std::string &status) const;
	size_t updateSize() const;
	void   updateStatusBar(const std::string &str) const;

	Task(const std::string &path, const standards::standard &mode = standards::ZEROS);
	~Task();

  public:
	wxDataViewItem      queueRow_;
	mutable bool        locked_;    // is true when the task is being processed
	mutable bool        completed_; // true when the task has completed
	mutable bool        error_;     // true if error
	std::string         path_;
	standards::standard mode_;
	std::string         errorMessage_;

	static void                      setFrame(Frame *const frameInstance);
	static void                      test();
	static const wxDataViewItem &    add(const std::string &path, const standards::standard &mode = standards::ZEROS);
	static const std::deque<Task *> &getTasks(); // to execute isIncluded on all items in the queue
	static const bool                inQueue(const std::string &path);
	static void                      empty();
	static void                      removeByTaskPtr(Task *task);
	static const bool                isEmpty(); // returns true if all tasks are completed/erroneous or tasks are emtpy
	static const bool                callNext();

	virtual const size_t getSize() const                           = 0;
	virtual const bool   isIncluded(const std::string &path) const = 0; // to prevent duplicate values
	virtual void         execute()                                 = 0;

	void setMode(const standards::standard &mode);
	void reset();
};
