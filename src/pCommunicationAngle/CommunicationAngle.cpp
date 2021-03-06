/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT                                             */
/*    FILE: CommunicationAngle.cpp                          */
/*    DATE: 28.02.2019                                      */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include <math.h>
#include "CommunicationAngle.h"

using namespace std;

//---------------------------------------------------------
// Procedure: CommunicationAngle()
//   Purpose: Constructor. Creates a CommunicationAngle object and initializes //            values.  

CommunicationAngle::CommunicationAngle()
{
  // Defines the water characteristics
  m_surface_sound_speed = 1480;
  m_sound_speed_gradient = 0.016;
  m_water_depth = 6000;

  // The name of the collaborator. Initially set to an empty string, will be updated upon information
  m_collaborator = "";

  // Represents weather the app is initiated sufficiently to start calculating the acoustic path to reach the collaborator
  m_initiated = false;

  // The position variables which is used to calculate the acoustic path is initiated to arbitrary numbers which is highly unlikely to be used as values for the calculations. The depth value is impossible to reach, as iẗ́'s above the surface of the water'
  m_nav_x = 1000001;
  m_nav_y = 1000001;
  m_nav_depth = -1;
  m_col_nav_x = 1000001;
  m_col_nav_y = 1000001;
  m_col_nav_depth = -1;
}

//---------------------------------------------------------
// Procedure: CommunicationAngle()
//   Purpose: Destructor. Destroys a CommunicationAngle object
CommunicationAngle::~CommunicationAngle()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()
//   Purpose: Processes new incoming mails, and updates the member variables //            accordingly.
//   Returns: True.

bool CommunicationAngle::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
   
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;


    // If the message key is COLLABORATOR_NAME, the collaborator name membervariable is set to the string value of the message, and the app registers for the collaborator variables.
    if (msg.GetKey() == "COLLABORATOR_NAME"){
      m_collaborator = msg.GetString();
      cout << "Collaborator name is : " << m_collaborator << endl;
      RegisterCollaborator(m_collaborator);
    }

    // The follwing lines basically does the same. Updates the member variables when the message has the corresponding key value. 
    if (msg.GetKey() == "VEHICLE_NAME"){
      m_vehicle_name = msg.GetString();
    }

    if (msg.GetKey() == "NAV_X"){
      m_nav_x = msg.GetDouble();
      cout <<"My xpos is: " << m_nav_x << endl;
    }

    if (msg.GetKey() == "NAV_Y"){
      m_nav_y = msg.GetDouble();
      cout <<"My ypos is: " << m_nav_y << endl;
    }

    if (msg.GetKey() == "NAV_DEPTH"){
      m_nav_depth = msg.GetDouble();
      cout <<"My depth is: " << m_nav_depth << endl;
    }

    if(msg.GetKey() == "NAV_HEADING"){
      m_nav_heading = msg.GetDouble();
    }

    if(msg.GetKey() == "NAV_SPEED"){
      m_nav_speed = msg.GetDouble();
    }

    // If the message key is giving information about the collaborator, update the corresponding membervalue accordingly. 
    string ref;
    ref = m_collaborator + "_NAV_X";
    if(msg.GetKey() == ref){
      m_col_nav_x = msg.GetDouble();
      cout <<"Col xpos is: " << m_col_nav_x << endl;
    }

    ref = m_collaborator + "_NAV_Y";
    if(msg.GetKey() == ref){
      m_col_nav_y = msg.GetDouble();
      cout <<"Col ypos is: " << m_col_nav_y << endl;
    }

    ref = m_collaborator + "_NAV_DEPTH";
    if(msg.GetKey() == ref){
      m_col_nav_depth = msg.GetDouble();
      cout <<"Col depth is: " << m_col_nav_depth << endl;
    }

    ref = m_collaborator + "_NAV_HEADING";
    if(msg.GetKey() == ref){
      m_col_nav_heading = msg.GetDouble();
    }

    ref = m_collaborator + "_NAV_SPEED";
    if(msg.GetKey() == ref){
      m_col_nav_speed = msg.GetDouble();
    }
  }
  // If nav_depth and nav_x is updated for both our vehicle and the collaborator, set m_initiated to true
  if((m_col_nav_depth != -1) && (m_nav_depth != -1) && (m_nav_x != 1000001) && (m_col_nav_x != 1000001) && (m_nav_y != 1000001) && (m_col_nav_y != 1000001)){
    m_initiated = true;
  }
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()
//   Purpose: When the app is connected to server, RegisterVariables() //            is called.
//    Return: true.

bool CommunicationAngle::OnConnectToServer()
{
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//   Purpose: Continously calculated the Acoustic Path and Connectivity location of the vessel. If there exists an acoustic path between the vehicles, it notifies acoustic path with the fireangle and the transmissionloss. If no path exists, it either Notifies Connectivity location with NaN or the coordinate of a location where connection actually exists. 

bool CommunicationAngle::Iterate()
{
  // Only start iterating when m_initiated is set to true
  if (m_initiated == true){

    // Defines the point representing the circlecenter on the circle which arc represents the acoustic path from our vessel to the collaborator, given infinately depth.
    Point circleCenter = getCircleCenter(0,m_nav_depth);

    // Defines the fireangle to reach the collaborator from the current location
    double angle = calculateFireAngle(circleCenter);

    // Angle is set to 100 if no path exists. If this is not the case, a direct path exists
    if (angle != 100){  
      double radius = calculateCircleRadius(circleCenter);
      double loss = calculateTransmissionLoss(radius,angle);
      angle = -angle*180/M_PI;
      cout << "Angle: " << angle << endl;

      stringstream ss;
      ss << "elev_angle="<< angle <<",transmission_loss="<<loss<<",id=oliveros@mit.edu" << endl;
      Notify("ACOUSTIC_PATH",ss.str());
    }

    // Else no path exists
    else{

      // Set the connection to false
      bool connection = false;

      // The vehicle search upwards, 1 meter at the time, and checks if there is connection at that point. The height variable increases as it searches upwards.
      int height = 1;

      // While no connection between the vehicles exist, keep searching.
      while (connection == false){

        // Define the circlecenter which arc represents the acoustic transmission from the vehicle at the new hight to the collaborator
        Point circleCenter = getCircleCenter(m_nav_x,m_nav_depth - height);

        // Define the fireangle to reach the collaborator from the new point.
        double angle = calculateFireAngle(circleCenter);

        // If angle == 100, there exists no path
        if (angle == 100){

          // increase the height
          height += 1;

          // If the current position - height is less than 0, there is no point to keep searching upwards, as the vehicle is assumed to be unable to fly. Could implement search in x-direction here.
          if (m_nav_depth - height < 0){

            // Notify that there exists no path
            Notify("ACOUSTIC_PATH","NaN");
            connection = true;
          }
        }

        // If the angle is not 100, Notify CONNECTIVITY_LOCATION with the new position where communication is possible. Set connection to true.
        else{
          stringstream ss;
          double depth = m_nav_depth - height;
          ss << "x=" << m_nav_x << ",y=" <<m_nav_y << ",depth=" << depth <<",id=oliveros@mit.edu" << endl;
          Notify("CONNECTIVITY_LOCATION",ss.str());
          connection = true;
        }
      }
    }
  }
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//   Purpose: Is called once the app has started, in turns calls the //            RegisterVariables() function

bool CommunicationAngle::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string line  = *p;
      string param = tolower(biteStringX(line, '='));
      string value = line;
      
      if(param == "foo") {
        //handled
      }
      else if(param == "bar") {
        //handled
      }
    }
  }
  
  RegisterVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables()
//   Purpose: Register the app for the variables that includes information about the vehicle name, collaborator name and other information about our vehicle.

void CommunicationAngle::RegisterVariables()
{
  Register("VEHICLE_NAME", 0);
  Register("COLLABORATOR_NAME",0);
  Register("NAV_X",0);
  Register("NAV_Y",0);
  Register("NAV_DEPTH",0);
  Register("NAV_HEADING",0);
  Register("NAV_SPEED",0);
}

//---------------------------------------------------------
// Procedure: RegisterCollaborator()
//   Purpose: This is called once the collaborator name is recieved from the moosdb. Registers the app for all the variables that holds information about the collaborator vehicle.

void CommunicationAngle::RegisterCollaborator(string collaborator)
{
  string reg;
  reg = collaborator + "_NAV_X";
  Register(reg,0);

  reg = collaborator + "_NAV_Y";
  Register(reg,0);

  reg = collaborator + "_NAV_DEPTH";
  Register(reg,0);

  reg = collaborator + "_NAV_HEADING";
  Register(reg,0);

  reg = collaborator + "_NAV_SPEED";
  Register(reg,0);


}

//---------------------------------------------------------
// Procedure: calculateFireAngle()
//   Purpose: Is used to calculate the fireangle which is required for sound //            to reach a given point.
//     Input: The point which represents the middle of the circle where the //            sound travels between the two vehicles.
//   Returns: The angle required for the vehicle to hit the input point with a //            sound beam in water. Returns 100 if no such angle exists.

double CommunicationAngle::calculateFireAngle(Point circleCenter) const
  {
    // Defines the point where the vehicle is
    Point me(0,m_nav_depth);

    // Defines the line between our location and the location of the input circlecenter
    Line toCircleCenter(me,circleCenter);

    // Returns the angle from our vehicle to the circlecenter. This angel is tangential to the required fireangle.
    double angle = toCircleCenter.getGradient(); 

    // Adds pi/2 to calculate the fireangle required to hit the collaborator
    angle += M_PI/2;

    //Here we aim to calculate weather the path actually exists, given the waterdepth, height of the circle above the water and the radius of the circle representing the soundtransmission between the vehicles.
    double radius = calculateCircleRadius(circleCenter);
    double height = -m_surface_sound_speed/m_sound_speed_gradient;

    //If the height + radius is greater than the waterdepth, the circle radius will at some point be below the bottom of the sea. We have to do some logic to test if this actually matters.
    if (height + radius > m_water_depth){

      // Defines which side of the circle centers x position our and the collaborator vehicle is located.
      bool test_pos = (circleCenter.getX() - sqrt(pow(m_nav_x - m_col_nav_x,2) + pow(m_nav_y - m_col_nav_y,2)) < 0);

      // If they are defined on different sides of the x position of the circle center, set the angle value to 100, which is an invalid angle.
      if (test_pos == true){
        angle = 100;
      }
    }
    return angle;
}


//---------------------------------------------------------
// Procedure: getCircleCenter()
//   Purpose: Is called to get the circle center of which the arc represents //            the how the sound travels between two vehicles in water.
//     Input: The x and depth position of our vehicle.
//   Returns: The Point object representing the center of the circle described //            in purpose

Point CommunicationAngle::getCircleCenter(double nav_x, double nav_depth) const
{

  // Defines the points describing our position and the collaborator position and the line between.
  Point me(0,nav_depth);
  Point other(sqrt(pow(m_nav_x - m_col_nav_x,2) + pow(m_nav_y - m_col_nav_y,2)),m_col_nav_depth);
  Line between(me,other);

  // Calculates the height of the circle
  double height = -m_surface_sound_speed/m_sound_speed_gradient;

  // Calculats the circlecenter
  Point circleCenter = between.getCrossing(height);
  return circleCenter;
}


//---------------------------------------------------------
// Procedure: calculateCircleRadius()
//   Purpose: Calculated the radius of the sound transmission circle, given //            the position of our vehicle and the center of the circle.
//     Input: The point of the circleCenter
//   Returns: The circleradius as a double

double CommunicationAngle::calculateCircleRadius(Point circleCenter) const
{
  double radius = sqrt(pow(circleCenter.getX(),2) + pow(circleCenter.getY() - m_nav_depth,2));
  return radius;
}


//---------------------------------------------------------
// Procedure: calculateArcLength()
//   Purpose: Calculates the Arclength of the circle which represents the //            soundtransmission from our vehicle to the collaborator vehicle.
//     Input: The radius of the circle.
//   Returns: The length of the arc on the circle between our vehicle and the //            collaborator 

double CommunicationAngle::calculateArcLength(double radius) const
{
  double d = sqrt(pow(m_nav_x - m_col_nav_x,2) + pow(m_nav_depth - m_col_nav_depth,2));
  return radius*acos(1-pow(d,2)/(2*pow(radius,2)));
}

//---------------------------------------------------------
// Procedure: calculateTransmissionLoss
//   Purpose: Calculates the transmissionloss of a ray sent from our vehicle   //            to the collaborator, given a radius of the circle for which the //            sound speed travels along iẗ́̈́̈́'s arc and the angle the ray is //            sent. 
//     Input: The radius of the circle and the angle the sound is sent
//   Returns: The transmissionloss for the soundray. 
double CommunicationAngle::calculateTransmissionLoss(double radius,double out_angle) const
{
  // Calculates the soundspeed at the transmitter and receiver positions
  double sound_speed_transmitter = calculateSoundSpeed(m_nav_depth);
  double sound_speed_receiver = calculateSoundSpeed(m_col_nav_depth);

  // Calculates the angle from which the receiver gets receives the sound
  double in_angle = acos(cos(out_angle)*sound_speed_receiver/sound_speed_transmitter);

  // This parameter implies how much the sound spread in the water
  double d_theta = 0.001;

  // Calculates the length of the trajectory the sound travels along
  double arclength = radius*(out_angle + in_angle);

  // Calculates the length to two x- positions which defines how the sound travels

  double r_1 = radius*(sin(out_angle) + sin(arclength/radius - out_angle));
  double r_2 = radius*(sin(out_angle + d_theta) + sin(arclength/radius - out_angle - d_theta));

  // Calculates the area of pressure from the sound at the area around the receiver
  double j_area = (r_1/sin(in_angle))*((r_1 - r_2)/d_theta);

  // Calculates the pressure loss
  double p_s = (0.25*M_PI)*sqrt(abs((cos(out_angle)*sound_speed_receiver)/(sound_speed_transmitter*j_area)));
  double p_1 = 0.25*M_PI;
  
  // Returns the transmission loss
  return(-20*log10(p_s/p_1));
 }


//---------------------------------------------------------
// Procedure: calculateSoundSpeed()
//   Purpose: Calculates the soundspeed at a given depth
//     Input: The depth of interest
//   Returns: The sound speed at the given depth
double CommunicationAngle::calculateSoundSpeed(double depth) const
{
  return m_surface_sound_speed + m_sound_speed_gradient*depth;
}


