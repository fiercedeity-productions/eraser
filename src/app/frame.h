#pragma once
#include <wx/dataview.h>
#include <wx/wx.h>

extern const wxEventTypeTag<wxCommandEvent> UPDATE_PROGRESS;

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

	wxTextCtrl *      pathCtrl_;
	wxButton *        addFiles_;
	wxButton *        addFolder_;
	wxButton *        controlButton_;
	wxGauge *         progress_;
	wxDataViewColumn *queueCtrlCol1_, *queueCtrlCol2_, *queueCtrlCol3_;

	double borderSize_ = 8;
	bool   allowEdits_;

	void onUpdateProgress(wxCommandEvent &evt);
	void onChangeText(wxCommandEvent &evt);
	void onKeyDown(wxKeyEvent &evt);
	void onQueueKeyDown(wxKeyEvent &evt);
	void onEnter(wxCommandEvent &evt);
	void addFileDialog(wxCommandEvent &evt);
	void addDirDialog(wxCommandEvent &evt);

	void addToQueue(std::string path);

	void onSize(wxSizeEvent &evt);
	void onSelection(wxDataViewEvent &evt);

  public:
	wxDataViewListCtrl *queueCtrl_;
	void                resizeColumns();
	Frame();
	~Frame();
};