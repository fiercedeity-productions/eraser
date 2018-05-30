#pragma once

#include "task.h"
#include <thread>

Task::Task(Frame *frameInstance)
    : frameInstance_(frameInstance) {
	std::thread([=]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		wxCommandEvent evt(UPDATE);
		evt.SetString("hi");
		wxPostEvent(frameInstance_, evt);
	})
	    .detach();
}