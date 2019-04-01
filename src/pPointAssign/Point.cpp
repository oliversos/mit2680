#include "Point.h"
#include "sstream"

using namespace std;


Point::Point(int x, int y, int id)
{
	setX(x);
	setY(y);
	setID(id);
}

string Point::getString()
{
	stringstream ss;
	ss << "x:"<<m_x<<" y:"<<m_y<<" id:"<<m_id;
	return ss.str();
}