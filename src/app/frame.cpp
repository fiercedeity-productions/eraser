#pragma once
#include "frame.h"
#include "generator.h"
#include "standards.h"
#include "storeddata.h"
#include "task.h"
#include "updateprogressdata.h"
#include <filesystem>
#include <numeric>
#include <regex>
#include <set>
#include <sys/stat.h>
#include <wx/statline.h>

wxDEFINE_EVENT(CALL_NEXT_TASK, wxCommandEvent);
wxDEFINE_EVENT(UPDATE_VALUE, wxCommandEvent);
wxDEFINE_EVENT(SET_STATUS, wxCommandEvent);
wxDEFINE_EVENT(HIDE_STATUS, wxCommandEvent);
wxDEFINE_EVENT(SHOW_STATUS, wxCommandEvent);

BEGIN_EVENT_TABLE(Frame, wxFrame)
EVT_COMMAND(wxID_ANY, CALL_NEXT_TASK, Frame::callNextTask)
EVT_COMMAND(wxID_ANY, UPDATE_VALUE, Frame::onUpdateValue)
EVT_COMMAND(wxID_ANY, SET_STATUS, Frame::onSetStatus)
EVT_COMMAND(wxID_ANY, HIDE_STATUS, Frame::onHideStatus)
EVT_COMMAND(wxID_ANY, SHOW_STATUS, Frame::onShowStatus)
EVT_TEXT(wxID_FILE1, Frame::onChangeText)
EVT_TEXT_ENTER(wxID_FILE1, Frame::onEnter)
EVT_BUTTON(wxID_FILE2, Frame::addFileDialog)
EVT_BUTTON(wxID_FILE3, Frame::addDirDialog)
EVT_CHOICE(wxID_PREFERENCES, Frame::onEdit)
EVT_BUTTON(wxID_EXECUTE, Frame::onControlButton)
EVT_SIZE(Frame::onSize)
EVT_DATAVIEW_ITEM_VALUE_CHANGED(wxID_PROPERTIES, Frame::onQueueChanged)
EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_PROPERTIES, Frame::onQueueContextMenu)
EVT_DATAVIEW_SELECTION_CHANGED(wxID_PROPERTIES, Frame::onChangeSelection)
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

	queueCtrl_ = new wxDataViewListCtrl(panel_, wxID_PROPERTIES, wxDefaultPosition, wxDefaultSize,
	                                    wxDV_MULTIPLE | wxDV_HORIZ_RULES | wxDV_VERT_RULES);

	pathCtrl_  = new wxTextCtrl(panel_, wxID_FILE1, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                               wxTE_PROCESS_TAB | wxTE_PROCESS_ENTER);
	addFiles_  = new wxButton(panel_, wxID_FILE2, "Files...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	addFolder_ = new wxButton(panel_, wxID_FILE3, "Folder...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

	controlButton_ = new wxButton(panel_, wxID_EXECUTE, "Start", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	controlButton_->Disable(); // disabled initially as queue is initially empty

	// change standards::NAMES vector into a wxVector
	wxArrayString choices;
	for (std::string i : standards::NAMES)
		choices.push_back(i);
	modeChoice_ = new wxChoice(panel_, wxID_PREFERENCES, wxDefaultPosition, wxDefaultSize, choices);
	modeChoice_->SetSelection(0); // default selection is the first item

	addSizer_->Add(pathCtrl_, wxSizerFlags(1).Expand().Border(wxRIGHT, borderSize_ * scalingFactor_));
	addSizer_->Add(addFiles_, wxSizerFlags(0).Expand().Border(wxRIGHT, borderSize_ * scalingFactor_));
	addSizer_->Add(addFolder_, wxSizerFlags(0).Expand());

	controlsSizer_->Add(controlButton_, wxSizerFlags(1).Proportion(1).Expand().Border(wxRIGHT, borderSize_ * scalingFactor_));
	controlsSizer_->Add(modeChoice_, wxSizerFlags(1).Proportion(4).Expand());

	sizer_->Add(Generator::createLabel(panel_, borderSize_, scalingFactor_, "Add Files and Folders"), wxSizerFlags(0).Expand());
	sizer_->Add(addSizer_, wxSizerFlags(0).Expand().Border(wxALL & ~wxUP & ~wxDOWN, borderSize_ * scalingFactor_));
	sizer_->Add(Generator::createLabel(panel_, borderSize_, scalingFactor_, "Controls"), wxSizerFlags(0).Expand());
	sizer_->Add(controlsSizer_, wxSizerFlags(0).Expand().Border(wxALL & ~wxUP & ~wxDOWN, borderSize_ * scalingFactor_));
	sizer_->Add(Generator::createLabel(panel_, borderSize_, scalingFactor_, "Erase Queue"), wxSizerFlags(0).Expand());
	sizer_->Add(queueCtrl_, wxSizerFlags(1).Expand().Border(wxALL & ~wxUP, borderSize_ * scalingFactor_));

	pathCtrl_->Bind(wxEVT_CHAR, &Frame::onKeyDown, this); // use to catch key presses like tab and backspace
	queueCtrl_->Bind(wxEVT_CHAR, &Frame::onQueueKeyDown, this);

	// configure the property grid
	queueCtrlCol1_ = queueCtrl_->AppendTextColumn("Path", wxDATAVIEW_CELL_INERT);
	queueCtrlCol2_ = queueCtrl_->AppendTextColumn("Mode", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE);
	queueCtrlCol3_ = queueCtrl_->AppendTextColumn("Size (Bytes)", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE);
	queueCtrlCol4_ = queueCtrl_->AppendTextColumn("Status", wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE);
	queueCtrlCol5_ = queueCtrl_->AppendProgressColumn("Progress", wxDATAVIEW_CELL_INERT, scalingFactor_ * 128);

	queueCtrlCol1_->SetResizeable(false);
	queueCtrlCol2_->SetResizeable(false);
	queueCtrlCol3_->SetResizeable(false);
	queueCtrlCol4_->SetResizeable(false);
	queueCtrlCol5_->SetResizeable(false);

	CreateStatusBar();
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

void Frame::onControlButton(wxCommandEvent &evt) {
	if (!running_) {
		callNextTask(evt);
	} else if (running_ && !paused_) {
		paused_ = true;
		controlButton_->SetLabelText("Stopping...");
	} else if (running_ && paused_) {
		paused_ = false;
		controlButton_->SetLabelText("Stop");
	}
}

void Frame::callNextTask(wxCommandEvent &evt) {
	if (!paused_) {
		running_ = Task::callNext();
		if (running_)
			controlButton_->SetLabelText("Stop");
		else
			controlButton_->SetLabelText("Start");
	} else {
		controlButton_->SetLabelText("Start");
		paused_  = false;
		running_ = false;
	}

	onQueueChanged();
	evt.Skip();
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

	if (paths.empty())
		return;
	// separate the paths by semicolons
	std::string text;
	text = std::accumulate(std::next(paths.begin()), paths.end(), *paths.begin(),
	                       [](std::string a, std::string b) { return a + "; " + b; });

	// set the textbox text
	pathCtrl_->ChangeValue(text);
	// clear queue selection
	queueCtrl_->SetSelections(wxDataViewItemArray());
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
	// clear queue selection
	queueCtrl_->SetSelections(wxDataViewItemArray());
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

	// clear queue selection
	queueCtrl_->SetSelections(wxDataViewItemArray());
	// finally add to the queue
	for (auto i : splitPaths)
		addToQueue(i);
}

void Frame::addToQueue(std::string path) {
	// add to selection
	wxDataViewItemArray sel;
	queueCtrl_->GetSelections(sel);
	sel.push_back(Task::add(path, static_cast<standards::standard>(modeChoice_->GetSelection())));
	queueCtrl_->SetSelections(sel);
}

void Frame::resizeColumns() {
	queueCtrlCol1_->SetWidth(queueCtrl_->GetSize().GetX() - queueCtrlCol2_->GetWidth() - queueCtrlCol3_->GetWidth() -
	                         queueCtrlCol4_->GetWidth() - (scalingFactor_ * 128));
	queueCtrlCol5_->SetWidth(scalingFactor_ * 128);
}

void Frame::onSize(wxSizeEvent &evt) {
	resizeColumns();
	evt.Skip();
}

void Frame::onQueueKeyDown(wxKeyEvent &evt) {
	// if del is entered, remove the selections
	if (evt.GetKeyCode() == 127) {
		wxDataViewItemArray sel;
		queueCtrl_->GetSelections(sel);

		for (auto i : sel) {
			Task *task = reinterpret_cast<StoredData *>(queueCtrl_->GetItemData(i))->task;
			Task::removeByTaskPtr(task);
		}
	}

	evt.Skip();
}

void Frame::onEdit(wxCommandEvent &evt) {
	// edit selections!
	for (auto i : getUnlockedSelections()) {
		reinterpret_cast<StoredData *>(queueCtrl_->GetItemData(i))
		    ->task->setMode(static_cast<standards::standard>(modeChoice_->GetSelection()));
	}
}

const std::vector<wxDataViewItem> Frame::getUnlockedSelections() {
	wxDataViewItemArray sel;
	queueCtrl_->GetSelections(sel);
	// remove selections that are locked
	std::vector<wxDataViewItem> selVec;
	for (auto i : sel)
		selVec.emplace_back(i);

	selVec.erase(std::remove_if(selVec.begin(), selVec.end(),
	                            [&](const wxDataViewItem &i) {
		                            Task *task = reinterpret_cast<StoredData *>(queueCtrl_->GetItemData(i))->task;
		                            return task->completed_ || task->locked_;
	                            }),
	             selVec.end());

	return selVec;
}

void Frame::onQueueChanged(wxDataViewEvent &evt) {
	onQueueChanged();
}

void Frame::onQueueChanged() {
	resizeColumns();
	// enable or disable the start button depending on whether there are tasks
	controlButton_->Enable(!running_ && !Task::isEmpty() || running_);
}

void Frame::onUpdateValue(wxCommandEvent &evt) {
	std::unique_lock<std::mutex> lk(mut);

	UpdateProgressData *data = reinterpret_cast<UpdateProgressData *>(evt.GetClientData());

	queueCtrl_->SetValue(reinterpret_cast<UpdateProgressData *>(evt.GetClientData())->value,
	                     reinterpret_cast<UpdateProgressData *>(evt.GetClientData())->row,
	                     reinterpret_cast<UpdateProgressData *>(evt.GetClientData())->col);

	// std::cout << "value " << data->value << " row " << data->row << " column " << data->col << std::endl;
}

void Frame::onQueueContextMenu(wxDataViewEvent &evt) {
	wxDataViewItem item = evt.GetItem();
	if (!item.IsOk()) // discontinue computation if no item is clicked
		return;

	// store the selected items' task pointers
	std::vector<Task *> tasks;
	wxDataViewItemArray sel;
	queueCtrl_->GetSelections(sel);
	for (const auto &i : sel)
		tasks.push_back(reinterpret_cast<StoredData *>(queueCtrl_->GetItemData(i))->task);

	// count number of flags
	int numErrors   = std::count_if(tasks.begin(), tasks.end(), [](Task *t) { return t->error_; });
	int numUnlocked = std::count_if(tasks.begin(), tasks.end(), [](Task *t) { return !t->locked_; });
	int numReady =
	    std::count_if(tasks.begin(), tasks.end(), [](Task *t) { return !t->completed_ && !t->error_ && !t->locked_; });

	// populate the menu
	wxMenu menu;
	if (numUnlocked > 0)
		menu.Append(0, "Remove", nullptr);

	if (numErrors > 0)
		menu.Append(1, "Reset Status", nullptr); // only allow to reset status if the errors in the selection are more than 1

	if (numReady > 0) {
		wxMenu *standardMenu = new wxMenu;
		int     id           = 10;
		for (std::string i : standards::NAMES) {
			standardMenu->Append(id++, i, nullptr);
		}
		menu.Append(2, "Change Standard", standardMenu);
	}

	// programme the menu
	menu.Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &evt) {
		if (evt.GetId() == 0) { // remove
			for (Task *const t : tasks)
				Task::removeByTaskPtr(t);
		} else if (evt.GetId() == 1) { // reset status
			for (Task *const t : tasks) {
				if (!t->locked_ && !t->completed_)
					t->reset();
			}
		} else if (evt.GetId() >= 10 && evt.GetId() < 10 + standards::STANDARDS.size()) { // change standard
			for (Task *const t : tasks) {
				t->setMode(static_cast<standards::standard>(evt.GetId() - 10));
			}
		}
	});

	// show the menu
	PopupMenu(&menu);

	evt.Skip();
}

void Frame::onChangeSelection(wxDataViewEvent &evt) {
	// set the value of the choice dropdown to the mode value of the first item in the selecion
	wxDataViewItemArray sel;
	queueCtrl_->GetSelections(sel);
	if (sel.empty())
		return;

	modeChoice_->SetSelection(reinterpret_cast<StoredData *>(queueCtrl_->GetItemData(sel.front()))->task->mode_);
}

void Frame::onSetStatus(wxCommandEvent &evt) {
	this->SetStatusText(evt.GetString());
}

void Frame::onHideStatus(wxCommandEvent &evt) {
	if (GetStatusBar()->IsShown())
		GetStatusBar()->Hide();
}

void Frame::onShowStatus(wxCommandEvent &evt) {
	if (!GetStatusBar()->IsShown())
		GetStatusBar()->Show();
}