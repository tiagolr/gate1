#pragma once
#include "GATE1.h"
#include "IControls.h"

class Caption : public ICaptionControl
{
public:
  Caption(const IRECT& bounds, int paramIdx, const IText& text, const IColor& bgColor, bool showParamLabel = false)
    : ICaptionControl(bounds, paramIdx, text, bgColor, showParamLabel) {};

  void Draw(IGraphics& g) override;
};

class Button : public IVToggleControl
{
public:
  Button(const IRECT& bounds, int paramIdx, const char* label, const IVStyle& style, const char* offText, const char* onText)
    : IVToggleControl(bounds, paramIdx, label, style, offText, onText) {};

  Button(const IRECT& bounds, IActionFunction aF, const char* label, const IVStyle& style, const char* offText, const char* onText)
    : IVToggleControl(bounds, aF, label, style, offText, onText) {};

  void DrawWidget(IGraphics& g) override;
  void DrawValue(IGraphics& g, bool mouseOver) override;
};

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

class PatternSwitches : public IVTabSwitchControl
{
public:
  PatternSwitches(const IRECT& bounds, int paramIdx, const std::vector<const char*>& options, const char* label, const IVStyle& style, EVShape shape, EDirection direction)
    : IVTabSwitchControl(bounds, paramIdx, options, label, style, shape, direction) {};

  void DrawButton(IGraphics& g, const IRECT& r, bool pressed, bool mouseOver, ETabSegment segment, bool disabled) override;
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

  void Draw(IGraphics& g) override;
  void showPopupMenu();
  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override;

private:
  GATE1& gate;
};