#pragma once
#include "wx/wx.h"

struct UpdateProgressData {
	wxVariant value;
	int       row, col;
};