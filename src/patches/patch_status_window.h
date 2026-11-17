#pragma once
#include "patches/patch.h"

class PatchStatusWindow : public Patch
{
  public:
    virtual void Hook(int stage);
};
