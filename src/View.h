/*
  ==============================================================================

    View.h
    Author:  tiagolr

  ==============================================================================
*/

#pragma once
#include "GATE2.h"
#include "IControl.h"

//class GATE2; // Forward declaration

class View : public IControl
{
public:
  int winx = 0;
  int winy = 0;
  int winw = 0;
  int winh = 0;

  View(const IRECT&, GATE2&);
  void Draw(IGraphics& g) override;
  void OnResize() override;
  bool IsDirty() override {
    return true;
  }

  void drawGrid(IGraphics& g);
  void drawSegments(IGraphics& g);

private:
  GATE2& gate;
};