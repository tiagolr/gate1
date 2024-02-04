/*
  ==============================================================================

    Pattern.h
    Author:  tiagolr

  ==============================================================================
*/

#pragma once
#include <vector>

class GATE2;  // Forward declaration

struct Point {
    double x;
    double y;
    double tension;
    int type;
};

struct Segment {
    double x1;
    double x2;
    double y1;
    double y2;
    double tension;
    double power;
    int type;
};

class Pattern
{
public:
    int index;
    std::vector<Point> points;
    std::vector<Segment> segments;

    Pattern(GATE2&, int);
    int insertPoint(double x, double y, double tension, int type);
    void removePoint(double x, double y);
    void removePoint(int i);
    void removePointsInRange(double x1, double x2);
    void invert();
    void reverse();
    void clear();
    void buildSegments();
    void loadSine();
    void loadTriangle();
    void loadRandom(int grid);

    double get_y_curve(Segment seg, double x);
    double get_y_at(double x);

private:
    GATE2& gate;
};