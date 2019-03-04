#ifndef Line_HEADER
#define Line_HEADER

#include <cmath>
#include "Point.cpp"
#include "MOOS/libMOOS/MOOSLib.h"
using namespace std;

class Line

{
public: 
	Line(Point p_1,Point p_2);
	Line(Point p_1,double dir);
	~Line() {};

public:
	Point getCrossing(double) const;
	void printLine() const;
	double getGradient() const {return m_gradient;};
	Point getMyPos() const {return m_myPos;};

private: 
	Point m_myPos;
	Point m_otherPos;
	double m_gradient;
};

#endif