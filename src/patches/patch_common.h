#pragma once
#include "patches/patch.h"

class PatchCommon : public Patch
{
  public:
    virtual void Hook(int stage);
};
