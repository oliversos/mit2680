/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT Cambridge MA                                */
/*    FILE: Line.cpp                                        */
/*    DATE: 28.02.2019                                      */
/************************************************************/

#include "Line.h"
#include <math.h>  
#include <iostream>

using namespace std;


//------------------------------------------------------------
// Procedure: Line()
// Purpose: Constructor. Creates a Line object from two points on the line 
Line::Line(Point p_1, Point p_2)
{
	m_myPos = p_1;
	m_otherPos = p_2;
	m_gradient = atan2(p_2.getY() - p_1.getY(), p_2.getX() - p_1.getX());
}


//------------------------------------------------------------
// Procedure: Line()
// Purpose: Constructor. Creates a Line object from a point on the line and //          the direction of the line
Line::Line(Point p_1, double dir)
{
	m_myPos = p_1;
	m_gradient = dir;
	Point other = {p_1.getX() + 100*cos(dir), p_1.getY()+ 100*sin(dir)};
	m_otherPos = other;
}


//------------------------------------------------------------
// Procedure: getCrossing()
//   Purpose: Calculates the point where the line intersects with a given //            z-coordinate 
//     Input: The z-coordinate of intersection.
//   Returns: A point object that describes the intersection point
Point Line::getCrossing(double z_coord) const
{
	Point mid = {m_myPos.getX() + (m_otherPos.getX() - m_myPos.getX())/2, 
	m_myPos.getY() + (m_otherPos.getY() - m_myPos.getY())/2};
	double direction = m_gradient - M_PI/2;
	Line perpen (mid,direction);

	if (abs(perpen.getGradient() + M_PI/2) < 0.0001){
		return Point(perpen.getMyPos().getX(), z_coord);
	}
	else{
		double x = perpen.getMyPos().getX() + (perpen.getMyPos().getY() - z_coord)/tan(-perpen.getGradient());
 		return Point(x,z_coord);
	}
}


//------------------------------------------------------------
// Procedure: printLine()
//   Purpose: Prints out the member variables of the line.
void Line::printLine() const
{
	cout << "Line: x:" << m_myPos.getX() << ", y:" << m_myPos.getY() << ",  dir: " << m_gradient << endl;
}