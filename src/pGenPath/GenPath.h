/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: GenPath.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef GenPath_HEADER
#define GenPath_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "../pPointAssign/Point.h"


class GenPath : public CMOOSApp
{
 public:
   GenPath();
   ~GenPath();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();
   double calcDist(Point p1, Point p2);
   double calcDoubleDist(Point p,double x, double y);

 private: 
 	bool m_initiated;
 	vector<Point> m_visit_points;
 	double m_x_start;
 	double m_y_start;
 	double m_visit_radius;
 	double m_curr_x;
 	double m_curr_y;
};

#endif 
