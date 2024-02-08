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
  kPhase,
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
class Preferences;
class PlayButton;
class Rotary;
class PatternSwitches;

using namespace iplug;
using namespace igraphics;

/*
Dual resistance a or b RC lowpass filter
*/
class SmoothParam
{
public:
  double a; // resistance a
  double b; // resistance b
  double lp; // last value
  double smooth; // latest value

  void rcSet2(double rca, double rcb, double srate)
  {
    a = 1 / (rca * srate + 1);
    b = 1 / (rcb * srate + 1);
  }

  double rcLP2(double s, bool ab)
  {
    return lp += ab ? a * (s - lp) : b * (s - lp);
  }

  double smooth2(double s, bool ab)
  {
    lp = smooth;
    return smooth = rcLP2(s, ab);
  }
};

class GATE1 final : public Plugin
{
public:
  // prefs
  bool linkEdgePoints = false;
  bool dualSmooth = true;
  int triggerChannel = 10;
  bool drawWave = true;
  bool alwaysPlaying = false;
  bool midiMode = false;
  // state
  bool inited = false;
  bool isPlaying = false;
  bool snap = false;
  bool midiTrigger = false; // trigger via midi or slider is active
  int gridSegs = 8;
  double tensionMult = 0;
  double syncQN = 0;
  double xpos = 0;
  double ypos = 0;
  double beatPos = 0;
  int winpos = 0;
  int lwinpos = 0;
  SmoothParam* value;
  std::vector<sample> preSamples;
  std::vector<sample> postSamples;

  static const IColor COLOR_BG;
  static const IColor COLOR_ACTIVE;
  static const IColor COLOR_ACTIVE_DARK;
  static const IColor COLOR_ACTIVE_LIGHT;
  static const IColor COLOR_SEEK;

  Pattern* pattern;
  Pattern* patterns[12];
  View* view;
  PatternSwitches* patternSwitches;
  ICaptionControl* syncControl;
  Preferences* preferencesControl;
  Rotary* rateControl;
  Rotary* phaseControl;
  Rotary* minControl;
  Rotary* maxControl;
  Rotary* smoothControl;
  Rotary* attackControl;
  Rotary* releaseControl;
  Rotary* tensionControl;
  ICaptionControl* pointModeControl;
  ICaptionControl* paintModeControl;
  PlayButton* playControl;
  IVToggleControl* snapControl;
  IVNumberBoxControl* gridNumber;
  IVToggleControl* midiModeControl;

  IVStyle rotaryStyle;
  IVStyle patternSwitchStyle;
  IVStyle buttonStyle;
  IVStyle numberStyle;

  ISVG settingsSVG;

  GATE1(const InstanceInfo& info);

  void OnParentWindowResize(int width, int height) override;
  bool OnHostRequestingSupportedViewConfiguration(int width, int height) override;
  void OnHostSelectedViewConfiguration(int width, int height) override;
  void setSmooth();
  void layoutControls(IGraphics* g);
  void makeStyles();
  void makeControls(IGraphics* g);

  double getY(double x, double min, double max);
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void ProcessMidiMsg(const IMidiMsg& msg) override;
  void OnIdle() override;
  void OnReset() override;
  void OnParamChange(int paramIdx) override;
  bool SerializeState(IByteChunk &chunk) const override;
  int UnserializeState(const IByteChunk &chunk, int startPos) override;
  void OnRestoreState() override;
};

