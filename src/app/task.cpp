#include "task.h"
#include "dirtask.h"
#include "filetask.h"
#include "lib.h"
#include "storeddata.h"
#include "updateprogressdata.h"
#ifdef _MSC_VER
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include <thread>

Frame *             Task::frameInstance = nullptr;
std::vector<Task *> Task::tasks;

void Task::setFrame(Frame *const frameInstance) {
	Task::frameInstance = frameInstance;
}

void Task::test() {
}

const wxDataViewItem &Task::add(const std::string &path, const standards::standard &mode) {
	Task *task;

	if (std::experimental::filesystem::is_regular_file(path) || std::experimental::filesystem::is_block_file(path) ||
	    std::experimental::filesystem::is_character_file(path))
		task = new FileTask(path, mode);
	else if (std::experimental::filesystem::is_directory(path)) {
		task = new DirTask(path, mode);
	}

	tasks.push_back(task);
	frameInstance->onQueueChanged();
	return task->queueRow_;
}

const std::vector<Task *> &Task::getTasks() {
	return tasks;
}

Task::Task(const std::string &path, const standards::standard &mode)
    : locked_(false)
    , mode_(mode)
    , completed_(false)
    , error_(false) {

	try {
		path_ = std::experimental::filesystem::canonical(path).string();
	} catch (std::experimental::filesystem::filesystem_error &e) {
		errorMessage_ = e.what();
	}

	StoredData *storedData = new StoredData{this};

	wxVector<wxVariant> data;
	data.push_back(path_);
	data.push_back(standards::NAMES_SHORT[mode_]);
	data.push_back("Calculating...");
	data.push_back("Ready");
	data.push_back(0);
	frameInstance->queueCtrl_->AppendItem(data, reinterpret_cast<wxUIntPtr>(storedData));
	queueRow_ = frameInstance->queueCtrl_->RowToItem(frameInstance->queueCtrl_->GetItemCount() - 1);
}

const bool Task::inQueue(const std::string &path) {
	for (Task *const t : tasks)
		if (t->isIncluded(path))
			return true;

	return false;
}

const bool Task::isEmpty() {
	if (tasks.empty())
		return true;

	for (Task *t : tasks)
		if (!t->completed_ && !t->error_ && !t->locked_)
			return false;
	return true;
}

void Task::empty() {
	for (Task *const t : tasks) {
		delete t;
	}
}

Task::~Task() {
	frameInstance->queueCtrl_->DeleteItem(frameInstance->queueCtrl_->ItemToRow(queueRow_));
}

void Task::removeByTaskPtr(Task *task) {
	if (!task->locked_) {
		for (auto i = tasks.begin(); i != tasks.end(); ++i) {
			if (*i == task) {
				delete *i;
				tasks.erase(i);
				frameInstance->onQueueChanged();
				return;
			}
		}
	}
}

void Task::setMode(const standards::standard &mode) {
	if (!locked_ && !error_ && !completed_) {
		mode_ = mode;
		frameInstance->queueCtrl_->SetValue(standards::NAMES_SHORT[mode_], frameInstance->queueCtrl_->ItemToRow(queueRow_), 1);
	}
}

const bool Task::callNext() {
	if (!tasks.empty()) {
		// traverse down the queue until finding a task that has not been completed
		auto it = tasks.begin();
		while (it != tasks.end() && ((*it)->locked_ || (*it)->completed_ || (*it)->error_))
			++it;
		if (it != tasks.end()) {
			(*it)->execute();
			return true;
		}
	}
	return false;
}

void Task::updateProgressBar(const double &proportion) const {
	wxCommandEvent *evt = new wxCommandEvent(UPDATE_VALUE);
	evt->SetClientData(
	    new UpdateProgressData{static_cast<int>(100.0 * proportion), frameInstance->queueCtrl_->ItemToRow(queueRow_), 4});
	wxQueueEvent(frameInstance, evt);
}

void Task::updateStatus(const std::string &status) const {
	wxCommandEvent *evt = new wxCommandEvent(UPDATE_VALUE);
	evt->SetClientData(new UpdateProgressData{status, frameInstance->queueCtrl_->ItemToRow(queueRow_), 3});
	wxQueueEvent(frameInstance, evt);
}

size_t Task::updateSize() const {
	size_t size = getSize();

	wxCommandEvent *evt = new wxCommandEvent(UPDATE_VALUE);
	evt->SetClientData(new UpdateProgressData{std::string(size == static_cast<size_t>(-1) ? "" : std::to_string(size)),
	                                          frameInstance->queueCtrl_->ItemToRow(queueRow_), 2});
	wxQueueEvent(frameInstance, evt);

	return size;
}

void Task::reset() {
	error_        = false;
	locked_       = false;
	completed_    = false;
	errorMessage_ = "";
	updateProgressBar(0);
	updateStatus("Ready");
	updateSize(); // must be last as updateSize will modify "error_" if file does not exist or user does not have sufficient
	              // permissions
}

void Task::updateStatusBar(const std::string &str) const {
	wxCommandEvent *evt = new wxCommandEvent(SET_STATUS);
	evt->SetString(str);
	wxQueueEvent(frameInstance, evt);
}