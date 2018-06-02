#pragma once
#include "filetask.h"
#include "lib.h"
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <thread>

FileTask::FileTask(const std::string &path, const standards::standard &mode)
    : Task(path, mode) {
	// replace "Calculating" displayed by the datactrl to the actual value
	// frameInstance->queueCtrl_->SetValue(std::to_string(getSize()), frameInstance->queueCtrl_->ItemToRow(queueRow_), 2);
	updateSize();
}

const bool FileTask::isIncluded(const std::string &path) const {
	return std::experimental::filesystem::equivalent(path, path_) && !completed_;
}

const size_t FileTask::getSize() const {
	return Eraser::getSize(path_);
};

void FileTask::execute() {
	updateStatus("Writing...");
	std::thread([&]() {
		try {
			updateSize();
			locked_ = true;
			wxPuts("begin");
			Eraser::overwriteBytesMultiple(
			    path_, 4096 * 1024, standards::STANDARDS[mode_],
			    [&](size_t a, size_t b, size_t c) {
				    double proportion =
				        static_cast<double>((c * b) + a) / static_cast<double>(b * standards::STANDARDS[mode_].size());
				    std::stringstream summary;
				    summary << std::setprecision(3) << 100.0 * proportion;

				    std::thread(&FileTask::updateStatus, this, std::string("Writing: ") + summary.str() + "%").detach();

				    std::thread(&FileTask::updateProgressBar, this, proportion).detach();
			    },
			    [&](size_t, size_t) {
				    // mark the file as deleted
				    try {
					    std::experimental::filesystem::remove(path_);
				    } catch (std::experimental::filesystem::filesystem_error &e) {
					    updateStatus("Error");
					    error_ = true;

					    errorMessage_ = e.what();
				    }

				    std::thread(&FileTask::updateProgressBar, this, 1).detach();
				    locked_    = false;
				    error_     = false;
				    completed_ = true;
				    updateStatus("Completed");

				    // call the next task
				    wxQueueEvent(frameInstance, new wxCommandEvent(CALL_NEXT_TASK));
				    return;
			    });
			wxPuts("end");
		} catch (std::runtime_error &e) {
			updateStatus("Error");
			locked_    = false;
			completed_ = false;
			error_     = true;
			// TODO: support resetting the status

			// call the next task
			errorMessage_ = e.what();
			wxQueueEvent(frameInstance, new wxCommandEvent(CALL_NEXT_TASK));
		}
	})
	    .detach();
}

const std::pair<std::string, std::vector<std::string>> FileTask::getError() const {
	return error_ ? std::make_pair(path_, std::vector<std::string>{errorMessage_})
	              : std::make_pair(path_, std::vector<std::string>{});
}