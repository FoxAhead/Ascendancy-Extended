#pragma once
#include "patches/patch.h"

class PatchHotkeys : public Patch
{
  public:
    virtual void Hook(int stage);
};
