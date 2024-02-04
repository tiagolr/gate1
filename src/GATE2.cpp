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

  // init patterns
  for (int i = 0; i < 12; i++) {
    patterns[i] = new Pattern(*this, i);
    patterns[i]->insertPoint(0, 1, 0, 1);
    patterns[i]->insertPoint(0.5, 0, 0, 1);
    patterns[i]->insertPoint(1, 1, 0, 1);
    patterns[i]->buildSegments();
  }
  pattern = patterns[0];

  // makeGraphicsFunc
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS);
  };


  // layoutFunc
  mLayoutFunc = [&](IGraphics* pGraphics) {
    const IRECT b = pGraphics->GetBounds();

    // Layout controls on resize
    if(pGraphics->NControls()) {
      layoutControls(pGraphics);
      return;
    }

    pGraphics->SetLayoutOnResize(true);
    //pGraphics->AttachCornerResizer(EUIResizerMode::Size, true);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    //pGraphics->AttachPopupMenuControl();

    //Create controls
    IRECT rect;
    view = new View(rect, *this);
    pGraphics->AttachPanelBackground(COLOR_BG);
    pGraphics->AttachControl(view, 1);

    layoutControls(pGraphics);
  };
}

void GATE2::layoutControls(IGraphics* g)
{
  const IRECT b = g->GetBounds();
  g->GetBackgroundControl()->SetTargetAndDrawRECTs(b);
  g->GetControlWithTag(1)->SetTargetAndDrawRECTs(b.GetReducedFromTop(150));
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
