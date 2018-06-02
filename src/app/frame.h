#pragma once
#include <mutex>
#include <wx/dataview.h>
#include <wx/wx.h>

extern const wxEventTypeTag<wxCommandEvent> CALL_NEXT_TASK;
extern const wxEventTypeTag<wxCommandEvent> UPDATE_VALUE;
extern const wxEventTypeTag<wxCommandEvent> SET_STATUS;
extern const wxEventTypeTag<wxCommandEvent> HIDE_STATUS;
extern const wxEventTypeTag<wxCommandEvent> SHOW_STATUS;

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
	wxChoice *        modeChoice_;
	wxDataViewColumn *queueCtrlCol1_, *queueCtrlCol2_, *queueCtrlCol3_, *queueCtrlCol4_, *queueCtrlCol5_;

	double borderSize_ = 8;
	bool   allowEdits_;
	bool   running_ = false;

	void callNextTask(wxCommandEvent &evt);
	void onQueueKeyDown(wxKeyEvent &evt);
	void addFileDialog(wxCommandEvent &evt);
	void addDirDialog(wxCommandEvent &evt);

	void addToQueue(std::string path);

	void onSize(wxSizeEvent &evt);
	void onQueueContextMenu(wxDataViewEvent &evt);
	void onEnter(wxCommandEvent &evt);
	void onEdit(wxCommandEvent &evt);
	void onKeyDown(wxKeyEvent &evt);
	void onControlButton(wxCommandEvent &evt);
	void onChangeText(wxCommandEvent &evt);
	void onChangeSelection(wxDataViewEvent &evt);

	void onUpdateValue(wxCommandEvent &evt);
	void onSetStatus(wxCommandEvent &evt);
	void onHideStatus(wxCommandEvent &evt);
	void onShowStatus(wxCommandEvent &evt);

	const std::vector<wxDataViewItem> getUnlockedSelections();

	std::mutex mut;

  public:
	bool                paused_ = false;
	void                onQueueChanged(wxDataViewEvent &evt);
	void                onQueueChanged();
	wxDataViewListCtrl *queueCtrl_;
	void                resizeColumns();
	Frame();
	~Frame();
};