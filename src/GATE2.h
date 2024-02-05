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
  kAttack,
  kRelease,
  kTension,
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
  bool inited = false;
  bool snap = false;
  int gridSegs = 8;
  bool linkEdgePoints = false;
  bool dualSmooth = true;
  double tensionMult = 0;

  static const IColor COLOR_BG;
  static const IColor COLOR_ACTIVE;
  static const IColor COLOR_ACTIVE_DARK;
  static const IColor COLOR_ACTIVE_LIGHT;

  Pattern* pattern;
  Pattern* patterns[12];
  View* view;
  IVTabSwitchControl* patternSwitches;
  ICaptionControl* syncControl;
  IVKnobControl* rateControl;
  IVKnobControl* minControl;
  IVKnobControl* maxControl;
  IVKnobControl* smoothControl;
  IVKnobControl* attackControl;
  IVKnobControl* releaseControl;
  IVKnobControl* tensionControl;
  ICaptionControl* pointModeControl;
  ICaptionControl* paintModeControl;
  IVToggleControl* snapControl;
  IVNumberBoxControl* gridNumber;

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