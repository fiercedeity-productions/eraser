#pragma once
#include "frame.h"
#include "generator.h"
#include "task.h"
#include <filesystem>
#include <numeric>
#include <regex>
#include <wx/statline.h>

wxDEFINE_EVENT(UPDATE, wxCommandEvent); // for updating the progress bar

BEGIN_EVENT_TABLE(Frame, wxFrame)
EVT_COMMAND(wxID_ANY, UPDATE, Frame::onUpdate)
EVT_TEXT(wxID_FILE1, Frame::onChangeText)
EVT_BUTTON(wxID_FILE2, Frame::addFileDialog)
EVT_BUTTON(wxID_FILE3, Frame::addDirDialog)
END_EVENT_TABLE()

Frame::Frame()
    : wxFrame(nullptr, wxID_HOME, "Eraser") {
	// get scaling factor
	HDC    screen = GetDC(NULL);
	double hDpi   = GetDeviceCaps(screen, LOGPIXELSX);
	ReleaseDC(NULL, screen);

	scalingFactor_ = hDpi / 96;

	// configure gui
	panel_         = new wxPanel(this, wxID_ANY);
	sizer_         = new wxBoxSizer(wxVERTICAL);
	addSizer_      = new wxBoxSizer(wxHORIZONTAL);
	controlsSizer_ = new wxBoxSizer(wxHORIZONTAL);
	queueCtrl_     = new wxListCtrl(panel_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

	pathCtrl_  = new wxTextCtrl(panel_, wxID_FILE1, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                               wxTE_PROCESS_TAB | wxTE_PROCESS_ENTER);
	addFiles_  = new wxButton(panel_, wxID_FILE2, "Files...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	addFolder_ = new wxButton(panel_, wxID_FILE3, "Folder...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

	controlButton_ = new wxButton(panel_, wxID_ANY, "Start", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	progress_ =
	    new wxGauge(panel_, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL | wxGA_SMOOTH | wxGA_PROGRESS);

	addSizer_->Add(pathCtrl_, wxSizerFlags(1).Expand().Border(wxRIGHT, borderSize_ * scalingFactor_));
	addSizer_->Add(addFiles_, wxSizerFlags(0).Expand().Border(wxRIGHT, borderSize_ * scalingFactor_));
	addSizer_->Add(addFolder_, wxSizerFlags(0).Expand());

	controlsSizer_->Add(controlButton_, wxSizerFlags(0).Expand().Border(wxRIGHT, borderSize_ * scalingFactor_));
	controlsSizer_->Add(progress_, wxSizerFlags(1).Expand());

	sizer_->Add(Generator::createLabel(panel_, borderSize_, scalingFactor_, "Add Files and Folders"), wxSizerFlags(0).Expand());
	sizer_->Add(addSizer_, wxSizerFlags(0).Expand().Border(wxALL & ~wxUP & ~wxDOWN));
	sizer_->Add(Generator::createLabel(panel_, borderSize_, scalingFactor_, "Erase Queue"), wxSizerFlags(0).Expand());
	sizer_->Add(queueCtrl_, wxSizerFlags(1).Expand().Border(wxALL & ~wxUP & ~wxDOWN));
	sizer_->Add(Generator::createLabel(panel_, borderSize_, scalingFactor_, "Controls"), wxSizerFlags(0).Expand());
	sizer_->Add(controlsSizer_, wxSizerFlags(0).Expand().Border(wxALL & ~wxUP));

	pathCtrl_->Bind(wxEVT_CHAR, &Frame::onKeyDown, this); // use to catch key presses like tab and backspace

	panel_->SetSizerAndFit(sizer_);
	SetSize(800 * scalingFactor_, 600 * scalingFactor_);
	Show();
}

void Frame::onUpdate(wxCommandEvent &evt) {
	// wxPuts("update");
}

void Frame::addFileDialog(wxCommandEvent &evt) {
	wxFileDialog fileDialog(this, "Select Files to be Erased", "/", "", "All Files (*.*) | *.*",
	                        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (fileDialog.ShowModal() == wxID_CANCEL)
		return; // discontinue computation if user cancels

	// retrieve paths
	wxArrayString wxPaths;
	fileDialog.GetPaths(wxPaths);

	// turn paths into standard array
	std::vector<std::string> paths;
	std::transform(wxPaths.begin(), wxPaths.end(), std::back_inserter(paths), [](wxString str) { return str.ToStdString(); });

	// separate the paths by semicolons
	std::string text;
	text = std::accumulate(std::next(paths.begin()), paths.end(), *paths.begin(),
	                       [](std::string a, std::string b) { return a + "; " + b; });

	// set the textbox text
	pathCtrl_->ChangeValue(text);
}

void Frame::addDirDialog(wxCommandEvent &evt) {
	wxDirDialog dirDialog(this, "Select Folder to be Erased", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dirDialog.ShowModal() == wxID_CANCEL)
		return; // discontinue computation if user cancels

	// set the textbox text
	pathCtrl_->ChangeValue(dirDialog.GetPath());
}

void Frame::onChangeText(wxCommandEvent &evt) {
	if (!allowEdits_)
		return;
	// get the value of the textbox
	std::string userValue = evt.GetString().ToStdString();

	if (userValue.empty())
		return; // do nothing if the textbox is empty
		        // replace all / with \ or \ with / depending on os

	long ins = pathCtrl_->GetInsertionPoint();
	long selBeg, selEnd;
	pathCtrl_->GetSelection(&selBeg, &selEnd);

	std::string value =
	    std::regex_replace(evt.GetString().ToStdString(), std::regex(";(?!\\s)"), "; "); // replace ";" with "; "
	size_t                   elementBegin = 0;
	size_t                   cursor       = 0;
	std::vector<std::string> splitPaths;

#if defined(unix) || defined(__unix__) || defined(__unix)
	std::replace(userValue.begin(), userValue.end(), '\\', '/');
	pathCtrl_->ChangeValue(userValue);
	pathCtrl_->SetInsertionPoint(ins);
	pathCtrl_->SetSelection(selBeg, selEnd);
#else
	std::replace(userValue.begin(), userValue.end(), '/', '\\');
	pathCtrl_->ChangeValue(userValue);
	pathCtrl_->SetInsertionPoint(ins);
	pathCtrl_->SetSelection(selBeg, selEnd);
#endif

	// split the text by "; "
	while (cursor < value.length()) {
		if (value.length() - cursor > 1 && value.substr(cursor, 2) == "; ") {
			splitPaths.push_back(value.substr(elementBegin, cursor - elementBegin));
			cursor += 2;
			elementBegin = cursor;
			continue;
		} else if (cursor == value.length() - 1) {
			splitPaths.push_back(value.substr(elementBegin));
		}

		++cursor;
	}

	// remove empty items in the vector (e.g.: "a;b;;c")
	//												^ empty item
	std::remove_if(splitPaths.begin(), splitPaths.end(), [](std::string str) { return str.empty(); });

	// for (auto i = splitPaths.begin(); i != splitPaths.end(); ++i)
	// 	wxPuts(std::to_string(i - splitPaths.begin()) + " " + *i);

	// calculate the current element being edited
	size_t editBeg =
	    userValue.substr(0, ins).find_last_of(';') == std::string::npos ? 0 : userValue.substr(0, ins).find_last_of(';');
	size_t editEnd = editBeg + 1 +
	                 (editBeg + 1 >= userValue.length() || userValue.substr(editBeg + 1).find_first_of(';') == std::string::npos
	                      ? userValue.substr(editBeg + 1).length()
	                      : userValue.substr(editBeg + 1).find_first_of(';'));

	if (userValue[editBeg] == ';')
		++editBeg;
	while (userValue[editBeg] == ' ')
		++editBeg;

	std::string field = userValue.substr(editBeg, editEnd - editBeg);

	// only suggest possibilities if the cursor is at the end of the editing field
	if (ins == editEnd) {
		// get path
#if defined(unix) || defined(__unix__) || defined(__unix)
		std::string parent;
		bool        root = std::count_if(field.begin(), field.end(), [](char c) { return c == '/'; }) == 1;
		if (root)
			parent = "/";
		else
			parent = field.substr(0, field.find_last_of('/'));
#else
		std::string parent;
		bool        root = std::count_if(field.begin(), field.end(), [](char c) { return c == '\\'; }) == 1;
		if (root && field[0] == '\\')
			parent = '\\';
		else if (root && field.length() >= 3)
			parent = field.substr(0, 3);
		else if (field.length() >= 3)
			parent = field.substr(0, field.find_last_of('\\'));
#endif
		for (const auto &file : std::experimental::filesystem::directory_iterator(parent)) {
			std::string filePath    = file.path().string();
			std::string matchedArea = filePath.substr(0, field.length());

			// transform into lower case for comparison on windows only (non-*nix systems)
			std::string fieldAdjusted;
#if !defined(unix) && !defined(__unix__) && !defined(__unix)
			std::transform(matchedArea.begin(), matchedArea.end(), matchedArea.begin(), ::tolower);
			std::transform(field.begin(), field.end(), std::back_inserter(fieldAdjusted), ::tolower);
#endif

			if (fieldAdjusted == matchedArea) {
				std::string suggestion = filePath.substr(field.length());
				pathCtrl_->ChangeValue(userValue.substr(0, ins) + suggestion + userValue.substr(ins));
				pathCtrl_->SetSelection(ins, ins + suggestion.length());
				break;
			}
		}
	}
}

void Frame::onKeyDown(wxKeyEvent &evt) {
	if (evt.GetKeyCode() == 27) { // delete the suggestion if escape is pressed
		wxPuts("yes");
		allowEdits_ = false;
		pathCtrl_->RemoveSelection();
		evt.Skip();
	} else if (evt.GetKeyCode() == 8) { // do not activate autocomplete if backspace is entered
		allowEdits_ = false;
		evt.Skip();
	} else if (evt.GetKeyCode() == 9) { // use tab to complete autocomplete
		long selBeg, selEnd;
		pathCtrl_->GetSelection(&selBeg, &selEnd);
		pathCtrl_->SetSelection(selEnd, selEnd);
		pathCtrl_->SetInsertionPoint(selEnd);
		evt.StopPropagation(); // do not skip as tab would be inserted
	} else {
		allowEdits_ = true;
		evt.Skip();
	}
}