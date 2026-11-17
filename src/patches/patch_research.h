#pragma once
#include "patches/patch.h"

class PatchResearch : public Patch
{
  public:
    virtual void Hook(int stage);
};
