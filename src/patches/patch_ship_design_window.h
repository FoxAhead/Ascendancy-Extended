#pragma once
#include "patches/patch.h"

class PatchShipDesignWindow : public Patch
{
  public:
    virtual void Hook(int stage);
};
