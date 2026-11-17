#pragma once
#include "patches/patch.h"

class PatchScrollBar : public Patch
{
  public:
    virtual void Hook(int stage);
};
