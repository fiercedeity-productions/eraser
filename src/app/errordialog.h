#pragma once
#include <wx/wx.h>

class ErrorDialog : public wxDialog {
  public:
	ErrorDialog(wxFrame *parent, double borderSize, double scalingFactor,
	            std::vector<std::pair<std::string, std::vector<std::string>>> errors);
};