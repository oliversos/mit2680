/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT Cambridge MA                                */
/*    FILE: Point.cpp                                       */
/*    DATE: 28.02.2019                                      */
/************************************************************/

// This class represents a point, and is used to to represent positions in x- and depth efficiently.
class Point
{

	public:// Constructors and destructor
	 Point(double x, double y){m_x = x; m_y = y;};
	 Point() : Point(0,0){};
	 ~Point(){};

	 //Getters
	 double getX() const {return m_x;};
	 double getY() const {return m_y;};

	 //Setters
	 void setX(double x){m_x = x;};
	 void setY(double y){m_y = y;};

	 private:
	 //The x position m_x, and the depth represented as m_y
	 double m_x,m_y;
};