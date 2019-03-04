/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: CommunicationAngle.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef CommunicationAngle_HEADER
#define CommunicationAngle_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "Line.h"

class CommunicationAngle : public CMOOSApp
{
 public:
   CommunicationAngle();
   ~CommunicationAngle();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();
   void RegisterCollaborator(string collaborator);
   double calculateFireAngle(Point circleCenter) const;
   Point getCircleCenter() const;
   double calculateCircleRadius(Point circleCenter) const;
   double calculateArcLength(double radius) const;
   double calculateTransmissionLoss(double archlength) const;

 private: // Configuration variables
	 unsigned int m_surface_sound_speed;
	 unsigned int m_sound_speed_gradient;
	 unsigned int m_water_depth;
	 double m_time_interval;
	 string m_vehicle_name;
 
 private: // State variables
	 string m_collaborator;
	 double m_nav_x;
	 double m_nav_y;
	 double m_nav_depth;
	 double m_nav_heading;
	 double m_nav_speed;
	 double m_col_nav_x;
	 double m_col_nav_y;
	 double m_col_nav_depth;
	 double m_col_nav_heading;
	 double m_col_nav_speed;
};

#endif 
