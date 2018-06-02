#pragma once
#include "errordialog.h"
#include "generator.h"
#include <wx/listctrl.h>

ErrorDialog::ErrorDialog(wxFrame *parent, double borderSize, double scalingFactor,
                         std::vector<std::pair<std::string, std::vector<std::string>>> errors)
    : wxDialog(parent, wxID_ANY, "Errors") {
	wxPanel *   panel = new wxPanel(this);
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *label = Generator::createLabel(panel, borderSize, scalingFactor, "Errors");
	wxListCtrl *list  = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                      wxLC_REPORT & ~wxLC_HRULES | wxLC_VRULES | wxLC_NO_HEADER);

	list->AppendColumn("Path");
	list->AppendColumn("Errors");

	long index = 0;
	for (const auto &i : errors) {
		const long indexMajor = list->InsertItem(index++, i.first);

		list->SetItem(indexMajor, 1, i.second[0]);
		if (i.second.size() > 1)
			for (auto it = i.second.begin() + 1; it != i.second.end(); ++it) {
				const long indexMinor = list->InsertItem(index++, "");
				list->SetItem(indexMinor, 1, *it);
			}
	}

	list->SetColumnWidth(0, wxLIST_AUTOSIZE);
	list->SetColumnWidth(1, wxLIST_AUTOSIZE);

	sizer->Add(label, wxSizerFlags(0).Expand());
	sizer->Add(list, wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT | wxDOWN, borderSize * scalingFactor));

	panel->SetSizer(sizer);

	// resize the frame to try to show its full contents
	SetClientSize(
	    std::min(list->GetColumnWidth(0) + list->GetColumnWidth(1) + static_cast<int>(2.0 * scalingFactor * borderSize),
	             static_cast<int>(scalingFactor * 800.0)),
	    scalingFactor * 400.0);
	ShowModal();
}