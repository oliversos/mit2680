/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT                                             */
/*    FILE: CommunicationAngle.h                            */
/*    DATE: 02.28.2019                                      */
/************************************************************/

#ifndef CommunicationAngle_HEADER
#define CommunicationAngle_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "Line.h"

class CommunicationAngle : public CMOOSApp
{
 public: // Constructor and destructor
   CommunicationAngle();
   ~CommunicationAngle();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
 	 //Registers the app for the relevant variables
   void RegisterVariables();

   //Registers the app for the information of the colaborator vehicle
   void RegisterCollaborator(string collaborator);

   //Calculates the angle of the sound wave such that the collaborator recieves the message
   double calculateFireAngle(Point circleCenter) const;

   //Calciulates the center of the circle for which the arc defines the propagation of a soundwave
   Point getCircleCenter(double nav_x,double nav_y) const;

   // Calculates the radius of the circle for which arc your want to fire the soundwave along 
   double calculateCircleRadius(Point circleCenter) const;

   //Calculates the lenght of the arch which the soundwave travels from our vehicle to the collaborator
   double calculateArcLength(double radius) const;

   //Calculates the transmissionloss of a given message send through water
   double calculateTransmissionLoss(double radius,double archlength) const;

   //Calculates the soundspeed at a given depth
   double calculateSoundSpeed(double depth) const;

 private: // Configuration variables

   // The sound speed at surface, the gradient which represents the increase in speed with depth and the max water depth
	 double m_surface_sound_speed;
	 double m_sound_speed_gradient;
	 unsigned int m_water_depth;
 
 private: // State variables

  // The name of the vehicle and the collaborator
 	 string m_vehicle_name;
	 string m_collaborator;

	 // Defines if the app is fully initiated or not
	 bool m_initiated;

	 // Position, heading and speed variables for both the vehicle and it's collaborator
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
