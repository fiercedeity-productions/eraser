#pragma once

#include "dirtask.h"
#include "lib.h"
#include "updateprogressdata.h"
#ifdef _MSC_VER
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include <iomanip>
#include <numeric>
#include <sstream>
#include <thread>

#define SKIP 1024

DirTask::DirTask(const std::string &path, const standards::standard &mode)
    : Task(path, mode) {

	// TODO: do not allow addition of folder if any files within it are already added.

	std::thread([=]() {
		// put all files and folders inside "path" into the paths_ vector

		// replace "Calculating" displayed by the datactrl to the actual value
		// updateContents(); // calls updateSize()
		updateSize(); // will set size to blank as it is a folder
	})
	    .detach();
}

const bool DirTask::isIncluded(const std::string &path) const {
	// get absolute path
	std::string pathAbs = std::experimental::filesystem::canonical(path).string();
	return pathAbs.length() >= path_.length() &&
	       std::experimental::filesystem::equivalent(pathAbs.substr(0, path_.length()), path_) && !completed_;
}

// const size_t DirTask::getSize() const {
// 	locked_ = true;

// 	size_t culminativeSize = 0;
// 	size_t count           = 0;
// 	for (const std::string &path : filePaths_) {
// 		culminativeSize += GoodBye::getSize(path);

// 		if (!(++count % SKIP)) { // update gui every SKIP items
// 			std::thread([=]() {
// 				wxCommandEvent *evt = new wxCommandEvent(UPDATE_VALUE);
// 				evt->SetClientData(new UpdateProgressData{"Calculating: " + std::to_string(culminativeSize),
// 				                                          frameInstance->queueCtrl_->ItemToRow(queueRow_), 2});
// 				wxQueueEvent(frameInstance, evt);
// 			})
// 			    .detach();
// 			std::thread(&DirTask::updateStatusBar, this, path).detach();
// 		}
// 	}

// 	locked_ = false;
// 	return culminativeSize;
// }

const size_t DirTask::getSize() const {
	return -1;
}

void DirTask::execute() {
	// // TODO: implement actual execution
	// // simulate completion
	// std::thread(&DirTask::updateProgressBar, this, 1).detach();
	// locked_    = false;
	// completed_ = true;
	// updateStatus("Completed");

	// // call the next task
	// wxQueueEvent(frameInstance, new wxCommandEvent(CALL_NEXT_TASK));

	std::thread([&]() {
		if (!std::experimental::filesystem::is_directory(path_)) {
			updateStatus("Error");
			locked_    = false;
			completed_ = false;
			error_     = true;

			// call the next task
			errorMessage_ = "Directory does not exist";
			errorMessages_.push_back(errorMessage_);
			wxQueueEvent(frameInstance, new wxCommandEvent(CALL_NEXT_TASK));
			return;
		}

		wxQueueEvent(frameInstance, new wxCommandEvent(SHOW_STATUS));
		locked_ = true;
		for (auto &file : std::experimental::filesystem::recursive_directory_iterator(path_)) {
			if (std::experimental::filesystem::is_directory(file.path()))
				continue; // do not process folders

			if (frameInstance->paused_) {
				updateStatus("Stopped");
				updateProgressBar(0);
				locked_    = false;
				completed_ = false;
				error_     = true;

				// call the next task
				errorMessage_ = "Stopped by User";
				errorMessages_.push_back(errorMessage_);
				wxQueueEvent(frameInstance, new wxCommandEvent(CALL_NEXT_TASK));
				return;
			}
			try {
				std::thread(&DirTask::updateStatusBar, this, file.path().string() + "(...)").detach();
				std::thread(&DirTask::updateStatus, this, "Writing...").detach(); // set to 0% as is calling next file
				GoodBye::overwriteBytesMultiple(
				    file.path().string(), 4096 * 1024, standards::STANDARDS[mode_],
				    [&](size_t a, size_t b, size_t c) {
					    double proportion =
					        static_cast<double>((c * b) + a) / static_cast<double>(b * standards::STANDARDS[mode_].size());
					    std::stringstream summary;
					    summary << std::setprecision(3) << 100.0 * proportion;

					    std::thread(&DirTask::updateStatus, this, std::string("Writing: ") + summary.str() + "%").detach();
					    std::thread(&DirTask::updateStatusBar, this, file.path().string() + "(" + summary.str() + " %)")
					        .detach();
					    std::thread(&DirTask::updateProgressBar, this, proportion).detach();
				    },
				    [&](size_t, size_t) {
					    // mark the file as deleted
					    try {
						    std::experimental::filesystem::remove(file.path().string());
					    } catch (std::experimental::filesystem::filesystem_error &e) {
						    updateStatus("Error");
						    error_ = true;
						    updateProgressBar(0);

						    errorMessage_ = file.path().string() + ": " + e.what();
						    errorMessages_.push_back(errorMessage_);
					    }

					    std::thread(&DirTask::updateProgressBar, this, 0).detach(); // set to 0% as is calling next file
				    });
				wxPuts("end");
			} catch (std::runtime_error &e) {
				updateStatus("Error");
				updateProgressBar(0);
				error_ = true;

				// call the next task
				errorMessage_ = file.path().string() + ": " + e.what();
				errorMessages_.push_back(errorMessage_);
				wxQueueEvent(frameInstance, new wxCommandEvent(CALL_NEXT_TASK));
				wxQueueEvent(frameInstance, new wxCommandEvent(HIDE_STATUS));
			}
		}

		std::thread(&DirTask::updateProgressBar, this, 1).detach(); // set to 0% as is calling next file
		locked_ = false;

		if (!error_) {
			updateStatus("Completed");
			error_     = false;
			completed_ = true;

			try {
				// mark the folder as deleted
				std::experimental::filesystem::remove_all(path_);
			} catch (std::experimental::filesystem::filesystem_error &e) {
				updateStatus("Error");
				error_ = true;

				errorMessage_ = e.what();
				errorMessages_.push_back(errorMessage_);
			}
		} else {
			updateProgressBar(0);
		}

		// call the next task
		wxQueueEvent(frameInstance, new wxCommandEvent(HIDE_STATUS));
		wxQueueEvent(frameInstance, new wxCommandEvent(CALL_NEXT_TASK));
	})
	    .detach();
}

// void DirTask::updateContents() {
// 	locked_    = true;
// 	paths_     = {};
// 	filePaths_ = {};

// 	size_t culminativeSize = 0;
// 	size_t count           = 0;
// 	for (const auto &file : std::experimental::filesystem::recursive_directory_iterator(path_)) {

// 		paths_.push_back(file.path().string());
// 		if (std::experimental::filesystem::is_regular_file(file.path().string()) ||
// 		    std::experimental::filesystem::is_block_file(file.path().string()) ||
// 		    std::experimental::filesystem::is_character_file(file.path().string())) {
// 			filePaths_.push_back(file.path().string());
// 			culminativeSize += GoodBye::getSize(file.path().string());
// 		}
// 		if (!(++count % SKIP)) { // update gui every SKIP items
// 			std::thread([=]() {
// 				wxCommandEvent *evt = new wxCommandEvent(UPDATE_VALUE);
// 				evt->SetClientData(new UpdateProgressData{"Calculating: " + std::to_string(culminativeSize),
// 				                                          frameInstance->queueCtrl_->ItemToRow(queueRow_), 2});
// 				wxQueueEvent(frameInstance, evt);
// 			})
// 			    .detach();
// 			std::thread(&DirTask::updateStatusBar, this, file.path().string()).detach();
// 		}
// 	}

// 	wxCommandEvent *evt = new wxCommandEvent(UPDATE_VALUE);
// 	evt->SetClientData(
// 	    new UpdateProgressData{std::to_string(culminativeSize), frameInstance->queueCtrl_->ItemToRow(queueRow_), 2});
// 	wxQueueEvent(frameInstance, evt);

// 	locked_ = false;
// }

const std::pair<std::string, std::vector<std::string>> DirTask::getError() const {
	if (errorMessages_.empty())
		return std::make_pair(path_, std::vector<std::string>{});
	return std::make_pair(path_, errorMessages_);
}

void DirTask::reset() {
	Task::reset();
	errorMessages_.clear();
}