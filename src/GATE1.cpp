#include "GATE1.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "Test/TestSizeControl.h"
#include "View.h"
#include "Widgets.h"

const IColor GATE1::COLOR_BG = IColor::FromColorCode(0x181614);
const IColor GATE1::COLOR_ACTIVE = IColor::FromColorCode(0xFF8050);
const IColor GATE1::COLOR_ACTIVE_LIGHT = IColor::FromColorCode(0xffb193);
const IColor GATE1::COLOR_ACTIVE_DARK = IColor::FromColorCode(0x88442b);
const IColor GATE1::COLOR_SEEK = IColor::FromColorCode(0x80FFFF);

GATE1::GATE1(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets)),
  settingsSVG(nullptr)
{
  // init params
  GetParam(kPattern)->InitInt("Pattern", 1, 1, 12);
  GetParam(kSync)->InitEnum("Sync", 5, 18, "", 0, "", "Rate Hz", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "4/1", "1/16t", "1/8t", "1/4t", "1/2t", "1/1t", "1/16.", "1/8.", "1/4.", "1/2.", "1/1.");
  GetParam(kRate)->InitFrequency("Rate", 1., 0.01, 140);
  GetParam(kPhase)->InitDouble("Phase", 0, 0, 1, 0.01);
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

  preSamples.resize(PLUG_MAX_WIDTH, 0);
  postSamples.resize(PLUG_MAX_WIDTH, 0);
  value = new SmoothParam();
  makeStyles();

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

    g->EnableTooltips(true);
    g->EnableMouseOver(true);
    g->SetLayoutOnResize(true);
    g->LoadFont("Roboto-Regular", ROBOTO_FN);
    g->LoadFont("Roboto-Bold", ROBOTO_BOLD_FN);
    settingsSVG = g->LoadSVG(SETTINGS_SVG);

    makeControls(g);
    inited = true;
    setSmooth();
    layoutControls(g);
  };
}

void GATE1::setSmooth()
{
  if (dualSmooth) {
    auto const attack = GetParam(kAttack)->Value();
    auto const release = GetParam(kRelease)->Value();
    value->rcSet2(attack * 0.0025, release * 0.0025, GetSampleRate());
  }
  else {
    auto const lfosmooth = GetParam(kSmooth)->Value();
    value->rcSet2(lfosmooth * 0.0025, lfosmooth * 0.0025, GetSampleRate());
  }
}

void GATE1::makeControls(IGraphics* g)
{
  view = new View(IRECT(), *this);
  g->AttachPanelBackground(COLOR_BG);
  g->AttachControl(view);
  auto t = IText(26, COLOR_WHITE, "Roboto-Bold", EAlign::Near);
  g->AttachControl(new ITextControl(IRECT(10,14,100,35), "GATE-1", t, true));
  patternSwitches = new PatternSwitches(IRECT(), kPattern, {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"}, "", patternSwitchStyle, EVShape::EndsRounded, EDirection::Horizontal);
  patternSwitches->SetTooltip("Pattern select");
  g->AttachControl(patternSwitches);
  syncControl = new Caption(IRECT(), kSync, IText(16.f, COLOR_ACTIVE, "Roboto-Bold"), COLOR_ACTIVE_DARK);
  syncControl->SetTooltip("Tempo sync");
  g->AttachControl(syncControl);
  rateControl = new Rotary(IRECT(), kRate, "Rate", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
  g->AttachControl(rateControl);
  phaseControl = new Rotary(IRECT(), kPhase, "Phase", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
  g->AttachControl(phaseControl);
  minControl = new Rotary(IRECT(), kMin, "Min", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
  g->AttachControl(minControl);
  maxControl = new Rotary(IRECT(), kMax, "Max", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
  g->AttachControl(maxControl);
  smoothControl = new Rotary(IRECT(), kSmooth, "Smooth", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
  g->AttachControl(smoothControl);
  attackControl = new Rotary(IRECT(), kAttack, "Attack", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
  g->AttachControl(attackControl);
  releaseControl = new Rotary(IRECT(), kRelease, "Release", rotaryStyle, true, false, -135.f, 135.f, -135.f, EDirection::Vertical, 2.0, 4.0);
  g->AttachControl(releaseControl);
  tensionControl = new Rotary(IRECT(), kTension, "Tension", rotaryStyle, true, false, -135.f, 135.f, 0.f, EDirection::Vertical, 2.0, 4.0);
  g->AttachControl(tensionControl);
  pointModeControl = new Caption(IRECT(), kPointMode, IText(16.f, COLOR_ACTIVE, "Roboto-Bold"), COLOR_ACTIVE_DARK);
  pointModeControl->SetTooltip("Point mode");
  g->AttachControl(pointModeControl);
  paintModeControl = new Caption(IRECT(), kPaintMode, IText(16.f, COLOR_ACTIVE, "Roboto-Bold"), COLOR_ACTIVE_DARK, true);
  paintModeControl->SetTooltip("Paint mode (RMB)");
  g->AttachControl(paintModeControl);
  playControl = new PlayButton(IRECT(), [&](IControl* pCaller){
    alwaysPlaying = !alwaysPlaying;
    playControl->SetValue(alwaysPlaying ? 1 : 0);
    playControl->SetDirty(false);
    });
  playControl->SetValue(alwaysPlaying ? 1 : 0);
  playControl->SetTooltip("Always playing on/off");
  g->AttachControl(playControl);
  midiModeControl = new Button(IRECT(), [&](IControl* pCaller) {
    midiMode = !midiMode;
    midiModeControl->SetValue(midiMode ? 1 : 0);
    midiModeControl->SetDirty(false);
    }, "", buttonStyle, "MIDI", "MIDI");
  midiModeControl->SetValue(midiMode ? 1 : 0);
  midiModeControl->SetTooltip("MIDI mode - use midi notes to trigger envelope");
  g->AttachControl(midiModeControl);
  snapControl = new Button(IRECT(), kSnap, " ", buttonStyle, "Snap", "Snap");
  g->AttachControl(snapControl);
  t = IText(16, COLOR_WHITE, "Roboto-Bold", EAlign::Near);
  gridNumber = new NumberControl(IRECT(), kGrid, nullptr, "", numberStyle, false, 8, 2, 32, "Grid %0.f", false);
  g->AttachControl(gridNumber);
  preferencesControl = new Preferences(IRECT(), [&](IControl* pCaller) {
    preferencesControl->showPopupMenu();
    }, ". . .", numberStyle, *this);
  g->AttachControl(preferencesControl);
  t = IText(16, COLOR_GRAY, "Roboto-Bold", EAlign::Near);
  paintLabel = new ITextControl(IRECT(), "Paint", t);
  pointLabel = new ITextControl(IRECT(), "Point", t);
  g->AttachControl(paintLabel);
  g->AttachControl(pointLabel);
  retriggerControl = new Button(IRECT(), [&](IControl* pCaller) {
    retriggerEnvelope();
    retriggerControl->SetValue(0);
    retriggerControl->SetDirty(false);
  }, "", buttonStyle, "R", "R");
  retriggerControl->SetTooltip("Retrigger envelope");
  g->AttachControl(retriggerControl);
  aboutControl = new About(g->GetBounds(), [](IControl* pCaller) {
    pCaller->Hide(true);
  });
  g->AttachControl(aboutControl);
  aboutControl->Hide(true);
}

void GATE1::layoutControls(IGraphics* g)
{
  if (!inited)
    return;

  const IRECT b = g->GetBounds();
  g->GetBackgroundControl()->SetTargetAndDrawRECTs(b);
  view->SetTargetAndDrawRECTs(b.GetReducedFromTop(158));

  // first row left
  int drawx = 95;
  int drawy = 10;
  syncControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy + 3, drawx+80, drawy+20+3));
  drawx += 90;
  patternSwitches->SetTargetAndDrawRECTs(IRECT(drawx, drawy+3, drawx + 250, drawy + 20+3));

  // first row right
  drawx = b.R - 35;
  preferencesControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+25, drawy+25).GetPadded(-2.5).GetHShifted(2.5).GetVShifted(2.5));

  // second row left
  drawy += 35;
  drawx = 0;
  rateControl->Hide(GetParam(kSync)->Value() > 0);
  if (!rateControl->IsHidden()) {
    rateControl->SetTargetAndDrawRECTs(IRECT(drawx,drawy,drawx+80,drawy+80));
    drawx += 70;
  }
  phaseControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+80));
  drawx += 70;
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
  drawy += 90;
  paintLabel->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+40, drawy+20));
  drawx += 42;
  paintModeControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+20));
  drawx += 90;
  pointLabel->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+40, drawy+20));
  drawx += 42;
  pointModeControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+80, drawy+20));
  drawx += 90;
  playControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx + 20 , drawy+20).GetPadded(-1));
  drawx += 30;
  retriggerControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx + 20, drawy + 20));

  // third row right
  drawx = b.R - 70;
  midiModeControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx + 60, drawy + 20));
  drawx -= 70;
  snapControl->SetTargetAndDrawRECTs(IRECT(drawx, drawy, drawx+60, drawy+20));
  drawx -= 85;
  gridNumber->SetTargetAndDrawRECTs(IRECT(drawx+30, drawy, drawx+40+40, drawy+20));

  aboutControl->SetTargetAndDrawRECTs(b);
}

void GATE1::makeStyles()
{
  //Create styles
  patternSwitchStyle = (new IVStyle())->WithColors({
    COLOR_TRANSPARENT,
    COLOR_ACTIVE_DARK,
    COLOR_ACTIVE,
    COLOR_BG,
    COLOR_TRANSPARENT,
    COLOR_TRANSPARENT,
    });
  patternSwitchStyle.roundness = 0.25;
  patternSwitchStyle.drawShadows = false;
  patternSwitchStyle.drawFrame = false;
  patternSwitchStyle.valueText.mFGColor = COLOR_BG;
  patternSwitchStyle.valueText = patternSwitchStyle.valueText.WithFont("Roboto-Bold");

  rotaryStyle = (new IVStyle())->WithColors({
    COLOR_TRANSPARENT,
    COLOR_BG,
    COLOR_BG,
    COLOR_ACTIVE,
    COLOR_BG,
    COLOR_BG,
    COLOR_ACTIVE,
    });
  rotaryStyle.frameThickness = 2;
  rotaryStyle.drawFrame = false;
  rotaryStyle.drawShadows = false;
  rotaryStyle.valueText.mFGColor = COLOR_WHITE;
  rotaryStyle.valueText = rotaryStyle.valueText.WithFont("Roboto-Bold");
  rotaryStyle.labelText.mFGColor = COLOR_GRAY;
  rotaryStyle.labelText.mSize = 16;
  rotaryStyle.labelText = rotaryStyle.labelText.WithFont("Roboto-Bold");

  buttonStyle = (new IVStyle())->WithColors({
    COLOR_TRANSPARENT,
    COLOR_ACTIVE_DARK,
    COLOR_ACTIVE,
    COLOR_BG,
    COLOR_TRANSPARENT,
    COLOR_BG,
    });
  buttonStyle.roundness = 0.25;
  buttonStyle.drawShadows = false;
  buttonStyle.drawFrame = false;
  buttonStyle.showLabel = false;
  buttonStyle.valueText.mFGColor = COLOR_BG;
  buttonStyle.valueText.mSize = 16;
  buttonStyle.valueText = buttonStyle.valueText.WithFont("Roboto-Bold");

  numberStyle = (new IVStyle())->WithColors({
    COLOR_TRANSPARENT,
    COLOR_BG,
    COLOR_BG,
    COLOR_TRANSPARENT,
    COLOR_TRANSPARENT,
    });
  numberStyle.shadowOffset = 0;
  numberStyle.roundness = 0.5;
  numberStyle.valueText.mFGColor = COLOR_ACTIVE;
  numberStyle.valueText.mSize = 16;
  numberStyle.valueText = numberStyle.valueText.WithFont("Roboto-Bold");
  numberStyle.labelText.mFGColor = COLOR_ACTIVE;
  numberStyle.labelText.mSize = 16;
  numberStyle.labelText = numberStyle.labelText.WithFont("Roboto-Bold");
}

void GATE1::OnParamChange(int paramIdx)
{
  if (paramIdx == kPattern) {
    pattern = patterns[(int)GetParam(kPattern)->Value() - 1];
  }
  else if (paramIdx == kSync) {
    dirtyControls = true;
    auto sync = GetParam(kSync)->Value();
    if (sync == 0) syncQN = 0;
    if (sync == 1) syncQN = 1./4.; // 1/16
    if (sync == 2) syncQN = 1./2.; // 1/8
    if (sync == 3) syncQN = 1/1; // 1/4
    if (sync == 4) syncQN = 1*2; // 1/2
    if (sync == 5) syncQN = 1*4; // 1bar
    if (sync == 6) syncQN = 1*8; // 2bar
    if (sync == 7) syncQN = 1*16; // 4bar
    if (sync == 8) syncQN = 1./6.; // 1/16t
    if (sync == 9) syncQN = 1./3.; // 1/8t
    if (sync == 10) syncQN = 2./3.; // 1/4t
    if (sync == 11) syncQN = 4./3.; // 1/2t
    if (sync == 12) syncQN = 8./3.; // 1/1t
    if (sync == 13) syncQN = 1./4.*1.5; // 1/16.
    if (sync == 14) syncQN = 1./2.*1.5; // 1/8.
    if (sync == 15) syncQN = 1./1.*1.5; // 1/4.
    if (sync == 16) syncQN = 2./1.*1.5; // 1/2.
    if (sync == 17) syncQN = 4./1.*1.5; // 1/1.
  }
  else if (paramIdx == kGrid) {
    gridSegs = (int)GetParam(kGrid)->Value();
  }
  else if (paramIdx == kTension) {
    tensionMult = GetParam(kTension)->Value();
  }
  else if (paramIdx == kRetrigger && GetParam(kRetrigger)->Value() == 1) {
    midiTrigger = true;
  }
  else if (paramIdx == kAttack || paramIdx == kRelease || paramIdx == kSmooth) {
    setSmooth();
  }
  else if (paramIdx == kRetrigger && GetParam(kRetrigger)->Value() == 1 && canRetrigger()) {
    retriggerEnvelope();
  }
}

void GATE1::OnParentWindowResize(int width, int height)
{
  if (GetUI())
    GetUI()->Resize(width, height, 1.f, false);
}

bool GATE1::OnHostRequestingSupportedViewConfiguration(int width, int height)
{
  return ConstrainEditorResize(width, height);
}

void GATE1::OnHostSelectedViewConfiguration(int width, int height)
{
  if (GetUI())
    GetUI()->Resize(width, height, 1.f, true);
}

bool GATE1::canRetrigger()
{
  bool isSync = GetParam(kSync)->Value() > 0;
  return !midiMode && (!isSync && (alwaysPlaying || isPlaying)) || (alwaysPlaying && !isPlaying);
}

void GATE1::retriggerEnvelope()
{
  double phase = GetParam(kPhase)->Value();
  if (GetParam(kSync)->Value() > 0)
    beatPos = -phase * syncQN;
  else 
    beatPos = -phase;
}

double inline GATE1::getY(double x, double min, double max)
{
  return min + (max - min) * (1 - pattern->get_y_at(x));
}

void GATE1::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const int nChans = NOutChansConnected();
  const double srate = GetSampleRate();
  const double tempo = GetTempo();
  const double beatsPerSpl = tempo / (60. * srate);
  isPlaying = GetTransportIsRunning();
  const double sync = GetParam(kSync)->Value();
  const double phase = GetParam(kPhase)->Value();
  const double ratehz = GetParam(kRate)->Value();
  // update beatpos only in tempo sync mode during playback
  if (!midiMode && isPlaying && sync > 0) {
    beatPos = GetPPQPos();
  }
  const double lfomin = GetParam(kMin)->Value() / 100;
  const double lfomax = GetParam(kMax)->Value() / 100;
  double nextValue;

  auto processDisplaySamples = [&](int s) {
    winpos = std::floor(xpos * winw);
    if (lwinpos != winpos) {
      preSamples[winpos] = 0;
      postSamples[winpos] = 0;
    }
    lwinpos = winpos;
    double avgPreSample = 0;
    double avgPostSample = 0;
    for (int c = 0; c < nChans; ++c) {
      avgPreSample += inputs[c][s];
      avgPostSample += outputs[c][s];
    }
    avgPreSample = std::abs(avgPreSample / nChans);
    avgPostSample = std::abs(avgPostSample / nChans);
    if (preSamples[winpos] < avgPreSample)
      preSamples[winpos] = avgPreSample;
    if (postSamples[winpos] < avgPostSample)
      postSamples[winpos] = avgPostSample;
  };

  //mode == 0 && always_playing && !(play_state & 1) && retrigger && !lretrigger ? (
  //  retrigger_lfo();
  //);  

  if (!midiMode && (isPlaying || alwaysPlaying)) {
    for (int s = 0; s < nFrames; ++s) {
      if (sync > 0 && syncQN > 0) { // syncQN > 0 avoids crash where user changes rate mid processBlock
        beatPos += beatsPerSpl;
        xpos = beatPos / syncQN + phase;
      }
      else {
        beatPos += 1 / srate * ratehz;
        xpos = beatPos + phase;
      }
      xpos -= std::floor(xpos);

      nextValue = getY(xpos, lfomin, lfomax);
      ypos = value->smooth2(nextValue, nextValue > ypos);

      for (int c = 0; c < nChans; ++c) {
        outputs[c][s] = inputs[c][s] * ypos;
      }

      processDisplaySamples(s);
    }
  }

  else if (midiMode && (alwaysPlaying || midiTrigger)) {
    if (alwaysPlaying && midiTrigger) { // reset phase on midiTrigger
      xpos = phase;
      midiTrigger = false;
    }
    for (int s = 0; s < nFrames; ++s) {
      xpos += sync > 0
        ? beatsPerSpl / syncQN
        : 1 / srate * ratehz;
      if (!alwaysPlaying && xpos >= 1) {
        midiTrigger = false;
        xpos = 1;
      }
      else {
        xpos -= std::floor(xpos);
      }
      nextValue = getY(xpos, lfomin, lfomax);
      ypos = value->smooth2(nextValue, nextValue > ypos);
      for (int c = 0; c < nChans; ++c) {
        outputs[c][s] = inputs[c][s] * ypos;
      }
      processDisplaySamples(s);
    }
  }

  // keep processing the same position if stopped in MIDI mode
  else if (midiMode && !alwaysPlaying && !midiTrigger) {
    for (int s = 0; s < nFrames; ++s) {
      ypos = getY(xpos, lfomin, lfomax);
      for (int c = 0; c < nChans; ++c) {
        outputs[c][s] = inputs[c][s] * ypos;
      }
    }
  }

  else { // FIX Fl studio sound issues on stop play
    for (int s = 0; s < nFrames; ++s) {
      for (int c = 0; c < nChans; ++c) {
        outputs[c][s] = inputs[c][s];
      }
    }
  }
}

void GATE1::ProcessMidiMsg(const IMidiMsg& msg)
{
  int status = msg.StatusMsg();
  int channel = msg.Channel();
  int vel = msg.Velocity();

  if (status == IMidiMsg::kNoteOn && vel > 0 && (channel == triggerChannel - 1 || triggerChannel == 17)) {
    GetParam(kPattern)->Set(msg.NoteNumber() % 12 + 1);
    SendCurrentParamValuesFromDelegate();
    DirtyParametersFromUI();
  }
  else if (midiMode && status == IMidiMsg::kNoteOn && vel > 0) {
    midiTrigger = true;
    xpos = GetParam(kPhase)->Value();
  }
  SendMidiMsg(msg); // passthrough
}

void GATE1::OnReset()
{
  preSamples.clear();
  preSamples.resize(PLUG_MAX_WIDTH, 0);
  postSamples.clear();
  postSamples.resize(PLUG_MAX_WIDTH, 0);
  bool sync = GetParam(kSync)->Value() > 0;
  double phase = GetParam(kPhase)->Value();
  double min = GetParam(kMin)->Value() / 100;
  double max = GetParam(kMax)->Value() / 100;

  // reset value.smooth on play
  if (sync) {
    double x = GetPPQPos() / syncQN + phase;
    x -= std::floor(x);
    value->smooth = getY(x, min, max); 
  }

  // reset beatPos on play in Hz mode
  if (!midiMode && !sync) {
    beatPos = 0; 
  }
}

void GATE1::OnIdle()
{
  auto g = GetUI();
  if (dirtyControls && g) {
    dirtyControls = false;
    layoutControls(g);
    g->SetAllControlsDirty(); // FIX - erases extra drawn knob
  }
  if (g && canRetrigger() != !retriggerControl->IsHidden()) {
    retriggerControl->Hide(!canRetrigger());
  }
}

bool GATE1::SerializeState(IByteChunk &chunk) const
{
  chunk.Put(&linkEdgePoints);
  chunk.Put(&dualSmooth);
  chunk.Put(&triggerChannel);
  chunk.Put(&drawWave);
  chunk.Put(&alwaysPlaying);
  chunk.Put(&midiMode);

  // reserved space
  int cSize = chunk.Size();
  char zero[1];
  memset(zero, 0, 1);
  for (int i = 0; i < 1024 - cSize; ++i)
    chunk.PutBytes(zero, 1);

  // serialize patterns
  for (int i = 0; i < 12; ++i) {
    auto pattern = patterns[i];
    int size = pattern->points.size();
    chunk.Put(&size);
    for (int j = 0; j < size; ++j) {
      auto point = pattern->points[j];
      chunk.Put(&point);
    }
  }

  return SerializeParams(chunk);
}

// this over-ridden method is called when the host is trying to load the plug-in state and you need to unpack the data into your algorithm
int GATE1::UnserializeState(const IByteChunk &chunk, int startPos)
{
  startPos = chunk.Get(&linkEdgePoints, startPos);
  startPos = chunk.Get(&dualSmooth, startPos);
  startPos = chunk.Get(&triggerChannel, startPos);
  startPos = chunk.Get(&drawWave, startPos);
  startPos = chunk.Get(&alwaysPlaying, startPos);
  startPos = chunk.Get(&midiMode, startPos);

  // reserved space
  char buffer[1];
  int size = 1024 - startPos;
  for (int i = 0; i < size; ++i)
    startPos = chunk.GetBytes(buffer, 1, startPos);

  for (int i = 0; i < 12; ++i) {
    auto pattern = patterns[i];
    pattern->points.clear();
    int size;
    startPos = chunk.Get(&size, startPos);
    for (int j = 0; j < size; ++j) {
      Point point;
      startPos = chunk.Get(&point, startPos);
      pattern->points.push_back(point);
    }
    pattern->buildSegments();
  }

  return UnserializeParams(chunk, startPos);
}

void GATE1::OnRestoreState() {
  SendCurrentParamValuesFromDelegate();
  setSmooth();
  dirtyControls = true;
};