#pragma once
#include "patches/patch.h"

class PatchNextTurn : public Patch
{
  public:
    virtual void Hook(int stage);
};
