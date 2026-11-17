#pragma once
#include "patches/patch.h"

class PatchCosmosWindow : public Patch
{
  public:
    virtual void Hook(int stage);
};
