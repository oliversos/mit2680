/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: GenPath.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "GenPath.h"
#include "XYSegList.h"
#include <math.h>

using namespace std;

//---------------------------------------------------------
// Constructor

GenPath::GenPath()
{
  m_initiated = false;
}

//---------------------------------------------------------
// Destructor

GenPath::~GenPath()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool GenPath::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
   
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if(msg.GetKey() == "VISIT_POINT"){

      cout << "Fikk mail, visit point!" << endl;

      string info = msg.GetString();

      if (!(info == "firstpoint") && !m_initiated){
        if (info == "lastpoint"){
          m_initiated = true;
        }
        else{ 

          Point currPoint;
          vector<string> point_vector = parseString(info,',');

          for (unsigned int i = 0; i < 3; i++){
            string orig = point_vector[i];
            string type = biteString(orig,'=');
            int value = stoi(orig);

            if (type == "x"){
              currPoint.setX(value);
            }

            if (type == "y"){
              currPoint.setY(value);
            }

            if(type == "id"){
              currPoint.setID(value);
            }
          }
          m_visit_points.push_back(currPoint);
        }
      }
    }

    if (msg.GetKey() == "NAV_X" && !m_initiated){
      m_x_start = msg.GetDouble();
    }

    if(msg.GetKey() == "NAV_Y" && !m_initiated){
      m_y_start = msg.GetDouble();
    }
  }
  return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool GenPath::OnConnectToServer()
{
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool GenPath::Iterate()
{
  if (m_initiated){

    XYSegList my_seglist;

    Point curr_point(m_x_start,m_y_start,0); 

    while (!m_visit_points.empty()){
      Point closest_point = m_visit_points[0];
      int index = 0;
      double min_dist = calcDist(curr_point,closest_point);

      if (m_visit_points.size() > 1){
        for (unsigned int i = 1; i < m_visit_points.size(); i++){
          Point temp_point = m_visit_points[i];
          double dist = calcDist(temp_point,curr_point);
          if (dist < min_dist){
            min_dist = dist;
            closest_point = temp_point;
            index = i;
          }
        }
      }
      my_seglist.add_vertex(closest_point.getX(),closest_point.getY());
      curr_point = closest_point;
      m_visit_points.erase(m_visit_points.begin() + index);
    }
    string update_str = "points = ";
    update_str      += my_seglist.get_spec();
    Notify("UPDATES_VAR", update_str);  // UPDATES_VAR depends on your config
    cout << "Sending out updates var!     " << update_str << endl;
    Notify("DEPLOY","true");
  }
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool GenPath::OnStartUp()
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

void GenPath::RegisterVariables()
{
  Register("VISIT_POINT",0);
  Register("NAV_X",0);
  Register("NAV_Y",0);
}

double GenPath::calcDist(Point p1, Point p2)
{
  return sqrt(pow((p1.getX() - p2.getX()),2) + pow((p1.getY() - p2.getY()),2));
}

