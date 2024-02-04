#include "GATE2.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "Test/TestSizeControl.h"
#include "View.h"

const IColor GATE2::COLOR_BG = IColor::FromColorCode(0x141618);
const IColor GATE2::COLOR_ACTIVE = IColor::FromColorCode(0xFF8050);

GATE2::GATE2(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  // init params
  GetParam(kPattern)->InitInt("Pattern", 1, 1, 12);
  GetParam(kSync)->InitInt("Sync", 5, 0, 17);
  GetParam(kRate)->InitFrequency("Rate", 1., 0.01, 140);
  GetParam(kMin)->InitDouble("Min", 0, 0, 100, 1);
  GetParam(kMax)->InitDouble("Max", 100, 0, 100, 1);
  GetParam(kSmooth)->InitDouble("Smooth", 0, 0, 100, 1);
  GetParam(kAttSmooth)->InitDouble("Attack", 0, 0, 100, 1);
  GetParam(kRelSmooth)->InitDouble("Release", 0, 0, 100, 1);
  GetParam(kTensionMult)->InitDouble("Tension", 0, -100, 100, 1);
  GetParam(kPaintMode)->InitEnum("Paint", 1, 5, "", 0, "", "Erase", "Line", "Saw up", "Saw Down", "Triangle");
  GetParam(kPointMode)->InitEnum("Point", 1, 8, "", 0, "", "Hold", "Curve", "S-Curve", "Pulse", "Wave", "Triangle", "Stairs", "Smooth Stairs");

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
    g->EnableMouseOver(true);
    const IRECT b = g->GetBounds();

    // Layout controls on resize
    if(g->NControls()) {
      layoutControls(g);
      return;
    }

    g->SetLayoutOnResize(true);
    g->LoadFont("Roboto-Regular", ROBOTO_FN);

    //Create controls
    IRECT rect;
    view = new View(rect, *this);
    g->AttachPanelBackground(COLOR_BG);
    g->AttachControl(view);
    IVStyle patternSwitchStyle = new IVStyle();
    patternSwitchStyle.roundness = 0.5;
    patternSwitchStyle.shadowOffset = 0;
    g->AttachControl(new IVTabSwitchControl(IRECT(10, 10, 10 + 250, 10 + 25), kPattern, {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"}, "", patternSwitchStyle));

    layoutControls(g);
  };
}

void GATE2::OnParamChange(int paramIdx)
{
  if (paramIdx == kPattern) {
    pattern = patterns[(int)GetParam(kPattern)->Value()];
  }
}

void GATE2::layoutControls(IGraphics* g)
{
  const IRECT b = g->GetBounds();
  g->GetBackgroundControl()->SetTargetAndDrawRECTs(b);
  view->SetTargetAndDrawRECTs(b.GetReducedFromTop(150));
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
