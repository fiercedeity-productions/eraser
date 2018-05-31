#pragma once
#include <wx/listctrl.h>
#include <wx/wx.h>

extern const wxEventTypeTag<wxCommandEvent> UPDATE;

class Frame : public wxFrame {
  private:
	// add custom commands
	DECLARE_EVENT_TABLE()

	// ui elements
	double      scalingFactor_;
	wxPanel *   panel_;
	wxBoxSizer *sizer_;
	wxBoxSizer *addSizer_;
	wxBoxSizer *controlsSizer_;

	wxTextCtrl *pathCtrl_;
	wxButton *  addFiles_;
	wxButton *  addFolder_;
	wxListCtrl *queueCtrl_;
	wxButton *  controlButton_;
	wxGauge *   progress_;

	double borderSize_ = 8;

	void onUpdate(wxCommandEvent &evt);
	void onChangeText(wxCommandEvent &evt);
	void onKeyDown(wxKeyEvent &evt);
	void addFileDialog(wxCommandEvent &evt);
	void addDirDialog(wxCommandEvent &evt);

	bool allowEdits_;

  public:
	Frame();
};