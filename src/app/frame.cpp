#pragma once
#include "frame.h"
#include "generator.h"
#include "storeddata.h"
#include "task.h"
#include <filesystem>
#include <numeric>
#include <regex>
#include <set>
#include <sys/stat.h>
#include <wx/statline.h>

wxDEFINE_EVENT(UPDATE_PROGRESS, wxCommandEvent); // for updating the progress bar

BEGIN_EVENT_TABLE(Frame, wxFrame)
EVT_COMMAND(wxID_ANY, UPDATE_PROGRESS, Frame::onUpdateProgress)
EVT_TEXT(wxID_FILE1, Frame::onChangeText)
EVT_TEXT_ENTER(wxID_FILE1, Frame::onEnter)
EVT_BUTTON(wxID_FILE2, Frame::addFileDialog)
EVT_BUTTON(wxID_FILE3, Frame::addDirDialog)
EVT_SIZE(Frame::onSize)
EVT_DATAVIEW_SELECTION_CHANGED(wxID_PROPERTIES, Frame::onSelection)
END_EVENT_TABLE()

struct insensitiveComp {
	bool operator()(const std::string &a, const std::string &b) {
		return stricmp(a.c_str(), b.c_str()) < 0;
	}
};

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
	queueCtrl_     = new wxDataViewListCtrl(panel_, wxID_PROPERTIES, wxDefaultPosition, wxDefaultSize,
                                        wxDV_MULTIPLE | wxDV_HORIZ_RULES | wxDV_VERT_RULES);

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
	queueCtrl_->Bind(wxEVT_CHAR, &Frame::onQueueKeyDown, this);

	// configure the property grid
	queueCtrlCol1_ = queueCtrl_->AppendTextColumn("Path", wxDATAVIEW_CELL_INERT);
	queueCtrlCol2_ = queueCtrl_->AppendTextColumn("Status", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE);
	queueCtrlCol3_ = queueCtrl_->AppendProgressColumn("Progress", wxDATAVIEW_CELL_INERT);

	queueCtrlCol1_->SetResizeable(false);
	queueCtrlCol2_->SetResizeable(false);
	queueCtrlCol3_->SetResizeable(false);

	panel_->SetSizerAndFit(sizer_);
	SetSize(800 * scalingFactor_, 600 * scalingFactor_);
	Show();
	resizeColumns();
	Task::setFrame(this);
}

Frame::~Frame() {
	// empty the array of task pointers
	Task::empty();
}

void Frame::onUpdateProgress(wxCommandEvent &evt) {
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

	// count the number of invaid paths
	// TODO: also erase if already in the queue
	size_t      removedPaths = 0;
	std::string removedPath;
	paths.erase(std::remove_if(paths.begin(), paths.end(),
	                           [&](std::string path) {
		                           struct stat perms;
		                           int         res = stat(path.c_str(), &perms);
		                           if (~perms.st_mode & (S_IWRITE | S_IREAD) || res == -1 || Task::inQueue(path)) {
			                           ++removedPaths;
			                           removedPath = path;
			                           return true;
		                           }
		                           return false;
	                           }),
	            paths.end());

	if (removedPaths > 0)
		wxMessageBox(removedPaths == 1
		                 ? "\"" + removedPath + "\" does not exist or is unable to be accessed. It will not be added."
		                 : std::to_string(removedPaths) +
		                       " of these files do not exist or are unable to be accessed. They will not be added.",
		             "Invalid Files", 5L, this);

	// separate the paths by semicolons
	std::string text;
	text = std::accumulate(std::next(paths.begin()), paths.end(), *paths.begin(),
	                       [](std::string a, std::string b) { return a + "; " + b; });

	// set the textbox text
	pathCtrl_->ChangeValue(text);
	// add to the queue
	for (std::string str : paths)
		addToQueue(str);
}

void Frame::addDirDialog(wxCommandEvent &evt) {
	wxDirDialog dirDialog(this, "Select Folder to be Erased", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (dirDialog.ShowModal() == wxID_CANCEL)
		return; // discontinue computation if user cancels

	std::string path = dirDialog.GetPath().ToStdString();
	// check permissions
	// TODO: also erase if already in the queue
	struct stat perms;
	int         res = stat(path.c_str(), &perms);
	if (~perms.st_mode & (S_IWRITE | S_IREAD) || res == -1 || Task::inQueue(path)) {
		wxMessageBox("Insufficient permissions for \"" + path + "\". It will not be added.", "Folder Permissions", 5L, this);
		return;
	}

	// set the textbox text
	pathCtrl_->ChangeValue(path);
	// add to queue
	addToQueue(path);
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

	// replace \ by / or vice versa

	char dirSeparator;
	char replaceDirSeparator;

#if defined(unix) || defined(__unix__) || defined(__unix)
	dirSeparator        = '/';
	replaceDirSeparator = '\\';
#else
	dirSeparator        = '\\';
	replaceDirSeparator = '/';
#endif

	std::replace(userValue.begin(), userValue.end(), replaceDirSeparator, dirSeparator);
	pathCtrl_->ChangeValue(userValue);
	pathCtrl_->SetInsertionPoint(ins);
	pathCtrl_->SetSelection(selBeg, selEnd);

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
		std::string parent;
		bool        root =
		    std::count_if(field.begin(), field.end(), [=](char c) { return c == dirSeparator; }) == 1 && field[0] != '.';
#if defined(unix) || defined(__unix__) || defined(__unix)
		if (root && field[0] == dirSeparator)
			parent = dirSeparator;
		else if (field[0] != '.')
			parent = field.substr(0, field.find_last_of(dirSeparator));
		else
			return;
#else
		if (root && field[0] == dirSeparator)
			parent = dirSeparator;
		else if (root && field.length() >= 3)
			parent = field.substr(0, 3);
		else if (field.length() >= 3 && field[0] != '.')
			parent = field.substr(0, field.find_last_of(dirSeparator));
		else
			return;
#endif
		for (const auto &file : std::experimental::filesystem::directory_iterator(parent)) {
			std::string filePath = file.path().string();
			// if filePath is a directory, add a trailing "/" or "\"
			if (std::experimental::filesystem::is_directory(filePath)) {
				filePath += dirSeparator;
			}
			std::string matchedArea = filePath.substr(0, field.length());

			// transform into lower case for comparison on windows only (non-*nix systems)
			std::string fieldAdjusted;
#if !defined(unix) && !defined(__unix__) && !defined(__unix)
			std::transform(matchedArea.begin(), matchedArea.end(), matchedArea.begin(), ::tolower);
			std::transform(field.begin(), field.end(), std::back_inserter(fieldAdjusted), ::tolower);
#endif

			struct stat perms;
			stat(filePath.c_str(), &perms);
			if (~perms.st_mode & (S_IWRITE | S_IREAD))
				continue;

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
	long selBeg, selEnd;
	pathCtrl_->GetSelection(&selBeg, &selEnd);

	if (evt.GetKeyCode() == 27) { // delete the suggestion if escape is pressed
		allowEdits_ = false;
		pathCtrl_->RemoveSelection();
		evt.Skip();
	} else if (evt.GetKeyCode() == 8) { // do not activate autocomplete if backspace is entered
		allowEdits_ = false;
		evt.Skip();
	} else if (evt.GetKeyCode() == 9) { // use tab to complete autocomplete
		pathCtrl_->SetSelection(selEnd, selEnd);
		pathCtrl_->SetInsertionPoint(selEnd);
		evt.StopPropagation(); // do not skip as tab would be inserted
	} else if (evt.GetKeyCode() == 13 && selEnd > selBeg) {
		pathCtrl_->SetSelection(selEnd, selEnd);
		pathCtrl_->SetInsertionPoint(selEnd);
		evt.Skip(); // skip to allow onenter to be triggered
	} else {
		allowEdits_ = true;
		evt.Skip();
	}
}

void Frame::onEnter(wxCommandEvent &evt) {
	if (evt.GetString().IsEmpty())
		return; // do nothing if the textbox is empty
		        // replace all / with \ or \ with / depending on os

	std::string value =
	    std::regex_replace(evt.GetString().ToStdString(), std::regex(";(?!\\s)"), "; "); // replace ";" with "; "
	size_t elementBegin = 0;
	size_t cursor       = 0;

	std::set<std::string, insensitiveComp> splitPaths;

	// split the text by "; "
	while (cursor < value.length()) {
		if (value.length() - cursor > 1 && value.substr(cursor, 2) == "; ") {
			splitPaths.insert(value.substr(elementBegin, cursor - elementBegin));
			cursor += 2;
			elementBegin = cursor;
			continue;
		} else if (cursor == value.length() - 1) {
			splitPaths.insert(value.substr(elementBegin));
		}

		++cursor;
	}

	// add trailing "/" or "\" if folder
	char dirSeparator;
	char replaceDirSeparator;

#if defined(unix) || defined(__unix__) || defined(__unix)
	dirSeparator        = '/';
	replaceDirSeparator = '\\';
#else
	dirSeparator        = '\\';
	replaceDirSeparator = '/';
#endif
	for (auto i = splitPaths.begin(); i != splitPaths.end();) {
		if (std::experimental::filesystem::is_directory(i->c_str()) && i->back() != dirSeparator) {
			splitPaths.insert(std::string(i->c_str()) + dirSeparator);
			i = splitPaths.erase(i);
		} else
			++i;
	}

	// remove empty items in the vector (e.g.: "a;b;;c")
	//												^ empty item
	// TODO: also erase if already in the queue

	size_t      removedPaths = 0;
	std::string removedPath;
	for (auto i = splitPaths.begin(); i != splitPaths.end();) {
		struct stat perms;
		int         res = stat(i->c_str(), &perms);

		if (i->empty() || ~perms.st_mode & (S_IWRITE | S_IREAD) || res == -1 || *i->c_str() == '.' || Task::inQueue(*i)) {
			if (!i->empty())
				++removedPaths;
			removedPath = i->c_str();
			i           = splitPaths.erase(i);
		} else
			++i;
	}

	if (removedPaths > 0)
		wxMessageBox(removedPaths == 1
		                 ? "\"" + removedPath + "\" does not exist or is unable to be accessed. It will not be added."
		                 : std::to_string(removedPaths) +
		                       " of these paths do not exist or are unable to accessed. They will not be added.",
		             "Invalid Paths", 5L, this);

	// join the splitPaths vector and set that value to the pathCtrl_
	if (!splitPaths.empty())
		pathCtrl_->ChangeValue(std::accumulate(std::next(splitPaths.begin()), splitPaths.end(), *splitPaths.begin(),
		                                       [](std::string a, std::string b) { return a + "; " + b; }));
	else
		pathCtrl_->Clear();

	// discontinue computation if the split paths is empty after the above validation
	if (splitPaths.empty())
		return;

	pathCtrl_->SetSelection(pathCtrl_->GetValue().length(), pathCtrl_->GetValue().length());

	// finally add to the queue
	for (auto i : splitPaths)
		addToQueue(i);
}

void Frame::addToQueue(std::string path) {
	Task::add(path);
}

void Frame::resizeColumns() {
	queueCtrlCol1_->SetWidth(queueCtrl_->GetSize().GetX() - queueCtrlCol2_->GetWidth() - (scalingFactor_ * 128));
	queueCtrlCol3_->SetWidth(128);
}

void Frame::onSize(wxSizeEvent &evt) {
	resizeColumns();
	evt.Skip();
}

void Frame::onSelection(wxDataViewEvent &evt) {
	wxDataViewItemArray sel;
	queueCtrl_->GetSelections(sel);
	if (!sel.empty())
		wxPuts(reinterpret_cast<StoredData *>(queueCtrl_->GetItemData(sel[0]))->task->getPath());
}

void Frame::onQueueKeyDown(wxKeyEvent &evt) {
	// if del is entered, remove the selections
	if (evt.GetKeyCode() == 127) {
		wxDataViewItemArray sel;
		queueCtrl_->GetSelections(sel);

		for (auto i : sel) {
			Task *task = reinterpret_cast<StoredData *>(queueCtrl_->GetItemData(i))->task;
			if (!task->isLocked()) {
				Task::removeByTaskPtr(task);
				queueCtrl_->DeleteItem(queueCtrl_->ItemToRow(i));
			}
		}
	}

	evt.Skip();
}