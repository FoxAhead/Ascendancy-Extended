#pragma once
#include "patches/patch.h"

class PatchBattleModeWindow : public Patch
{
  public:
    virtual void Hook(int stage);
};
