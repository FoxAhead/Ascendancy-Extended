#pragma once
#include "patches/patch.h"

class PatchMainBtns : public Patch
{
  public:
    virtual void Hook(int stage);
};
