/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT Cambridge MA                                */
/*    FILE: Odometry.cpp                                    */
/*    DATE: 14.02.2019                                      */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "Odometry.h"
#include <cmath>

using namespace std;

//---------------------------------------------------------
// Procedure: Odometry()
// Purpose: Constructor. Creates an Odometry object and initaitializes values. 

Odometry::Odometry()
{
  m_first_reading_x = false;
  m_first_reading_y = false;
  m_previous_x = 0;
  m_previous_y = 0;
  m_total_distance = 0;
  m_current_x = 0;
  m_current_y = 0;
}

//---------------------------------------------------------
// Procedure: ~Odometry()
// Purpose: Destructor. Deletes an Odometry object.

Odometry::~Odometry()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()
//   Purpose: Processes new incoming mails. Updates the AppCastingMOOSApp with 
//            new message, and updates m_current_x and m_current_y on the //            corresponding incoming mails.
//   Returns: true.


bool Odometry::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
   
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    //If a message with key NAV_X is recieved, update the current x- position.
    //If it is the first message read by the Odometry app, update m_previous_x 
    //to describe the starting position. Set m_first_reading_x to true, //indicating that the startingposition is set. 

    if (msg.GetKey() == "NAV_X"){
      m_current_x = msg.GetDouble();
      if (m_first_reading_x == false){
        m_previous_x = msg.GetDouble();
        m_first_reading_x = true;
      }
    }

    if (msg.GetKey() == "NAV_Y"){
      m_current_y = msg.GetDouble();
      if(m_first_reading_y == false){
        m_previous_y = msg.GetDouble();
        m_first_reading_y = true;
      }
    }
  }
  return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()
//   Purpose: When the app is connected to server, RegisterVariables() //            is called.
//    Return: true. 
bool Odometry::OnConnectToServer()
{	
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//   Purpose: Updates the total distance travelled and sets m_previous_x to //            m_current_x and m_previous_y to m_current_y. Also uodates the   //            AppCasting and posting an appcast report.
//   Returns: true.

bool Odometry::Iterate()
{
  //Call AppCast function iterate()
  AppCastingMOOSApp::Iterate();

  // Only start iterating when m_first_reading_x = true or m_furst_reading_y = true
  if ((m_first_reading_x == true) || (m_first_reading_y == true)){

    //Calculate the difference between current position in x and y direction.
    double x_dist = m_current_x - m_previous_x;
    double y_dist = m_current_y - m_previous_y;

    //Add the distance to the total distance using pythagoras.
    m_total_distance += sqrt(pow(x_dist,2) + pow(y_dist,2));
    m_previous_x = m_current_x;
    m_previous_y = m_current_y;

    //Send the total distance back to MOOSDB, with the msg key ODOMETRY_DIST
    Notify("ODOMETRY_DIST",m_total_distance);
  }

  //Post the report to the AppCast
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//   Purpose: Invoces the OnStartUp() function in the AppCastingMOOSApp.
//   Returns: true.

bool Odometry::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();
  m_MissionReader.EnableVerbatimQuoting(false);
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables()
//   Purpose: Registers the app for NAV_X and NAV_Y to get position updates.   //            Registers for APPCAST_REQ in order to set up the appcasting //            properly. Also registers variables in the ApppCastingMOOSApp.

void Odometry::RegisterVariables()
{
  //Perform Register Variables procedure on the AppCast
  AppCastingMOOSApp::RegisterVariables();

  //Register the app for the following three variables in the MOOSDB. 
  //The 0 means that it checks all messages sent from the MOOSDB.
  Register("NAV_X", 0);
  Register("NAV_Y", 0);
  Register("APPCAST_REQ",0);
}

//---------------------------------------------------------
// Procedure: buildReport()
//   Purpose: Prints a report message to the AppCasting. 
//   Returns: true.

bool Odometry::buildReport()
{ 
  m_msgs << "Total distance travelled: " << m_total_distance << endl;
  return (true);
}

