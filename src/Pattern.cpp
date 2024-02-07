/*
  ==============================================================================

    Pattern.cpp
    Author:  tiagolr

  ==============================================================================
*/

#include "Pattern.h"
#include "GATE1.h"

std::vector<Point> Pattern::copy_pattern;

Pattern::Pattern(GATE1& p, int i) : gate(p)
{
    index = i;
};

int Pattern::insertPoint(double x, double y, double tension, int type)
{
    const Point p = { x, y, tension, type };
    if (!points.size()) {
        points.push_back(p);
        return 0;
    }

    for (size_t i = points.size() - 1; i >= 0; --i) {
        if (points[i].x <= x) {
            points.insert(points.begin() + i + 1, p);
            return (int)i;
        }
    }

    return -1;
};

void Pattern::removePoint(double x, double y)
{
    for (size_t i = 0; i < points.size(); ++i) {
        if (points[i].x == x && points[i].y == y) {
            points.erase(points.begin() + i);
            return;
        }
    }
}

void Pattern::removePoint(int i) {
    points.erase(points.begin() + i);
}

//void Pattern::removePointsInRange(double x1, double x2)
//{
//    const auto end = points.end();
//    for (auto i = points.begin(); i != end;) {
//        if (i->x >= x1 && i->x <= x2 && i != end - 1)
//            i = points.erase(i);
//        else
//            ++i;
//    }
//}

void Pattern::removePointsInRange(double x1, double x2)
{
  if (points.size() <= 2) return; // Skip if there are only two points or less

  auto start = points.begin() + 1; // Skip the first point
  auto end = points.end() - 1; // Skip the last point

  for (auto i = start; i != end; ++i) {
    if (i->x >= x1 && i->x <= x2) {
      i = points.erase(i);
      removePointsInRange(x1, x2);
      return;
    }
  }
}

void Pattern::invert()
{
    for (auto i = points.begin(); i != points.end(); ++i) {
        i->y = 1 - i->y;
    }
};

void Pattern::reverse()
{
    std::reverse(points.begin(), points.end());

    for (size_t i = 0; i < points.size(); ++i) {
        auto& p = points[i];
        p.x = 1 - p.x;
        if (i < points.size() - 1)
          p.tension = points[i + 1].tension * -1;
    }
};

void Pattern::clear()
{
    points.clear();
    insertPoint(0, 0.5, 0, 1);
    insertPoint(1, 0.5, 0, 1);
}

void Pattern::buildSegments()
{
    std::lock_guard<std::mutex> lock(mtx); // prevents crash while reading Y from another thread
    segments.clear();
    for (size_t i = 0; i < points.size() - 1; ++i) {
        auto p1 = points[i];
        auto p2 = points[i + 1];
        //auto pwr = std::pow(1.1, std::fabs(p1.tension * 50));
        segments.push_back({p1.x, p2.x, p1.y, p2.y, p1.tension, 0, p1.type});
    }
}

void Pattern::loadSine() {
    points.clear();
    insertPoint(0, 1, 0.33, 1);
    insertPoint(0.25, 0.5, -0.33, 1);
    insertPoint(0.5, 0, 0.33, 1);
    insertPoint(0.75, 0.5, -0.33, 1);
    insertPoint(1, 1, 0, 1);
}

void Pattern::loadTriangle() {
    points.clear();
    insertPoint(0, 1, 0, 1);
    insertPoint(0.5, 0, 0, 1);
    insertPoint(1, 1, 0, 1);
};

void Pattern::loadRandom(int grid) {
    points.clear();
    auto y = static_cast<double>(rand())/RAND_MAX;
    insertPoint(0, y, 0, 1);
    insertPoint(1, y, 0, 1);
    for (auto i = 0; i < grid; ++i) {
        auto r1 = static_cast<double>(rand()) / RAND_MAX;
        auto r2 = static_cast<double>(rand()) / RAND_MAX;
        insertPoint(std::min(0.9999999, std::max(0.000001, r1 / (double)grid + i / (double)grid)), r2, 0, 1);
    }
};

void Pattern::copy()
{
  copy_pattern = points;
}

void Pattern::paste()
{
  if (copy_pattern.size() > 0) {
    points = copy_pattern;
  }
}

/*
  Based of https://github.com/KottV/SimpleSide/blob/main/Source/types/SSCurve.cpp
*/
double Pattern::get_y_curve(Segment seg, double x)
{
    auto rise = seg.y1 > seg.y2;
    auto tmult = gate.tensionMult;
    auto ten = seg.tension + (rise ? -tmult / 100 : tmult / 100);
    if (ten > 1) ten = 1;
    if (ten < -1) ten = -1;
    auto pwr = pow(1.1, std::fabs(ten * 50));

    if (seg.x1 == seg.x2)
        return seg.y2;

    if (ten >= 0)
        return std::pow((x - seg.x1) / (seg.x2 - seg.x1), pwr) * (seg.y2 - seg.y1) + seg.y1;

    return -1 * (std::pow(1 - (x - seg.x1) / (seg.x2 - seg.x1), pwr) - 1) * (seg.y2 - seg.y1) + seg.y1;
}


double Pattern::get_y_scurve(Segment seg, double x)
{
  auto rise = seg.y1 > seg.y2;
  auto tmult = gate.tensionMult;
  auto ten = seg.tension + (rise ? -tmult / 100 : tmult / 100);
  if (ten > 1) ten = 1;
  if (ten < -1) ten = -1;
  auto pwr = pow(1.1, std::fabs(ten * 50));

  double xx = (seg.x2 + seg.x1) / 2;
  double yy = (seg.y2 + seg.y1) / 2;

  if (seg.x1 == seg.x2)
    return seg.y2;

  if (x < xx && ten >=0)
    return std::pow((x - seg.x1) / (xx - seg.x1), pwr) * (yy - seg.y1) + seg.y1; 
    
  if (x < xx && ten < 0)
    return -1 * (std::pow(1 - (x - seg.x1) / (xx - seg.x1), pwr) - 1) * (yy - seg.y1) + seg.y1;

  if (x >= xx && ten >= 0)
    return -1 * (std::pow(1 - (x - xx) / (seg.x2 - xx), pwr) - 1) * (seg.y2 - yy) + yy;

   return std::pow((x - xx) / (seg.x2 - xx), pwr) * (seg.y2 - yy) + yy;
}

double Pattern::get_y_pulse(Segment seg, double x)
{
  double t = std::max(std::floor(std::pow(seg.tension,2) * 100), 1.0); // num waves

  if (x == seg.x2)
    return seg.y2; 
    
  double cycle_width = (seg.x2 - seg.x1) / t;
  double x_in_cycle = mod((x - seg.x1), cycle_width);
  return x_in_cycle < cycle_width / 2
    ? (seg.tension >= 0 ? seg.y1 : seg.y2)
    : (seg.tension >= 0 ? seg.y2 : seg.y1);
}

double Pattern::get_y_wave(Segment seg, double x)
{
  double t = 2 * std::floor(std::fabs(std::pow(seg.tension,2) * 100) + 1) - 1; // wave num
  double amp = (seg.y2 - seg.y1) / 2;
  double vshift = seg.y1 + amp;
  double freq = t * 2 * PI / (2 * (seg.x2 - seg.x1));
  return -amp * cos(freq * (x - seg.x1)) + vshift;
}

double Pattern::get_y_triangle(Segment seg, double x)
{
  double tt = 2 * std::floor(std::fabs(std::pow(seg.tension,2) * 100) + 1) - 1.0;// wave num
  double amp = seg.y2 - seg.y1;
  double t = (seg.x2 - seg.x1) * 2 / tt;
  return amp * (2 * std::fabs((x - seg.x1) / t - std::floor(1./2. + (x - seg.x1) / t))) + seg.y1;
}

double Pattern::get_y_stairs(Segment seg, double x)
{
  double t = std::max(std::floor(std::pow(seg.tension,2) * 150), 2.); // num waves
  double step_size = 0.;
  double step_index = 0.;
  double y_step_size = 0.;

  if (seg.tension >= 0) {
    step_size = (seg.x2 - seg.x1) / t;
    step_index = std::floor((x - seg.x1) / step_size);
    y_step_size = (seg.y2 - seg.y1) / (t-1);
  }
  else {
    step_size = (seg.x2 - seg.x1) / (t-1);
    step_index = ceil((x - seg.x1) / step_size);
    y_step_size = (seg.y2 - seg.y1) / t;
  }

  if (x == seg.x2)
    return seg.y2;
  
  return seg.y1 + step_index * y_step_size;
}

double Pattern::get_y_smooth_stairs(Segment seg, double x)
{
  double pwr = 4;
  double t = std::max(floor(std::pow(seg.tension,2) * 150), 1.0); // num waves

  double gx = (seg.x2 - seg.x1) / t; // gridx
  double gy = (seg.y2 - seg.y1) / t; // gridy
  double step_index = std::floor((x - seg.x1) / gx);

  double xx1 = seg.x1 + gx * step_index;
  double xx2 = seg.x1 + gx * (step_index + 1);
  double xx = (xx1 + xx2) / 2;

  double yy1 = seg.y1 + gy * step_index;
  double yy2 = seg.y1 + gy * (step_index + 1);
  double yy = (yy1 + yy2) / 2;

  if (seg.x1 == seg.x2)
    return seg.y2;

  if (x < xx && seg.tension >= 0)
    return std::pow((x - xx1) / (xx - xx1), pwr) * (yy - yy1) + yy1;

  if (x < xx && seg.tension < 0)
    return -1 * (std::pow(1 - (x - xx1) / (xx - xx1), pwr) - 1) * (yy - yy1) + yy1;

  if (x >= xx && seg.tension >= 0)
    return -1 * (std::pow(1 - (x - xx) / (xx2 - xx), pwr) - 1) * (yy2 - yy) + yy;

  return std::pow((x - xx) / (xx2 - xx), pwr) * (yy2 - yy) + yy;
}


double Pattern::get_y_at(double x)
{
    std::lock_guard<std::mutex> lock(mtx); // prevents crash while building segments
    for (auto seg = segments.begin(); seg != segments.end(); ++seg) {
        if (seg->x1 <= x && seg->x2 >= x) {
            if (seg->type == 0) return seg->y1; // hold
            if (seg->type == 1) return get_y_curve(*seg, x);
            if (seg->type == 2) return get_y_scurve(*seg, x);
            if (seg->type == 3) return get_y_pulse(*seg, x);
            if (seg->type == 4) return get_y_wave(*seg, x);
            if (seg->type == 5) return get_y_triangle(*seg, x);
            if (seg->type == 6) return get_y_stairs(*seg, x);
            if (seg->type == 7) return get_y_smooth_stairs(*seg, x);
        }
    }

    return -1;
}

/*
  Modulus that works with fractional numbers
*/
double Pattern::mod(double a, double b)
{
  while( a >= b )
    a -= b;
  return a;
}
