#include "Line.h"
#include <iostream>

using namespace std;


int main()
{
	Point p1 = {9,2};
	Point p2 = {3,6};
	Line l(p1,p2);
	Point center = l.getCrossing(-10);
	cout << center.getX() << ", " << center.getY() << endl;
	Line toCircleCenter(p1,center);
  double angle = toCircleCenter.getGradient();
  cout << "Angle: " << angle << endl;
}