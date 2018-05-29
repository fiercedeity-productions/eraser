#pragma once

#include "frame.h"
#include <wx/wx.h>

class App : public wxApp {
  private:
	Frame *frame_;

  protected:
	virtual bool OnInit();
};