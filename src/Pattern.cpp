/*
  ==============================================================================

    Pattern.cpp
    Author:  tiagolr

  ==============================================================================
*/

#include "Pattern.h"
#include "GATE2.h"

Pattern::Pattern(GATE2& p, int i) : gate(p)
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

void Pattern::removePointsInRange(double x1, double x2)
{
    const auto end = points.end();
    for (auto i = points.begin(); i != end;) {
        if (i->x >= x1 && i->x <= x2 && i != end - 1)
            i = points.erase(i);
        else
            ++i;
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

    for (size_t i = 0; i < points.size() - 1; ++i) {
        auto p = points[i];
        p.x = 1 - p.x;
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
        insertPoint(std::fmin(0.9999999, std::fmax(0.000001, r1 / grid + i / grid)), r2, 0, 1);
    }
};

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
};

double Pattern::get_y_at(double x)
{
    for (auto seg = segments.begin(); seg != segments.end(); ++seg) {
        if (seg->x1 <= x && seg->x2 >= x) {
            if (seg->type == 0) return seg->y1; // hold
            if (seg->type == 1) return get_y_curve(*seg, x);
        }
    }

    return -1;
}