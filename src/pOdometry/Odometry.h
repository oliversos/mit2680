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

 protected: 
 // Specific MOOSApp functions for registering variables and AppCasting
   void RegisterVariables();
   bool buildReport();

 private: 
	// Configuration variables. m_first_readings variables describes if the position is set. This is used to calculate the correct total distance in the special case where the starting point of the vehicle is not (0,0). m_previous_x and m_previous_y is continously used to update the total distance traveled. 
 	bool m_first_reading_x;
 	bool m_first_reading_y;
 	double m_previous_x;
 	double m_previous_y;

 private: 
 // State variables. Describes the current position and distance traveled by the vehicle.
 	double m_current_x;
 	double m_current_y;
 	double m_total_distance;
};

#endif 
