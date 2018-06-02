#pragma once

#include "dirtask.h"
#include "lib.h"
#include "updateprogressdata.h"
#include <filesystem>
#include <iomanip>
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
	return std::experimental::filesystem::equivalent(pathAbs.substr(0, path_.length()), path_) && !completed_;
}

// const size_t DirTask::getSize() const {
// 	locked_ = true;

// 	size_t culminativeSize = 0;
// 	size_t count           = 0;
// 	for (const std::string &path : filePaths_) {
// 		culminativeSize += Eraser::getSize(path);

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
		wxQueueEvent(frameInstance, new wxCommandEvent(SHOW_STATUS));
		locked_ = true;
		for (auto &file : std::experimental::filesystem::recursive_directory_iterator(path_)) {
			try {
				std::thread(&DirTask::updateStatus, this, "Writing...").detach(); // set to 0% as is calling next file
				Eraser::overwriteBytesMultiple(
				    file.path().string(), 4096 * 1024, standards::STANDARDS[mode_],
				    [&](size_t a, size_t b, size_t c) {
					    double proportion =
					        static_cast<double>((c * b) + a) / static_cast<double>(b * standards::STANDARDS[mode_].size());
					    std::stringstream summary;
					    summary << std::setprecision(3) << 100.0 * proportion;

					    std::thread(&DirTask::updateStatus, this, std::string("Writing: ") + summary.str() + "%").detach();
					    std::thread(&DirTask::updateProgressBar, this, proportion).detach();
					    std::thread(&DirTask::updateStatusBar, this, file.path().string()).detach();
				    },
				    [&](size_t, size_t) {
					    // mark the file as deleted
					    try {
						    std::experimental::filesystem::remove(file.path().string());
					    } catch (std::experimental::filesystem::filesystem_error &e) {
						    updateStatus("Errors");
						    error_ = true;

						    errorMessage_ = e.what();
					    }

					    std::thread(&DirTask::updateProgressBar, this, 0).detach(); // set to 0% as is calling next file
				    });
				wxPuts("end");
			} catch (std::runtime_error &e) {
				updateStatus("Errors");
				locked_    = false;
				completed_ = false;
				error_     = true;
				// TODO: support resetting the status

				// call the next task
				errorMessage_ = e.what();
				wxQueueEvent(frameInstance, new wxCommandEvent(CALL_NEXT_TASK));
				return;
			}
		}

		std::thread(&DirTask::updateProgressBar, this, 1).detach(); // set to 0% as is calling next file
		updateStatus("Completed");

		locked_    = false;
		error_     = false;
		completed_ = true;

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
// 			culminativeSize += Eraser::getSize(file.path().string());
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