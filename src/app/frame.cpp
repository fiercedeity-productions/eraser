#pragma once
#include "frame.h"

Frame::Frame()
    : wxFrame(nullptr, wxID_HOME, "Eraser") {
	Show();
}