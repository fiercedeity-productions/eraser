#pragma once
#include "frame.h"

class Task {
  private:
	Frame *frameInstance_;

  public:
	Task(Frame *frameInstance);
};