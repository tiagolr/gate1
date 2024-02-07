#pragma once
#include "GATE1.h"
#include "IControls.h"

class Rotary : public IVKnobControl
{
public:
  Rotary(const IRECT& bounds, int paramIdx,
    const char* label = "",
    const IVStyle& style = DEFAULT_STYLE,
    bool valueIsEditable = false, bool valueInWidget = false,
    float a1 = -135.f, float a2 = 135.f, float aAnchor = -135.f,
    EDirection direction = EDirection::Vertical, double gearing = DEFAULT_GEARING, float trackSize = 2.f)
    : IVKnobControl(bounds, paramIdx, label, style, valueIsEditable, valueInWidget, a1, a2, aAnchor, direction, gearing, trackSize) {};

  void DrawWidget(IGraphics& g) override;
  void DrawIndicatorTrack(IGraphics& g, float angle, float cx, float cy, float radius) override;
};

class PlayButton : public IControl
{
public:
  PlayButton(const IRECT& bounds, IActionFunction af) : IControl(bounds, af) {};
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  void Draw(IGraphics& g) override;
};

class Preferences : public IVButtonControl
{
public:
  Preferences(const IRECT& bounds, IActionFunction aF, const char* label, const IVStyle& style, GATE1& g)
    : IVButtonControl(bounds, aF, label, style), gate(g) {};

  void showPopupMenu();
  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override;

private:
  GATE1& gate;
};