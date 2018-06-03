#pragma once
#include "errordialog.h"
#include "generator.h"
#include <wx/clipbrd.h>
#include <wx/listctrl.h>

BEGIN_EVENT_TABLE(ErrorDialog, wxDialog)
EVT_LIST_ITEM_ACTIVATED(wxID_VIEW_LIST, ErrorDialog::onActivate)
EVT_LIST_ITEM_RIGHT_CLICK(wxID_VIEW_LIST, ErrorDialog::onContext)
END_EVENT_TABLE()

ErrorDialog::ErrorDialog(wxFrame *parent, double borderSize, double scalingFactor,
                         std::vector<std::pair<std::string, std::vector<std::string>>> errors)
    : wxDialog(parent, wxID_ANY, "Errors") {
	wxPanel *   panel = new wxPanel(this);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *label = Generator::createLabel(panel, borderSize, scalingFactor, "Errors");
	list_             = new wxListCtrl(panel, wxID_VIEW_LIST, wxDefaultPosition, wxDefaultSize,
                           wxLC_REPORT & ~wxLC_HRULES | wxLC_VRULES | wxLC_NO_HEADER);

	list_->AppendColumn("Path");
	list_->AppendColumn("Errors");

	long index = 0;
	for (const auto &i : errors) {
		const long indexMajor = list_->InsertItem(index++, i.first);

		list_->SetItem(indexMajor, 1, i.second[0]);
		if (i.second.size() > 1)
			for (auto it = i.second.begin() + 1; it != i.second.end(); ++it) {
				const long indexMinor = list_->InsertItem(index++, "");
				list_->SetItem(indexMinor, 1, *it);
			}
	}

	list_->SetColumnWidth(0, wxLIST_AUTOSIZE);
	list_->SetColumnWidth(1, wxLIST_AUTOSIZE);

	sizer->Add(label, wxSizerFlags(0).Expand());
	sizer->Add(list_, wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT | wxDOWN, borderSize * scalingFactor));

	panel->SetSizer(sizer);

	// resize the frame to try to show its full contents
	SetClientSize(
	    std::min(list_->GetColumnWidth(0) + list_->GetColumnWidth(1) + static_cast<int>(2.0 * scalingFactor * borderSize),
	             static_cast<int>(scalingFactor * 800.0)),
	    scalingFactor * 400.0);
	ShowModal();
}

void ErrorDialog::onActivate(wxListEvent &evt) {
	onActivate();
	evt.Skip();
}

void ErrorDialog::onActivate() {
	// put all items in selection to clipboard
	std::string summary;

	long item = -1;
	while ((item = list_->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) {
		if (!list_->GetItemText(item, 0).empty())
			summary += "\n" + list_->GetItemText(item, 0) + ":\n";
		summary += "\t" + list_->GetItemText(item, 1) + "\n";
	}
	summary.pop_back(); // remove trailing

	wxTheClipboard->Open();
	wxTheClipboard->SetData(new wxTextDataObject(summary));
	wxTheClipboard->Close();
}

void ErrorDialog::onContext(wxListEvent &evt) {
	wxMenu *menu = new wxMenu;
	menu->Append(0, "Copy");
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [&](wxCommandEvent &evt) {
		if (evt.GetId() == 0) {
			onActivate();
		}
	});

	PopupMenu(menu);
}