/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: PointAssign.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <algorithm>
#include <sstream>
#include "MBUtils.h"
#include "PointAssign.h"
#include "Point.h"
#include "XYPoint.h"

using namespace std;

//---------------------------------------------------------
// Constructor

PointAssign::PointAssign()
{
  m_assign_by_region = true;
  m_send_points = false;
}

//---------------------------------------------------------
// Destructor

PointAssign::~PointAssign()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool PointAssign::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
   
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    if (msg.GetKey() == "VISIT_POINT"){
      string point_values = msg.GetString();

      if (!(point_values == "firstpoint")){
        if (point_values == "lastpoint"){
          m_send_points = true;
        }
        else{
          vector<string> point_vector = parseString(point_values, ',');
          Point currPoint;
          int curr_id = 0;

          for(unsigned int i = 0; i < 3; i++){
            string orig = point_vector[i];
            string type = biteStringX(orig,'=');
            int value = stoi(orig);
            if (type == "x"){
              currPoint.setX(value);
            }
            if (type == "y"){
              currPoint.setY(value);
            }
            if (type == "id"){
              curr_id = value;
              currPoint.setID(value);
            }
          }
          m_visit_points[curr_id] = currPoint;
        }
      }
    }

    if (msg.GetKey() == "NODE_REPORT"){
    	if (m_vname.size() != 2){
    		string report = msg.GetString();

    		vector<string> report_list = parseString(report,',');

    		for (unsigned int i = 0; i < report_list.size() ; i++){
    			string def = biteStringX(report_list[i],'=');
    			if (def == "NAME"){
            string name = report_list[i];
            for(int i = 0; i < name.length(); i++) {
              name.at(i) = toupper(name.at(i));
            }   
    				bool exists = find(m_vname.begin(), m_vname.end(), name) != m_vname.end();
    				if (!exists){
    					m_vname.push_back(name);
              cout << "Added name: " << name << endl;
              if (m_vname.size() == 2){
                Notify("UTS_PAUSE","false");
              }
    				}
    			}
    		}
    	}
    }
  }
  return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool PointAssign::OnConnectToServer()
{
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PointAssign::Iterate()
{
	if (m_send_points && !m_visit_points.empty()){
		stringstream n1, n2;
		n1 << "VISIT_POINT_" << m_vname[0] << endl;
		n2 << "VISIT_POINT_" << m_vname[1] << endl;
		string name1 = n1.str();
		string name2 = n2.str();	

		if (!m_assign_by_region){
			assignByAlternate(name1,name2);
		}
		else{
			assignByRegion(name1,name2);
		}
		m_send_points = false;
	}
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PointAssign::OnStartUp()
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

void PointAssign::RegisterVariables()
{
  Register("VISIT_POINT",0);
  Register("NODE_REPORT",0);
}



void PointAssign::assignByAlternate(string name1, string name2) 
{
	Notify(name1,"firstpoint");
  Notify(name2,"firstpoint");
  cout << "Printing names:" << endl;
  cout << "Name 1:" << name1 << endl;
  cout << "Name 2:" << name2 << endl;
	for (unsigned int i = 0; i < m_visit_points.size(); i ++){
		if (m_visit_points.count(i)){
			Point current = m_visit_points[i];
			stringstream message;
			message << "x=" << current.getX() << ", y=" << current.getY() << ", id=" << current.getID() << endl;
      if (i % 2 == 0){
        Notify(name1,message.str());
        string color = "red";
        postViewPoint(current.getX(),current.getY(),current.getID(),color);
      }
      else{
        Notify(name2,message.str());  
        string color = "blue";
        postViewPoint(current.getX(),current.getY(),current.getID(),color);
      }
		}
	}
	Notify(name1,"lastpoint");
  Notify(name2,"lastpoint");
}

void PointAssign::assignByRegion(string name1, string name2) 
{
	bool success1 = Notify(name1,"firstpoint");
	bool success2 = Notify(name2,"firstpoint");

	for (unsigned int i = 0; i < m_visit_points.size(); i++){

		if (m_visit_points.count(i)){

			Point current = m_visit_points[i];
			stringstream message;
			message << "x=" << current.getX() << ", y=" << current.getY() << ", id=" << current.getID() << endl;
			if (current.getX() >= 87){
				Notify(name1,message.str());
        cout << "Notifying: " << name1 << " with: " << message.str() << endl;
        string color = "red";
        postViewPoint(current.getX(),current.getY(),current.getID(),color);
			}
			else{
				Notify(name2,message.str());
        string color = "blue";
        postViewPoint(current.getX(),current.getY(),current.getID(),color);
			}
		}
	}
	Notify(name1,"lastpoint");
	Notify(name2,"lastpoint");
}


void PointAssign::postViewPoint(double x, double y, int label, string color)
{
  XYPoint point(x, y);
  stringstream ss;
  ss << label;
  point.set_label(ss.str());

  point.set_color("vertex", color);
  point.set_param("vertex_size", "2");

  string spec = point.get_spec();

  Notify("VIEW_POINT", spec);
}
