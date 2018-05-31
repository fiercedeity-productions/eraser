#pragma once
#include <wx/statline.h>
#include <wx/wx.h>

class Generator {
  public:
	inline static wxBoxSizer *createLabel(wxWindow *panel, double borderSize, double scalingFactor, std::string text) {
		wxBoxSizer *label = new wxBoxSizer(wxHORIZONTAL);
		label->Add(new wxStaticLine(panel, wxID_ANY, wxDefaultPosition, wxSize(1, 1)),
		           wxSizerFlags(1).Border(wxRIGHT, borderSize * scalingFactor).CenterVertical());
		label->Add(new wxStaticText(panel, wxID_ANY, wxString::FromUTF8(text)));
		label->Add(new wxStaticLine(panel, wxID_ANY, wxDefaultPosition, wxSize(1, 1)),
		           wxSizerFlags(1).Border(wxLEFT, borderSize * scalingFactor).CenterVertical());

		return label;
	}
};