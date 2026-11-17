#pragma once
#include "patches/patch.h"

class PatchBattleControlWindow : public Patch
{
  public:
    virtual void Hook(int stage);
};
