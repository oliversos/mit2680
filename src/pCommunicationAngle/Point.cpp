class Point
{
	private:
	 double m_x,m_y;

	public:
	 Point(double x, double y){m_x = x; m_y = y;};
	 Point() : Point(0,0){};
	 ~Point(){};
	 double getX() const {return m_x;};
	 double getY() const {return m_y;};
	 void setX(double x){m_x = x;};
	 void setY(double y){m_y = y;};
};