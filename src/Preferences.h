#pragma once
#include "GATE2.h"
#include "IControls.h"

class Preferences : public IVButtonControl
{
public:
  Preferences(const IRECT& bounds, IActionFunction aF, const char* label, const IVStyle& style, GATE2& g)
    : IVButtonControl(bounds, aF, label, style), gate(g) {};

  void showPopupMenu();
  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override;

private:
  GATE2& gate;
};