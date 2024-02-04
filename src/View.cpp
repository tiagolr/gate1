/*
  ==============================================================================

    View.cpp
    Author:  tiagolr

  ==============================================================================
*/

#include "View.h"
#include "IControl.h"
#include "IPlugTimer.h"

View::View(const IRECT& bounds, GATE2& p) : IControl(bounds), gate(p)
{
  
};

void View::OnResize() {
  winx = mRECT.L + 10;
  winy = mRECT.T + 10;
  winw = mRECT.W() - 20;
  winh = mRECT.H() - 20;
}

void View::Draw(IGraphics& g) {
  g.DrawRect(COLOR_GREEN, mRECT);
  drawGrid(g);
  drawSegments(g);
}

void View::drawGrid(IGraphics& g)
{
  int grid = (double)8;
  double gridx = double(winw) / grid;
  double gridy = double(winh) / grid;

  auto colorNormal = COLOR_WHITE.WithOpacity(0.15);
  auto colorBold = COLOR_WHITE.WithOpacity(0.75);

  for (int i = 0; i < grid + 1; ++i)
  {
    auto color = grid % 4 == 0 && i && i % 4 == 0 && i < grid ? colorBold : colorNormal;
    g.DrawLine(color, winx, winy + gridy * i, winx + winw, winy + gridy * i);
    g.DrawLine(color, winx + gridx * i, winy, winx + gridx * i, winy + winh);
  }
}

void View::drawSegments(IGraphics& g)
{
  auto points = gate.pattern->points;
  double lastX = winx;
  double lastY = points[0].y * winh + winy;

  auto colorBold = COLOR_WHITE;
  auto colorLight = COLOR_WHITE.WithOpacity(0.0625);

  for (int i = 0; i < winw + 1; ++i)
  {
    double px = double(i) / double(winw);
    double py = gate.pattern->get_y_at(px) * winh + winy;
    g.DrawLine(colorLight, i + winx, winy + winh, i + winx, py);
    g.DrawLine(colorBold, lastX, lastY, i + winx, py, 0, 2);
    lastX = i + winx;
    lastY = py;
  }

  g.DrawLine(colorBold, lastX, lastY, winw + winx, points[points.size() - 1].y * winh + winy);
}