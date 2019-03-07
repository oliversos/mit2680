/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT Cambridge MA                                */
/*    FILE: Line.h                                          */
/*    DATE: 28.02.2019                                      */
/************************************************************/

#ifndef Line_HEADER
#define Line_HEADER

#include <cmath>
#include "Point.cpp"
using namespace std;


// This class represents a line, and is used to calculate different line-operations
class Line

{
public:  // Constructors and destructor
	Line(Point p_1,Point p_2);
	Line(Point p_1,double dir);
	~Line() {};

public:  
	// Getters
	double getGradient() const {return m_gradient;};
	Point getMyPos() const {return m_myPos;};

	// Calculates the point where the line intersects with a given z coordinate as input.
	Point getCrossing(double) const;

	// Prints out the characteristics of the line
	void printLine() const;

private: 
	// A point on the line, used to initiate it
	Point m_myPos;
  
  // A second point on the line, used to initiate it
	Point m_otherPos;

	// The gradient of the line, defined in radians
	double m_gradient;
};

#endif