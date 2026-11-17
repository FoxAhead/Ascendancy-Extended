#pragma once
#include "patches/patch.h"

class PatchPlanetSquareWindow : public Patch
{
  public:
    virtual void Hook(int stage);
};
