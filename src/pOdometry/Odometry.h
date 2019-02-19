/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT Cambridge MA                                */
/*    FILE: Odometry.h                                      */
/*    DATE: 14.02.2019                                      */
/************************************************************/

#ifndef Odometry_HEADER
#define Odometry_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"


class Odometry : public AppCastingMOOSApp
{
 public: // Constructor and desctructor
   Odometry();
   ~Odometry();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Specific MOOSApp functions for registering variables and AppCasting
   void RegisterVariables();
   bool buildReport();

 private: // Configuration variables
 	bool m_first_reading;
 	double m_previous_x;
 	double m_previous_y;
 	double m_total_distance;

 private: // State variables
 	double m_current_x;
 	double m_current_y;
};

#endif 
