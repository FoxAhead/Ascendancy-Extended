#pragma once
#include "patches/patch.h"

class PatchFleetWindow : public Patch
{
  public:
    virtual void Hook(int stage);
};
