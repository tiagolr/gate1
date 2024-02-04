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
  drawGrid(g);
  drawSegments(g);
  drawMidPoints(g);
  drawPoints(g);
}

void View::drawGrid(IGraphics& g)
{
  int grid = (double)gate.gridSegs;
  double gridx = double(winw) / grid;
  double gridy = double(winh) / grid;

  auto colorNormal = COLOR_WHITE.WithOpacity(0.15f);
  auto colorBold = COLOR_WHITE.WithOpacity(0.30f);

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

void View::drawPoints(IGraphics& g)
{
  auto points = gate.pattern->points;

  for (auto pt = points.begin(); pt != points.end(); ++pt)
  {
    auto xx = pt->x * winw + winx;
    auto yy = pt->y * winh + winy;
    g.FillCircle(COLOR_WHITE, xx, yy, 4);
  }

  if (selectedPoint == -1 && selectedMidpoint == -1 && hoverPoint > -1)
  {
    auto xx = points[hoverPoint].x * winw + winx;
    auto yy = points[hoverPoint].y * winh + winy;
    g.FillCircle(COLOR_WHITE.WithOpacity(0.5), xx, yy, HOVER_RADIUS);
  }

  if (selectedPoint != -1)
  {
    auto xx = points[selectedPoint].x * winw + winx;
    auto yy = points[selectedPoint].y * winh + winy;
    g.FillCircle(COLOR_RED.WithOpacity(0.5), xx, yy, 4);
  }
}

std::vector<double> View::getMidpointXY(Segment seg)
{
  double x = (seg.x1 + seg.x2) * 0.5;
  double y = seg.type > 1
    ? (seg.y1 + seg.y2) / 2
    : gate.pattern->get_y_at(x);
  double xx = x * winw + winx;
  double yy = y * winh + winy;
  return { xx, yy };
}

void View::drawMidPoints(IGraphics& g)
{
  auto segs = gate.pattern->segments;

  for (auto seg = segs.begin(); seg != segs.end(); ++seg) {
    if (!isCollinear(*seg)) {
      auto xy = getMidpointXY(*seg);
      g.DrawCircle(GATE2::COLOR_ACTIVE, xy[0], xy[1], 3, 0, 2);
    }
  }

  if (selectedPoint == -1 && selectedMidpoint == -1 && hoverMidpoint != -1) {
    auto seg = segs[hoverMidpoint];
    auto xy = getMidpointXY(seg);
    g.FillCircle(GATE2::COLOR_ACTIVE.WithOpacity(0.5), xy[0], xy[1], HOVER_RADIUS);
  }

  if (selectedMidpoint != -1) {
    auto seg = segs[selectedMidpoint];
    auto xy = getMidpointXY(seg);
    g.FillCircle(GATE2::COLOR_ACTIVE, xy[0], xy[1], 3);
  }
}

int View::getHoveredPoint(int x, int y)
{
  auto points = gate.pattern->points;
  for (auto i = 0; i < points.size(); ++i) {
    auto xx = points[i].x * winw + winx;
    auto yy = points[i].y * winh + winy;
    if (pointInRect(x, y, xx - HOVER_RADIUS, yy - HOVER_RADIUS, HOVER_RADIUS * 2, HOVER_RADIUS * 2)) {
      return i;
    }
  }
  return -1;
};

int View::getHoveredMidpoint(int x, int y)
{
  auto segs = gate.pattern->segments;
  for (auto i = 0; i < segs.size(); ++i) {
    auto seg = segs[i];
    auto xy = getMidpointXY(seg);
    if (!isCollinear(seg) && pointInRect(x, y, xy[0] - HOVER_RADIUS, xy[1] - HOVER_RADIUS, HOVER_RADIUS * 2, HOVER_RADIUS * 2)) {
      return i;
    }
  }
  return -1;
};

void View::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  if (mod.L) {
    selectedPoint = getHoveredPoint((int)x, (int)y);
    if (selectedPoint == -1)
      selectedMidpoint = getHoveredMidpoint((int)x, (int)y);

    if (selectedPoint > -1 || selectedMidpoint > -1) {
      if (selectedMidpoint > -1) {
        origTension = gate.pattern->points[selectedMidpoint].tension;
        dragStartY = (int)y;
        GetUI()->HideMouseCursor(true, true);
      }
      else {
        GetUI()->HideMouseCursor(true, false);
      }
    }
  }
}

void View::OnMouseUp(float x, float y, const IMouseMod& mod)
{
  selectedMidpoint = -1;
  selectedPoint = -1;
  GetUI()->HideMouseCursor(false);
}

void View::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod)
{
  if (selectedPoint == -1 && selectedMidpoint == -1)
    return;

  auto& points = gate.pattern->points;
  int grid = (double)gate.gridSegs;
  double gridx = double(winw) / grid;
  double gridy = double(winh) / grid;
  double xx = (double)x;
  double yy = (double)y;
  if (isSnapping(mod)) {
    xx = round((xx - winx) / gridx) * gridx + winx;
    yy = round((yy - winy) / gridy) * gridy + winy;
  }
  xx = (xx - winx) / winw;
  yy = (yy - winy) / winh;

  if (selectedPoint > -1) {
    auto& point = points[selectedPoint];
    point.y = yy;
    if (point.y > 1) point.y = 1;
    if (point.y < 0) point.y = 0;

    if (selectedPoint == 0 && gate.linkEdgePoints) {
      auto next = points[selectedPoint + 1];
      next.y = point.y;
    }

    if (selectedPoint == points.size() - 1 && gate.linkEdgePoints) {
      auto first = points[0];
      first.y = point.y;
    }

    if (selectedPoint > 0 && selectedPoint < points.size() - 1) {
      point.x = xx;
      if (point.x > 1) point.x = 1;
      if (point.x < 0) point.x = 0;
      auto prev = points[selectedPoint - 1];
      auto next = points[selectedPoint + 1];
      if (point.x < prev.x) point.x = prev.x;
      if (point.x > next.x) point.x = next.x;
    }
  }

  if (selectedMidpoint > -1) {
    int distance = (int)y - dragStartY;
    auto& mpoint = points[selectedMidpoint];
    auto next = points[selectedMidpoint + 1];
    if (mpoint.y < next.y) distance *= -1;
    float tension = mpoint.tension + float(distance) / 500.;
    if (tension > 1) tension = 1;
    if (tension < -1) tension = -1;
    mpoint.tension = tension;
  }

  gate.pattern->buildSegments();
}

void View::OnMouseOver(float x, float y, const IMouseMod& mod)
{
  hoverPoint = getHoveredPoint((int)x , (int)y);
  if (hoverPoint == -1)
    hoverMidpoint = getHoveredMidpoint((int)x, (int)y);
}

void View::OnMouseDblClick(float x, float y, const IMouseMod& mod)
{
  auto& points = gate.pattern->points;
  auto segs = gate.pattern->segments;
  int pt = getHoveredPoint((int)x, (int)y);
  int mid = getHoveredMidpoint((int)x, (int)y);
  if (pt > 0 && pt < points.size() - 1) {
    gate.pattern->removePoint(pt);
  }
  if (pt == -1 && mid > -1) {
    points[mid].tension = 0;
  }
  if (pt == -1 && mid == -1) {
    int px = (int)x;
    int py = (int)y;
    if (isSnapping(mod)) {
      int grid = (double)gate.gridSegs;
      double gridx = double(winw) / grid;
      double gridy = double(winh) / grid;
      px = std::round(double(px - winx) / gridx) * gridx + winx;
      py = std::round(double(px - winx) / gridy) * gridy + winy;
    }
    double x = double(px - winx) / winw;
    double y = double(py - winy) / winh;
    if (x >= 0 && x <= 1 && y >= 0 && y <= 1) { // point in env window
      if (x == 1) x -= 0.000001; // special case avoid inserting point after last point
      DBGMSG("POINT %i", (int)gate.GetParam(kPointMode)->Value());
      gate.pattern->insertPoint(x, y, 0, (int)gate.GetParam(kPointMode)->Value());
    }
  }

  gate.pattern->buildSegments();
}

// ==================================================

bool View::isSnapping(const IMouseMod& mod) {
  bool snap = gate.GetParam(kSnap)->Value() == 1;
  return (snap && !mod.C) || (!snap && mod.C);
}

bool View::isCollinear(Segment seg)
{
  return std::fabs(seg.x1 - seg.x2) < 0.01 || std::fabs(seg.y1 - seg.y2) < 0.01;
};

bool View::pointInRect(int x, int y, int xx, int yy, int w, int h)
{
  return x >= xx && x <= xx + w && y >= yy && y <= yy + h;
};