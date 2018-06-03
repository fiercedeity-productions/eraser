#pragma once
#include <wx/listctrl.h>
#include <wx/wx.h>

class ErrorDialog : public wxDialog {
  private:
	DECLARE_EVENT_TABLE()
	wxListCtrl *list_;
	void        onActivate(wxListEvent &evt);
	void        onActivate();
	void        onContext(wxListEvent &evt);

  public:
	ErrorDialog(wxFrame *parent, double borderSize, double scalingFactor,
	            std::vector<std::pair<std::string, std::vector<std::string>>> errors);
};