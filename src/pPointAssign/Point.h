/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: Point.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef Point_HEADER
#define Point_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

using namespace std;

class Point 
{
public:
	Point() {};
	Point(int x, int y, int id);
	~Point() {};

public: 
	void setX(int x)   {m_x = x;};
	void setY(int y)   {m_y = y;};
	void setID(int id) {m_id = id;}; 
	string getString();

public: 
	int getX()   {return m_x;};
	int getY()   {return m_y;};
	int getID()  {return m_id;};

private:
	int m_x;
	int m_y;
	int m_id;
};

#endif
	