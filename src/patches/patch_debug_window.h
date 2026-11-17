#pragma once
#include "patches/patch.h"

class PatchDebugWindow : public Patch
{
  public:
    virtual void Hook(int stage);
};
