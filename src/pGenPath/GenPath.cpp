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
  m_visit_radius = 5.0;
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
            string type = biteStringX(orig,'=');
            int value = stoi(orig);

            if (type == "x"){
              currPoint.setX(value);
              cout << "Setting x: " << value << endl;
            }

            if (type == "y"){
              currPoint.setY(value);
              cout << "Setting y: " << value << endl;
            }

            if(type == "id"){
              currPoint.setID(value);
              cout << "Setting id: " << value << endl;
            }
          }
          m_visit_points.push_back(currPoint);
        }
      }
    }

    if (msg.GetKey() == "NAV_X" && !m_initiated){
      m_x_start = msg.GetDouble();
    }

    if (msg.GetKey() == "NAV_X"){
      m_curr_x = msg.GetDouble();
    }

    if(msg.GetKey() == "NAV_Y" && !m_initiated){
      m_y_start = msg.GetDouble();
    }

    if (msg.GetKey() == "NAV_Y"){
      m_curr_y = msg.GetDouble();
    }

    if (msg.GetKey() == "GENPATH_REGENERATE"){
      m_initiated = true;
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

    vector<Point> visit_points = m_visit_points;

    while (!visit_points.empty()){
      Point closest_point = visit_points[0];
      int index = 0;
      double min_dist = calcDist(curr_point,closest_point);

      if (visit_points.size() > 1){
        for (unsigned int i = 1; i < visit_points.size(); i++){
          Point temp_point = visit_points[i];
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
      visit_points.erase(visit_points.begin() + index);
    }
    string update_str = "points=";
    update_str      += my_seglist.get_spec();
    Notify("VISIT_POINTS", update_str);  
    cout << "Sending the following update var: " << update_str << endl;
    m_initiated = false;
  }

  for (unsigned int i = 0; i < m_visit_points.size(); i++){
    Point current = m_visit_points[i];
    double dist = calcDoubleDist(current,m_curr_x,m_curr_y);
    if (dist < m_visit_radius){
      m_visit_points.erase(m_visit_points.begin() + i);
    }
  }

  if (m_visit_points.empty()){
    Notify("DEPLOY","false");
    Notify("RETURN","true");
    Notify("REFUEL","false");
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
  Register("GENPATH_REGENERATE",0);
}

double GenPath::calcDist(Point p1, Point p2)
{
  return sqrt(pow((p1.getX() - p2.getX()),2) + pow((p1.getY() - p2.getY()),2));
}

double GenPath::calcDoubleDist(Point p, double x, double y)
{
  return sqrt(pow((p.getX() - x),2) + pow((p.getY() - y),2));
}

