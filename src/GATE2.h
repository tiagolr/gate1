#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "ISender.h"
#include "IControls.h"
#include "Pattern.h"

const int kNumPresets = 1;

enum EParams
{
  kPattern = 0,
  kSync,
  kRate,
  kMin,
  kMax,
  kSmooth,
  kAttSmooth,
  kRelSmooth,
  kTensionMult,
  kRetrigger,
  kSnap,
  kGrid,
  kPaintMode,
  kPointMode,
  kNumParams
};

enum EControlTags
{
  kCtrlTagScope = 0,
  kNumCtrlTags
};

class View;

using namespace iplug;
using namespace igraphics;

class GATE2 final : public Plugin
{
public:
  bool snap = false;
  int gridSegs = 8;
  bool linkEdgePoints = false;

  static const IColor COLOR_BG;
  static const IColor COLOR_ACTIVE;

  Pattern* pattern;
  Pattern* patterns[12];
  View* view;

  GATE2(const InstanceInfo& info);

  void OnParentWindowResize(int width, int height) override;
  bool OnHostRequestingSupportedViewConfiguration(int width, int height) override;
  void OnHostSelectedViewConfiguration(int width, int height) override;
  
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void OnIdle() override;
  void OnParamChange(int paramIdx) override;

  void layoutControls(IGraphics* g);

private:
  FastSinOscillator<sample> mOsc {440.f};
  IBufferSender<> mScopeSender;
};
