#pragma once

#include "task.h"
#include "filetask.h"
#include "storeddata.h"
#include <filesystem>
#include <thread>

Frame *            Task::frameInstance = 0;
std::deque<Task *> Task::tasks         = {};

void Task::setFrame(Frame *const frameInstance) {
	Task::frameInstance = frameInstance;
}

void Task::test() {
	wxCommandEvent evt(UPDATE_PROGRESS);
	evt.SetString("hi");
	wxPostEvent(frameInstance, evt);
}

void Task::add(const std::string &path) {
	if (std::experimental::filesystem::is_regular_file(path) || std::experimental::filesystem::is_block_file(path) ||
	    std::experimental::filesystem::is_character_file(path))
		tasks.push_back(new FileTask(path));
}

const std::deque<Task *> &Task::getTasks() {
	return tasks;
}

Task::Task(const std::string &path)
    : path_(path)
    , locked_(false) {

	StoredData *storedData = new StoredData{this};

	frameInstance->resizeColumns();
	wxPuts("new task: " + path);
	wxVector<wxVariant> data;
	data.push_back(path);
	data.push_back("Pending");
	data.push_back(0);
	frameInstance->queueCtrl_->AppendItem(data, reinterpret_cast<wxUIntPtr>(storedData));
	queueRow_ = frameInstance->queueCtrl_->RowToItem(frameInstance->queueCtrl_->GetItemCount() - 1);
}

bool Task::inQueue(const std::string &path) {
	for (Task *&const t : tasks)
		if (t->isIncluded(path))
			return true;

	return false;
}

bool Task::isIncluded(const std::string &path) const {
	return path == path_;
}

void Task::empty() {
	for (Task *&const t : tasks) {
		delete t;
	}
}

const std::string &Task::getPath() {
	return path_;
};

Task::~Task() {
}

const bool &Task::isLocked() {
	return locked_;
}

void Task::removeByTaskPtr(Task *task) {
	for (auto i = tasks.begin(); i != tasks.end(); ++i) {
		if (*i == task) {
			delete *i;
			tasks.erase(i);
			return;
		}
	}
}