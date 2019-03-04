/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: CommunicationAngle.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include <math.h>
#include "CommunicationAngle.h"

using namespace std;

//---------------------------------------------------------
// Constructor

CommunicationAngle::CommunicationAngle()
{
  m_surface_sound_speed = 1480;
  m_sound_speed_gradient = 0.016;
  m_water_depth = 6000;
  m_collaborator = "";
}

//---------------------------------------------------------
// Destructor

CommunicationAngle::~CommunicationAngle()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool CommunicationAngle::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
   
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    string key = msg.GetKey();

    if (key == "COLLABORATOR_NAME" && msg.IsString()){
      m_collaborator = msg.GetString();
      RegisterCollaborator(m_collaborator);
    }

    if (key == "VEHICLE_NAME" && msg.IsString()){
      m_vehicle_name = msg.GetString();
    }

    if (key == "NAV_X"){
      m_nav_x = msg.GetDouble();
    }

    if (key == "NAV_Y"){
      m_nav_y = msg.GetDouble();
    }

    if (key == "NAV_DEPTH"){
      m_nav_depth = msg.GetDouble();
    }

    if(key == "NAV_HEADING"){
      m_nav_heading = msg.GetDouble();
    }

    if(key == "NAV_SPEED"){
      m_nav_speed = msg.GetDouble();
    }

    string ref;
    ref = m_collaborator + "_NAV_X";
    if(key == ref){
      m_col_nav_x = msg.GetDouble();
    }

    ref = m_collaborator + "_NAV_Y";
    if(key == ref){
      m_col_nav_y = msg.GetDouble();
    }

    ref = m_collaborator + "_NAV_DEPTH";
    if(key == ref){
      m_col_nav_depth = msg.GetDouble();
    }

    ref = m_collaborator + "_NAV_HEADING";
    if(key == ref){
      m_col_nav_heading = msg.GetDouble();
    }

    ref = m_collaborator + "_NAV_SPEED";
    if(key == ref){
      m_col_nav_speed = msg.GetDouble();
    }
  }



#if 0 // Keep these around just for template
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool CommunicationAngle::OnConnectToServer()
{
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool CommunicationAngle::Iterate()
{
  Point circleCenter = getCircleCenter();
  double angle = calculateFireAngle(circleCenter);
  if (angle != 100){
    double radius = calculateCircleRadius(circleCenter);
    Notify("ACOUSTIC_PATH","elev_angle");
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

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
// Procedure: RegisterVariables

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
// Procedure: RegisterCollaborator

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
// Procedure: checkIfExistingPath

double CommunicationAngle::calculateFireAngle(Point circleCenter) const
  {
    Point me(m_nav_x,m_nav_y);
    Line toCircleCenter(me,circleCenter);
    double angle = toCircleCenter.getGradient(); 

    if (m_col_nav_x > m_nav_x){
      angle += M_PI/2;
    }
    else{
      angle -= M_PI/2;
    }

    double radius = calculateCircleRadius(circleCenter);
    double height = -m_surface_sound_speed/m_sound_speed_gradient;
    if (height + radius > m_water_depth){

      int test_mypos = ((m_nav_x < circleCenter.getX()) - (m_nav_x > circleCenter.getX()));
      int test_otherpos = ((m_col_nav_x < circleCenter.getX()) - (m_col_nav_x > circleCenter.getX()));
      if (test_mypos != test_otherpos){
        angle = 100;
      }
    }
    return angle;
}


Point CommunicationAngle::getCircleCenter() const
{
  Point me(m_nav_x,m_nav_y);
  Point other(m_col_nav_x,m_col_nav_y);
  Line between(me,other);
  double height = -m_surface_sound_speed/m_sound_speed_gradient;
  Point circleCenter = between.getCrossing(height);
  return circleCenter;
}

double CommunicationAngle::calculateCircleRadius(Point circleCenter) const
{
  double radius = sqrt(pow(circleCenter.getX() - m_nav_x,2) + pow(circleCenter.getY() - m_nav_y,2));
  return radius;
}

double CommunicationAngle::calculateArcLength(double radius) const
{
  double d = sqrt(pow(m_nav_x - m_col_nav_x,2) + pow(m_nav_y - m_col_nav_y,2));
  return radius*acos(1-pow(d,2)/(2*pow(radius,2)));
}

double CommunicationAngle::calculateTransmissionLoss(double archlength) const
{
  return 0;
}


