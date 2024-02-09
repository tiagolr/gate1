#include "Widgets.h"
#include <string>

void Caption::Draw(IGraphics& g)
{
  const IParam* pParam = GetParam();

  if(pParam)
  {
    pParam->GetDisplay(mStr);

    if (mShowParamLabel)
    {
      mStr.Append(" ");
      mStr.Append(pParam->GetLabel());
    }
  }

  g.DrawRoundRect(GATE1::COLOR_ACTIVE, mRECT, 2.5);

  if (mStr.GetLength() && g.GetControlInTextEntry() != this)
    g.DrawText(mText, mStr.Get(), mRECT, &mBlend);

  if(mTri.W() > 0.f)
  {
    g.FillTriangle(GATE1::COLOR_ACTIVE, mTri.L, mTri.T, mTri.R, mTri.T, mTri.MW(), mTri.B, GetMouseIsOver() ? 0 : &BLEND_50);
  }
}

void Button::DrawWidget(IGraphics& g)
{
  if (GetValue() > 0.5)
    g.FillRoundRect(GATE1::COLOR_ACTIVE, mWidgetBounds, 2.5);
  else
    g.DrawRoundRect(GATE1::COLOR_ACTIVE, mWidgetBounds, 2.5);
}

void Button::DrawValue(IGraphics& g, bool mouseOver)
{
  if(GetValue() > 0.5) {
    mStyle.valueText.mFGColor = GATE1::COLOR_BG;
    g.DrawText(mStyle.valueText, mOnText.Get(), mValueBounds, &mBlend);
  }
  else {
    mStyle.valueText.mFGColor = GATE1::COLOR_ACTIVE;
    g.DrawText(mStyle.valueText, mOffText.Get(), mValueBounds, &mBlend);
  }
}

void Rotary::DrawWidget(IGraphics& g)
{
  float widgetRadius = GetRadius() - 2;
  const float cx = mWidgetBounds.MW(), cy = mWidgetBounds.MH() + 2;
  IRECT knobHandleBounds = mWidgetBounds.GetCentredInside((widgetRadius - mTrackToHandleDistance - 2) * 2.f ).GetVShifted(2);
  const float angle = mAngle1 + (static_cast<float>(GetValue()) * (mAngle2 - mAngle1));
  DrawIndicatorTrack(g, angle, cx, cy, widgetRadius);
  g.FillEllipse(IColor::FromColorCode(0x282D32), knobHandleBounds);
  DrawPointer(g, angle, cx, cy, knobHandleBounds.W() / 2.f);
}

void Rotary::DrawIndicatorTrack(IGraphics& g, float angle, float cx, float cy, float radius)
{
  g.DrawArc(IColor::FromColorCode(0x282D32), cx, cy, radius, -135, 135, &mBlend, mTrackSize);
  if (mTrackSize > 0.f)
  {
    g.DrawArc(GetColor(kX1), cx, cy, radius, angle >= mAnchorAngle ? mAnchorAngle : mAnchorAngle - (mAnchorAngle - angle), angle >= mAnchorAngle ? angle : mAnchorAngle, &mBlend, mTrackSize);
  }
}

void PatternSwitches::DrawButton(IGraphics& g, const IRECT& r, bool pressed, bool mouseOver, ETabSegment segment, bool disabled)
{
  DrawPressableRectangle(g, r, pressed, mouseOver, disabled, segment == ETabSegment::Start, segment == ETabSegment::End, segment == ETabSegment::Start, segment == ETabSegment::End);
}

void PlayButton::Draw(IGraphics& g)
{
  IRECT r = mRECT.GetPadded(-2);
  if (GetValue() > 0.5) {
    g.FillRect(GATE1::COLOR_ACTIVE, r);
  }
  else {
    g.FillTriangle(GATE1::COLOR_ACTIVE, r.L, r.T, r.R, (r.T + r.B) / 2, r.L, r.B);
  }
}

void PlayButton::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  SetDirty(true);
}

void Preferences::Draw(IGraphics& g)
{
  g.DrawSVG(gate.settingsSVG, mRECT,0, &COLOR_TRANSPARENT, &GATE1::COLOR_ACTIVE);
}

void Preferences::showPopupMenu()
{
  IPopupMenu* menu = new IPopupMenu();
  IPopupMenu* optionsMenu = new IPopupMenu();
  IPopupMenu* triggerMenu = new IPopupMenu();
  IPopupMenu* loadMenu = new IPopupMenu();


  triggerMenu->AddItem("Off")->SetChecked(gate.triggerChannel == 0);
  for (int i = 0; i < 16; ++i) {
    triggerMenu->AddItem(std::to_string(i+1).c_str())->SetChecked(gate.triggerChannel == i + 1);
  }
  triggerMenu->AddItem("Omni")->SetChecked(gate.triggerChannel == 17);

  optionsMenu->AddItem("Trigger channel")->SetSubmenu(triggerMenu);
  optionsMenu->AddItem("Dual smooth")->SetChecked(gate.dualSmooth);
  optionsMenu->AddItem("Link edge points")->SetChecked(gate.linkEdgePoints);

  loadMenu->AddItem("Sine");
  loadMenu->AddItem("Triangle");
  loadMenu->AddItem("Random");

  menu->AddItem("Options")->SetSubmenu(optionsMenu);
  menu->AddItem("Draw wave")->SetChecked(gate.drawWave);
  menu->AddSeparator();
  menu->AddItem("Invert");
  menu->AddItem("Reverse");
  menu->AddItem("Clear");
  menu->AddItem("Copy");
  menu->AddItem("Paste");
  menu->AddSeparator();
  menu->AddItem("Load")->SetSubmenu(loadMenu);
  menu->AddItem("About");
  
  GetUI()->CreatePopupMenu(*this, *menu, GetWidgetBounds());
}

void Preferences::OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) {
  if (pSelectedMenu == nullptr)
    return;

  auto text = pSelectedMenu->GetChosenItem()->GetText();

  for (int i = 1; i < 17; ++i) {
    if (strcmp(text, std::to_string(i).c_str()) == 0) {
      gate.triggerChannel = i;
    }
  }
  if (strcmp(text, "Omni") == 0) {
    gate.triggerChannel = 17;
  }
  else if (strcmp(text, "Off") == 0) {
    gate.triggerChannel = 0;
  }
  else if (strcmp(text, "Dual smooth") == 0) {
    gate.dualSmooth = !gate.dualSmooth;
    gate.dirtyControls = true;
    gate.setSmooth();
  }
  else if (strcmp(text, "Link edge points") == 0) {
    gate.linkEdgePoints = !gate.linkEdgePoints;
  }
  else if (strcmp(text, "Draw wave") == 0) {
    gate.drawWave = !gate.drawWave;
  }
  else if (strcmp(text, "Sine") == 0) {
    gate.pattern->loadSine();
    gate.pattern->buildSegments();
  }
  else if (strcmp(text, "Triangle") == 0) {
    gate.pattern->loadTriangle();
    gate.pattern->buildSegments();
  }
  else if (strcmp(text, "Random") == 0) {
    gate.pattern->loadRandom(gate.gridSegs);
    gate.pattern->buildSegments();
  }
  else if (strcmp(text, "Copy") == 0) {
    gate.pattern->copy();
  }
  else if (strcmp(text, "Paste") == 0) {
    gate.pattern->paste();
    gate.pattern->buildSegments();
  }
  else if (strcmp(text, "Clear") == 0) {
    gate.pattern->clear();
    gate.pattern->buildSegments();
  }
  else if (strcmp(text, "Invert") == 0) {
    gate.pattern->invert();
    gate.pattern->buildSegments();
  }
  else if (strcmp(text, "Reverse") == 0) {
    gate.pattern->reverse();
    gate.pattern->buildSegments();
  }
}