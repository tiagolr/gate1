#include "GATE2.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "Test/TestSizeControl.h"
#include "View.h"

const IColor GATE2::COLOR_BG = IColor::FromColorCode(0x181614);
const IColor GATE2::COLOR_ACTIVE = IColor::FromColorCode(0xFF8050);
const IColor GATE2::COLOR_ACTIVE_LIGHT = IColor::FromColorCode(0xffb193);
const IColor GATE2::COLOR_ACTIVE_DARK = IColor::FromColorCode(0x88442b);

GATE2::GATE2(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  // init params
  GetParam(kPattern)->InitInt("Pattern", 1, 1, 12);
  GetParam(kSync)->InitEnum("Sync", 5, 18, "", 0, "", "Rate Hz", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "4/1", "1/16t", "1/8t", "1/4t", "1/2t", "1/1t", "1/16.", "1/8.", "1/4.", "1/2.", "1/1.");
  GetParam(kRate)->InitFrequency("Rate", 1., 0.01, 140);
  GetParam(kMin)->InitDouble("Min", 0, 0, 100, 1);
  GetParam(kMax)->InitDouble("Max", 100, 0, 100, 1);
  GetParam(kSmooth)->InitDouble("Smooth", 0, 0, 100, 1);
  GetParam(kAttack)->InitDouble("Attack", 0, 0, 100, 1);
  GetParam(kRelease)->InitDouble("Release", 0, 0, 100, 1);
  GetParam(kTension)->InitDouble("Tension", 0, -100, 100, 1);
  GetParam(kPaintMode)->InitEnum("Paint", 1, 5, "", 0, "", "Erase", "Line", "Saw up", "Saw Down", "Triangle");
  GetParam(kPointMode)->InitEnum("Point", 1, 8, "", 0, "", "Hold", "Curve", "S-Curve", "Pulse", "Wave", "Triangle", "Stairs", "Smooth St");
  GetParam(kSnap)->InitBool("Snap", 0);
  GetParam(kGrid)->InitInt("Grid", 8, 2, 32);
  GetParam(kRetrigger)->InitBool("Retrigger", 0);

  // Init style

  // init patterns
  for (int i = 0; i < 12; i++) {
    patterns[i] = new Pattern(*this, i);
    patterns[i]->insertPoint(0, 1, 0, 1);
    patterns[i]->insertPoint(0.5, 0, 0, 1);
    patterns[i]->insertPoint(1, 1, 0, 1);
    patterns[i]->buildSegments();
  }
  pattern = patterns[0];

  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS);
  };

  mLayoutFunc = [&](IGraphics* g) {
    // Layout controls on resize
    if(g->NControls()) {
      layoutControls(g);
      return;
    }

    g->EnableMouseOver(true);
    g->SetLayoutOnResize(true);
    g->LoadFont("Roboto-Regular", ROBOTO_FN);
    g->LoadFont("Roboto-Bold", ROBOTO_BOLD_FN);

    //Create styles
    IVColorSpec switchColors = {
      COLOR_BG,
      COLOR_ACTIVE_DARK,
      COLOR_ACTIVE,
      COLOR_BG,
      COLOR_ACTIVE_LIGHT,
      COLOR_WHITE,
    };
    IVStyle patternSwitchStyle = (new IVStyle())->WithColors(switchColors);
    patternSwitchStyle.roundness = 0.5;
    patternSwitchStyle.shadowOffset = 0;
    patternSwitchStyle.valueText.mFGColor = COLOR_BG;
    patternSwitchStyle.valueText = patternSwitchStyle.valueText.WithFont("Roboto-Bold");

    IVColorSpec rotaryColors = {
      COLOR_BG,
      COLOR_BG,
      COLOR_BG,
      COLOR_ACTIVE,
      COLOR_BG,
      COLOR_BG,
      COLOR_ACTIVE,
    };
    IVStyle rotaryStyle = (new IVStyle())->WithColors(rotaryColors);
    rotaryStyle.frameThickness = 2;
    rotaryStyle.shadowOffset = 0;
    rotaryStyle.valueText.mFGColor = COLOR_WHITE;
    rotaryStyle.valueText = rotaryStyle.valueText.WithFont("Roboto-Bold");
    rotaryStyle.labelText.mFGColor = COLOR_WHITE;
    rotaryStyle.labelText.mSize = 16;
    rotaryStyle.labelText = rotaryStyle.labelText.WithFont("Roboto-Bold");

    IVColorSpec buttonColors = {
      COLOR_BG,
      COLOR_ACTIVE_DARK,
      COLOR_ACTIVE,
      COLOR_BG,
      COLOR_TRANSPARENT,
      COLOR_BG,
    };
    IVStyle buttonStyle = (new IVStyle())->WithColors(buttonColors);
    buttonStyle.roundness = 0.5;
    buttonStyle.showLabel = false;
    buttonStyle.valueText.mFGColor = COLOR_BG;
    buttonStyle.valueText.mSize = 16;
    buttonStyle.valueText = buttonStyle.valueText.WithFont("Roboto-Bold");

    IVColorSpec numberColors = {
      COLOR_BG,
      COLOR_BG,
      COLOR_BG,
      COLOR_BG,
      COLOR_TRANSPARENT,
    };
    IVStyle numberStyle = (new IVStyle())->WithColors(numberColors);
    numberStyle.roundness = 0.5;
    numberStyle.valueText.mFGColor = COLOR_ACTIVE;
    numberStyle.valueText.mSize = 16;
    numberStyle.valueText = numberStyle.valueText.WithFont("Roboto-Bold");
    numberStyle.labelText.mFGColor = COLOR_ACTIVE;
    numberStyle.labelText.mSize = 16;
    numberStyle.labelText = numberStyle.labelText.WithFont("Roboto-Bold");

    //Create controls
    IRECT rect;
    view = new View(rect, *this);
    g->AttachPanelBackground(COLOR_BG);
    g->AttachControl(view);
    auto t = IText(26, COLOR_WHITE, "Roboto-Bold", EAlign::Near);
    g->AttachControl(new ITextControl(IRECT(10,14,100,35), "GATE-1", t, true));
    patternSwitches = new IVTabSwitchControl(IRECT(), kPattern, {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"}, "", patternSwitchStyle, EVShape::EndsRounded);
    g->AttachControl(patternSwitches);
    syncControl = new ICaptionControl(IRECT(), kSync, IText(16.f, COLOR_BG, "Roboto-Bold"), COLOR_ACTIVE);
    g->AttachControl(syncControl);
    rateControl = new IVKnobControl(IRECT(), kRate, "Rate", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
    g->AttachControl(rateControl);
    minControl = new IVKnobControl(IRECT(), kMin, "Min", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
    g->AttachControl(minControl);
    maxControl = new IVKnobControl(IRECT(), kMax, "Max", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
    g->AttachControl(maxControl);
    smoothControl = new IVKnobControl(IRECT(), kSmooth, "Smooth", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
    g->AttachControl(smoothControl);
    attackControl = new IVKnobControl(IRECT(), kAttack, "Attack", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
    g->AttachControl(attackControl);
    releaseControl = new IVKnobControl(IRECT(), kRelease, "Release", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
    g->AttachControl(releaseControl);
    tensionControl = new IVKnobControl(IRECT(), kTension, "Tension", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
    g->AttachControl(tensionControl);
    pointModeControl = new ICaptionControl(IRECT(), kPointMode, IText(16.f, COLOR_BG, "Roboto-Bold"), COLOR_ACTIVE);
    g->AttachControl(pointModeControl);
    paintModeControl = new ICaptionControl(IRECT(), kPaintMode, IText(16.f, COLOR_BG, "Roboto-Bold"), COLOR_ACTIVE);
    g->AttachControl(paintModeControl);
    snapControl = new IVToggleControl(IRECT(), kSnap, " ", buttonStyle, "Snap", "Snap");
    g->AttachControl(snapControl);
    t = IText(16, COLOR_WHITE, "Roboto-Bold", EAlign::Near);
    gridNumber = new IVNumberBoxControl(IRECT(), kGrid, nullptr, "", numberStyle, false, 8, 2, 32, "Grid %0.f", false);
    g->AttachControl(gridNumber);
    optionsControl = new OptionsControl(IRECT(), [&](IControl* pCaller) {
      optionsControl->showPopupMenu();
    }, ". . .", numberStyle, *this);
    g->AttachControl(optionsControl);

    inited = true;
    layoutControls(g);
  };
}

void GATE2::layoutControls(IGraphics* g)
{
  if (!inited)
    return;

  const IRECT b = g->GetBounds();
  g->GetBackgroundControl()->SetTargetAndDrawRECTs(b);
  view->SetTargetAndDrawRECTs(b.GetReducedFromTop(165));

  // first row left
  int drawx = 100;
  int drawy = 10;
  syncControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy + 3, drawx+80, drawy+20+3));
  drawx += 90;
  patternSwitches->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx + 250, drawy + 25));

  // first row right
  drawx = b.R - 50;
  optionsControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy+10, drawx+40, drawy+25+10));

  // second row left
  drawy = 50;
  drawx = 0;
  rateControl->Hide(GetParam(kSync)->Value() > 0);
  if (!rateControl->IsHidden()) {
    rateControl->SetTargetAndDrawRECTs(IRECT(drawx,drawy,drawx+80,drawy+80));
    drawx += 70;
  }
  minControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+80));
  drawx += 70;
  maxControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+80));
  drawx += 70;
  smoothControl->Hide(dualSmooth);
  if (!dualSmooth) {
    smoothControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+80));
    drawx += 70;
  }
  attackControl->Hide(!dualSmooth);
  releaseControl->Hide(!dualSmooth);
  if (dualSmooth) {
    attackControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+80));
    drawx += 70;
    releaseControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+80));
    drawx += 70;
  }
  tensionControl ->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+80));

  // third row left
  drawx = 10;
  drawy = 140;
  paintModeControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+20));
  drawx += 90;
  pointModeControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+20));

  // third row right
  drawx = b.R - 73;
  snapControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+70, drawy+30));
  drawx -= 80;
  gridNumber->SetTargetAndDrawRECTs(IRECT(drawx+30, drawy+3, drawx+40+40, drawy+20+3));
}

void GATE2::OnParamChange(int paramIdx)
{
  if (paramIdx == kPattern) {
    pattern = patterns[(int)GetParam(kPattern)->Value() - 1];
  }
  else if (paramIdx == kSync) {
    layoutControls(GetUI());
  }
  else if (paramIdx == kGrid) {
    gridSegs = (int)GetParam(kGrid)->Value();
  }
  else if (paramIdx == kTension) {
    tensionMult = GetParam(kTension)->Value();
  }
}

void GATE2::OnParentWindowResize(int width, int height)
{
  if (GetUI())
    GetUI()->Resize(width, height, 1.f, false);
}

bool GATE2::OnHostRequestingSupportedViewConfiguration(int width, int height)
{
  return ConstrainEditorResize(width, height);
}

void GATE2::OnHostSelectedViewConfiguration(int width, int height)
{
  if (GetUI())
    GetUI()->Resize(width, height, 1.f, true);
}

void GATE2::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  //const double gain = GetParam(kGain)->DBToAmp();

  //for (int s = 0; s < nFrames; s++) {
  //  outputs[0][s] = mOsc.Process() * gain;
  //  outputs[1][s] = outputs[0][s];
  //}

  //mScopeSender.ProcessBlock(outputs, nFrames, kCtrlTagScope, 1);
}

void GATE2::ProcessMidiMsg(const IMidiMsg& msg)
{

  int status = msg.StatusMsg();

  auto midi2CPS = [](int pitch) {
    return 440. * pow(2., (pitch - 69.) / 12.);
  };

  if (status == IMidiMsg::kNoteOn) {
    mOsc.SetFreqCPS(midi2CPS(msg.NoteNumber()));     
  }

  SendMidiMsg(msg); // passthrough
}

void GATE2::OnIdle()
{
  mScopeSender.TransmitData(*this);
}
