/*****************************************************************/
/*    NAME: Oliver Os and Simen Sem Oevereng                     */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: HazardMgr.cpp                                        */
/*    DATE: Apr 12  2019                                         */
/*    This app expands the functionality of the MOOS-IvP app     */
/*    uFldHazardMgr by adjusting it according to specification   */
/*    in lab 12 and 13 of the 2.680 class at MIT, Spring 2019    */
/*****************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "HazardMgr.h"
#include "XYFormatUtilsHazard.h"
#include "XYFormatUtilsPoly.h"
#include "ACTable.h"
#include <time.h>
#include <sstream>

#include <string>
#include <algorithm> // for std::sort()

using namespace std;

const bool debug = true;

//---------------------------------------------------------
// Constructor

HazardMgr::HazardMgr()
{
  // Config variables
  m_swath_width_desired = 25;
  m_pd_desired          = 0.9;

  // State Variables 
  m_sensor_config_requested = false;
  m_sensor_config_set   = false;
  m_swath_width_granted = 0;
  m_pd_granted          = 0;

  m_sensor_config_reqs = 0;
  m_sensor_config_acks = 0;
  m_sensor_report_reqs = 0;
  m_detection_reports  = 0;
  m_send_report = true;

  m_summary_reports = 0;

  // Added
  m_name = "";
  m_msg = ""; // Added when writing documentation as it wast initiated. Might cause problems later if Oliver has assumed that it was uninitiated before?
  m_last_msg_sent = 0;

  m_max_time               = 0;
  m_mission_start_time     = 0; 
  m_penalty.missed_hazard  = 0;
  m_penalty.nonopt_hazard  = 0;
  m_penalty.false_alarm    = 0;
  m_penalty.max_time_over  = 0;
  m_penalty.max_time_rate  = 0;
  m_pclass = 1;
  m_initiated = false;

  m_classification_msg = "";

  m_search_region_str = "";
  m_pclass = 1;

}

//---------------------------------------------------------
// Procedure: OnNewMail
bool HazardMgr::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key   = msg.GetKey();
    string sval  = msg.GetString(); 
    
    if(key == "UHZ_CONFIG_ACK"){
      handleMailSensorConfigAck(sval);
    }

    else if(key == "UHZ_OPTIONS_SUMMARY"){
      handleMailSensorOptionsSummary(sval);
    }

    else if(key == "UHZ_DETECTION_REPORT"){
      handleMailDetectionReport(sval);
    }

    else if(key == "HAZARDSET_REQUEST"){
      handleMailReportRequest();
    }

    else if(key == "UHZ_MISSION_PARAMS"){
      handleMailMissionParams(sval);
    }

    else if(key == "NODE_REPORT_LOCAL"){
      if (m_name == ""){
        handleAddName(sval);
      }
    }

    else if (key == "HAZARD_REPORT"){ 
      Notify("GOT_HAZREP",sval);
      handleHazardReport(sval);
    }

    else if(key == "UHZ_HAZARD_REPORT"){
      handleClassificationReport(sval);
    }

    else if(key == "RETURN"){
      //decideHazards();
    }
    else{
      reportRunWarning("Unhandled Mail: " + key);
    }
  }
  
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer
bool HazardMgr::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second
bool HazardMgr::Iterate()
{
  AppCastingMOOSApp::Iterate();

  if(!m_sensor_config_requested)
    postSensorConfigRequest();

  if(m_sensor_config_set)
    postSensorInfoRequest();


  if ((MOOSTime() - m_last_msg_sent > 65) && m_send_report){
    postHazardMessage();
  }

  if (m_initiated == false){
    // This is the total message to be sent to the )collaborator. This is done to initialize the HAZARD_REPORT variable as a string, since there was problems with it never being published as a string since it was set to be a double somewhere else.
    string sending_msg;
    sending_msg += "src_node=" + m_name;
    sending_msg += ",dest_node=all";
    sending_msg += ",var_name=HAZARD_REPORT";
    sending_msg += ",string_val=heisann";
    Notify("NODE_MESSAGE_LOCAL",sending_msg);
    m_initiated = true;
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open
bool HazardMgr::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(true);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if((param == "swath_width") && isNumber(value)) {
      m_swath_width_desired = atof(value.c_str());
      handled = true;
    }
    else if(((param == "sensor_pd") || (param == "pd")) && isNumber(value)) {
      m_pd_desired = atof(value.c_str());
      handled = true;
    }
    else if(param == "report_name") {
      value = stripQuotes(value);
      m_report_name = value;
      handled = true;
    }
    else if(param == "region") {
      XYPolygon poly = string2Poly(value);
      if(poly.is_convex())
        m_search_region = poly;
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }
  m_hazard_set.setSource(m_host_community);
  m_hazard_set.setName(m_report_name);
  m_hazard_set.setRegion(m_search_region);
  
  registerVariables();  
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables
void HazardMgr::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("UHZ_DETECTION_REPORT", 0);
  Register("UHZ_CONFIG_ACK", 0);
  Register("UHZ_OPTIONS_SUMMARY", 0);
  Register("UHZ_MISSION_PARAMS", 0);
  Register("HAZARDSET_REQUEST", 0);
  Register("NODE_REPORT_LOCAL",0);
  Register("HAZARD_REPORT",0);
  Register("UHZ_HAZARD_REPORT",0);
  Register("RETURN",0);
}

//---------------------------------------------------------
// Procedure: postSensorConfigRequest
void HazardMgr::postSensorConfigRequest()
{
  string request = "vname=" + m_host_community;
  
  request += ",width=" + doubleToStringX(m_swath_width_desired,2);
  request += ",pd="    + doubleToStringX(m_pd_desired,2);

  m_sensor_config_requested = true;
  m_sensor_config_reqs++;
  Notify("UHZ_CONFIG_REQUEST", request);
}

//---------------------------------------------------------
// Procedure: postSensorInfoRequest
void HazardMgr::postSensorInfoRequest()
{
  string request = "vname=" + m_host_community;

  m_sensor_report_reqs++;
  Notify("UHZ_SENSOR_REQUEST", request);
}

//---------------------------------------------------------
// Procedure: handleMailSensorConfigAck
// Edits:     m_pclass
bool HazardMgr::handleMailSensorConfigAck(string str)
{
  // Expected ack parameters:
  string vname, width, pd, pfa, pclass;
  
  // Parse and handle ack message components
  bool   valid_msg = true;
  string original_msg = str;

  vector<string> svector = parseString(str, ',');
  unsigned int i, vsize = svector.size();
  for(i=0; i<vsize; i++) {
    string param = biteStringX(svector[i], '=');
    string value = svector[i];

    // TODO: Store pclass as member variable
    if(param == "vname")
      vname = value;
    else if(param == "pd")
      pd = value;
    else if(param == "width")
      width = value;
    else if(param == "pfa")
      pfa = value;
    else if(param == "pclass"){
      pclass = value;
      m_pclass = stod(value);
    }
    else
      valid_msg = false;       

  }


  if((vname=="")||(width=="")||(pd=="")||(pfa=="")||(pclass==""))
    valid_msg = false;
  
  if(!valid_msg)
    reportRunWarning("Unhandled Sensor Config Ack:" + original_msg);

  if(valid_msg) {
    m_sensor_config_set = true;
    m_sensor_config_acks++;
    m_swath_width_granted = atof(width.c_str());
    m_pd_granted = atof(pd.c_str());
  }

  return(valid_msg);
}

//---------------------------------------------------------
// Procedure: handleMailDetectionReport
//      Note: The detection report should look something like:
//            UHZ_DETECTION_REPORT = vname=betty,x=51,y=11.3,label=12 
bool HazardMgr::handleMailDetectionReport(string str)
{
  m_detection_reports++;

  XYHazard new_hazard = string2Hazard(str);
  new_hazard.setType("hazard");

  string hazlabel = new_hazard.getLabel();
  
  if(hazlabel == "") {
    reportRunWarning("Detection report received for hazard w/out label");
    return(false);
  }

  int ix = m_hazard_set.findHazard(hazlabel);
  if(ix == -1)
    m_hazard_set.addHazard(new_hazard);
  else
    m_hazard_set.setHazard(ix, new_hazard);

  string event = "New Detection, label=" + new_hazard.getLabel();
  event += ", x=" + doubleToString(new_hazard.getX(),1);
  event += ", y=" + doubleToString(new_hazard.getY(),1);

  reportEvent(event);

  string req = "vname=" + m_host_community + ",label=" + hazlabel;
  Notify("UHZ_CLASSIFY_REQUEST", req);

  return(true);
}


//---------------------------------------------------------
// Procedure: handleMailReportRequest
void HazardMgr::handleMailReportRequest()
{
  m_summary_reports++;

  m_hazard_set.findMinXPath(20);
  //unsigned int count    = m_hazard_set.findMinXPath(20);

  // original call
  string summary_report = m_hazard_set.getSpec("final_report");

  Notify("TESTREP0",summary_report);

  // New call
  summary_report = decideHazards();
  Notify("TESTREP1",decideHazards());

  // For debugging
  string str = sortedClassificationsToString();
  str += "# - Detections: " + to_string(m_hazard_set.size());
  str += "# - Classifications: " + to_string(m_classifications.size());

  Notify("TESTREP3",str);

  Notify("HAZARDSET_REPORT", summary_report);
}


//---------------------------------------------------------
// Procedure: handleMailMissionParams
// Purpose:   Store all parameters for the mission. Update member variables
//            and publish search region to a waypoint update MOOS variable to 
//            decide search region
//   Example: UHZ_MISSION_PARAMS = 
//                       penalty_missed_hazard=100,               
//                       penalty_nonopt_hazard=55,                
//                       penalty_false_alarm=35,                  
//                       penalty_max_time_over=200,               
//                       penalty_max_time_rate=0.45,              
//                       transit_path_width=25,                           
//                       search_region = pts={-150,-75:-150,-50:40,-50:40,-75}
void HazardMgr::handleMailMissionParams(string str)
{
  vector<string> svector = parseStringZ(str, ',', "{");
  unsigned int i, vsize = svector.size();
  for(i=0; i<vsize; i++) {
    string param = biteStringX(svector[i], '=');
    string value = svector[i];
    
    if(param == "penalty_missed_hazard")
      m_penalty.missed_hazard = stod(value);

    if(param == "penalty_nonopt_hazard")
      m_penalty.nonopt_hazard = stod(value);

    if(param == "penalty_false_alarm")
      m_penalty.false_alarm = stod(value);

    if(param == "penalty_max_time_over")
      m_penalty.max_time_over = stod(value);

    if(param == "penalty_max_time_rate")
      m_penalty.max_time_rate = stod(value);

    if(param == "max_time")
      m_max_time = stod(value);

    if(param == "transit_path_width")
      m_transit_path_width = stod(value);

    if(param == "search_region")
      m_search_region_str = value;
  }
}

//---------------------------------------------------------
// Procedure: handleClassificationReport
// Purpose:   Deals with incoming classification reports from the MOOS 
//            variable UHZ_HAZARD_REPORT, sent from another app after the 
//            vehicle has requested classification
//            Example str: "label=12,type=benign"
void HazardMgr::handleClassificationReport(string str){

  vector<string> svector = parseString(str, ',');
  int lab = -1;
  string haz_str = "";
  bool haz = false;

  unsigned int vsize = svector.size();
  for(unsigned int i=0; i<vsize; i++) {
    string param = biteStringX(svector[i], '=');
    string value = svector[i];

    if(param == "label"){
      lab = stoi(value);
    }

    if(param == "type"){
      haz_str = value;
    }
  }

  if(lab != -1 && !haz_str.empty()){
    if(haz_str == "benign"){
      haz = false;
    } else{
      haz = true;
    }
    m_classification_msg += "l=";
    m_classification_msg += to_string(lab);
    m_classification_msg += "_p=";
    if (haz)
      m_classification_msg += "0.7";
    else
      m_classification_msg += "0.3";
    m_classification_msg += "#";
    updateNewClassification(lab,m_pclass,haz);
  } // if we have got a labal and a type
} // for all elements in current string
  // func


//------------------------------------------------------------
// Procedure: buildReport()
bool HazardMgr::buildReport() 
{
  // TODO: 
  if(!debug){
    m_msgs << "Config Requested:"                                  << endl;
    m_msgs << "    swath_width_desired: " << m_swath_width_desired << endl;
    m_msgs << "             pd_desired: " << m_pd_desired          << endl;
    m_msgs << "   config requests sent: " << m_sensor_config_reqs  << endl;
    m_msgs << "                  acked: " << m_sensor_config_acks  << endl;
    m_msgs << "------------------------ "                          << endl;
    m_msgs << "Config Result:"                                     << endl;
    m_msgs << "       config confirmed: " << boolToString(m_sensor_config_set) << endl;
    m_msgs << "    swath_width_granted: " << m_swath_width_granted << endl;
    m_msgs << "             pd_granted: " << m_pd_granted          << endl << endl;
    m_msgs << "--------------------------------------------" << endl << endl;

    m_msgs << "               sensor requests: " << m_sensor_report_reqs << endl;
    m_msgs << "             detection reports: " << m_detection_reports  << endl << endl; 

    m_msgs << "   Hazardset Reports Requested: " << m_summary_reports << endl;
    m_msgs << "      Hazardset Reports Posted: " << m_summary_reports << endl;
    m_msgs << "                   Report Name: " << m_report_name << endl;
  }

  return(true);
}


//---------------------------------------------------------
// Procedure: handleAddName(string)
// PURPOSE:   finds the local vehicle's name for use in message sending
// @param     str: a string of the name
// @edits     m_name: member string with name
// @return    no returns
void HazardMgr::handleAddName(string str)
{
  vector<string> report_list = parseString(str,',');

  for (unsigned int i = 0; i < report_list.size() ; i++){
    string def = biteStringX(report_list[i],'=');
    if (def == "NAME"){
      string name = report_list[i];
      for(int i = 0; i < name.length(); i++) {
        name.at(i) = toupper(name.at(i));
      }   
      m_name = name;

      UnRegister("NODE_REPORT_LOCAL");
    }
  }
}

//---------------------------------------------------------
// Procedure: postHazardMessage()
// PURPOSE:   To update the output report string that is sent to shoreside and 
//            other vehicles on which hazards that has been found by LOCAL
// @param     no inputs
// @edits     m_msg: member string for all reports
// @return    no returns
void HazardMgr::postHazardMessage()
{

  string sending_msg; //This is the total message to be sent to the )collaborator
  sending_msg += "src_node=" + m_name;
  sending_msg += ",dest_node=all";
  sending_msg += ",var_name=HAZARD_REPORT";
  sending_msg += ",string_val=";

  string haz_msg; // Message to be sent as haz_reg


  string added_msg = m_classification_msg;
  m_classification_msg = "";
  haz_msg += added_msg;


  XYHazardSet unsent_hazards; // all hazards that has not been sent yet
  int sending = 0;

  // Get each hazard, and check to see if it is already sent
  // If not, then add to the set of unsent hazards
  for (unsigned int i = 0; i < m_hazard_set.size() ; i++){

    XYHazard current = m_hazard_set.getHazard(i);
    string label = current.getLabel();

    if (!(m_hazard_sent.hasHazard(label)) ){
      unsent_hazards.addHazard(current);
      sending++;
    }
  } 

  // If we found unsent hazards, add it to output string
  // TODO: Handle case where the output is too long for constraint of 100
  if (sending > 0){

    // For all unsent, add specs to out member message string
    // After adding to string, add it to set of sent hazards
    for (unsigned int i = 0; i < sending; i++){
      XYHazard current_unsent = unsent_hazards.getHazard(i);
      string add_msg = current_unsent.getSpec("");

      vector<string> info_vector = parseString(add_msg, ',');
      string info;
      for(unsigned int i = 0; i < 4; i++){ 
        string orig = info_vector[i];
        string type = biteStringX(orig,'=');

        if (type == "x"){
          info += "x=";
          info += orig;
          info += "_";
        }
        if (type == "y"){
          info += "y=";
          info += orig;
          info += "_";
        }
        if (type == "label"){
          info += "l=";
          info += orig;
          info += "_";
        }
        if (type == "type"){
          info += "t=";
          if (orig == "hazard"){
            orig = "h";
          }
          else{
            orig = "b";
          }
          info += orig;
        }
      }
      // TODO remove false
      // if message length is longer than 100 elements, tell by M 
      // here, test must be changed for length of string_val
      if (haz_msg.length() + info.length() + 3 > 100) {
        break;
      }

      info += "#";
      haz_msg += info;
      m_hazard_sent.addHazard(current_unsent);
      sending --;
    }
    sending_msg += haz_msg; 
    // "\"" + haz_msg + "\"";
  }

  // TODO: remove false
  // TODO: this F has to be removed later in a string reading function, if m_msg is kept as member OR could be solved by Notify("NODE_MESSAGE_LOCAL",m_msg + "M");
  if (haz_msg != ""){
    Notify("HAZ_MSSG",haz_msg);
    Notify("NODE_MESSAGE_LOCAL",sending_msg);
  }
  m_last_msg_sent = MOOSTime();

  if(debug)
    cout << "POSTED: " << sending_msg << endl;
}

//---------------------------------------------------------
// Procedure: handleHazardReport(string)
// PURPOSE:   Handles an incoming hazard report
// @param     str: a string with a hazard report
// @edits     m_hazard_set, m_hazard_sent
// @return    no returns
void HazardMgr::handleHazardReport(string str)
{
  Notify("ALL_HAZ",str);
  vector<string> received_hazards = parseString(str,"#");
  Notify("SIZE",received_hazards.size());

  for (unsigned int i = 0 ; i < received_hazards.size() ; i++){
    string current = received_hazards[i];
    Notify("MZZG",current);


    // TODO: maybe we have to control for "" here as well, since # might be put on the end of a message (I can see that such a situation should never occur in postMessage, but seems like god programmeringsskikk)

    string info;
    vector <string> haz_info = parseString(current,'_');

    int length = haz_info.size();
    Notify("LENGTH4",length);
    if (length == 4){
      for(unsigned int i = 0; i < length; i++){ 

        string orig = haz_info[i];
        string type = biteStringX(orig,'=');

        if (type == "x"){
          info += "x=";
          info += orig;
          info += ",";
        }
        if (type == "y"){
          info += "y=";
          info += orig;
          info += ",";
        }
        if (type == "l"){
          info += "label=";
          info += orig;
          info += ",";
        }
        if (type == "t"){
          info += "type=";
          if (orig == "h"){
            orig = "hazard";
          }
          else{
            orig = "benign";
          }
          info += orig;
        }


        XYHazard new_hazard = string2Hazard(info);
        string label = new_hazard.getLabel();
        Notify("ADDING_HAZARD",label);

        // If the hazard is not in the LOCAL set of hazards, then we can add it to 
        if (!m_hazard_set.hasHazard(label)){
          m_hazard_set.addHazard(new_hazard);
          m_hazard_sent.addHazard(new_hazard);
        }
      }
    }

    if (length == 2){
      Notify("LENGTH2",length);
      int id = -1;
      double p;
      bool t;
      for (unsigned int i = 0 ; i < length ; i++){
        string orig = haz_info[i];
        string type = biteStringX(orig,'=');
        if (type == "l"){
          id = stoi(orig);
        }
        if (type == "p"){
          p = stod(orig);
        }
      }
      updateIncomingClassification(id,p);
    }
      // All hazards has been received - stop waiting
      // TODO: where is such a variable handled?
  }
}

/*
Classification HazardMgr::findClassification(int id)
{
  for (unsigned int i = 0; i < m_classifications.size(); i++){
    if (m_classifications[i].getLabel() == id){
      return m_classifications[i];
    }
  }
  Classification c();
  c.setLabel(-1);
  return c;
}*/

//---------------------------------------------------------
// Procedure: setClassificationStatus
// PURPOSE:   Sets status on a Classification object regarding whether it has 
//            been shared to the other vehicle or not
// @param     id: label number - status: true if it has been sent
// @edits     edits Classification objects in m_classification vector
// @return    no returns
void HazardMgr::setClassificationStatus(int id,bool status)
{
  for (unsigned int i = 0; i < m_classifications.size(); i++){
    if (m_classifications[i].getLabel() == id){
      m_classifications[i].setShared(status);
    }
  }
}

//---------------------------------------------------------
// Procedure: classificationExist
// PURPOSE:   Checks if a Classification object exists in the member vector of
//            classification objects, m_classifications
// @param     id: label number
// @edits     no edits
// @return    true if Classification object with label id exists
bool HazardMgr::classificationExist(int id)
{
  for (unsigned int i = 0; i < m_classifications.size(); i++){
    if (m_classifications[i].getLabel() == id){
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------
// Procedure: updateNewClassification
// PURPOSE:   Updates probability for a Classification object given a new 
//            classification report the THIS vehicle has requested
// @param     id: label number, prob: probability of id being hazard, 
//            newClass: 
// @edits     Classification objects in m_classification vector
// @return    no returns
void HazardMgr::updateNewClassification(int id, double prob, bool newClass)
{

  if (classificationExist(id)){
    for (unsigned int i = 0; i < m_classifications.size(); i++){
      if (m_classifications[i].getLabel() == id){
        Notify("NEW_PROB",prob);
        Notify("NEW_CLASS",newClass);        
        m_classifications[i].calculateProb(newClass,prob);
        break;
      }
    }
  }
  else{
    if (newClass){
      Classification newClassification(id,prob,false);
      newClassification.setLastReport(newClass);
      m_classifications.push_back(newClassification);
    }
    else{
      double a = 1.0 - prob;
      Classification newClassification(id,a,false);
      newClassification.setLastReport(newClass);
      m_classifications.push_back(newClassification);
    }
  }
}


//---------------------------------------------------------
// Procedure: updateIncomingClassification
// PURPOSE:   Updates probability for a Classification object given a new 
//            classification report FROM THE OTHER vehicle. Adjusts probability
//            if object already has a classificaion; adds if not
// @param     id: label number, prob: probability of id being hazard
// @edits     Classification objects in m_classification vector
// @return    no returns
void HazardMgr::updateIncomingClassification(int id, double prob)
{
  Notify("UPDATE_INCOMING",m_classifications.size());
  if (classificationExist(id)){
    Notify("CLASSIFICATION_EXIST","true");
    for (unsigned int i = 0; i < m_classifications.size(); i++){
      if (m_classifications[i].getLabel() == id){
        Notify("BEFORE_INCOMING",m_classifications[i].getProb());
        m_classifications[i].calculateProb(true,prob);
        Notify("AFTER_INCOMING",m_classifications[i].getProb());
        break;
      }
    }
  }
  else{
    Classification newClassification(id,prob,true);
    m_classifications.push_back(newClassification);
    Notify("GOT_NEW_CLASSIFICATION",id);
  }
}

/*
Classification HazardMgr::findSentClassification(int id)
{
  for (unsigned int i = 0; i < m_sent_classifications.size(); i++)
  {
    if (m_sent_classifications[i].getLabel() == id){
      return m_sent_classifications[i];
    }
  }
  Classification c();
  c.setLabel(-1);
  return c;
}

bool HazardMgr::isUnsentClassification(int id)
{

}*/



//---------------------------------------------------------
// Procedure: sortClassifications
// PURPOSE:   Sorts m_classifications based on Classification objects' prob.
// @param     dir: direction of sort. true means from smallest to largest
// @edits     Order of Classification objects in m_classifications
// @return    no returns
void HazardMgr::sortClassifications(bool dir = true){
  // TODO: case where dir = false, > must be overloaded, and ::greater must be sent to std::sort. We don't case about that now.
  std::sort(m_classifications.begin(),m_classifications.end());
}

//---------------------------------------------------------
// Procedure: sortedClassificationsToString
// PURPOSE:   Creates string of classifications and their probabilites.
// @param     No inputs
// @edits     Sorts m_classifications.
// @return    A string of all classifications and their P(Hazard)
string HazardMgr::sortedClassificationsToString(){
  sortClassifications(true);

  string out_msg = "";

  std::vector<Classification>::iterator it;
  for(it = m_classifications.begin(); it != m_classifications.end(); ++it){
    if(it == m_classifications.begin())
      out_msg += (*it).printClassification();
    else
      out_msg += "#" + (*it).printClassification();    
  }
  return(out_msg);
}

//---------------------------------------------------------
// Procedure: decisionRule
// PURPOSE:   Decides if we want to report an object as Hazard or not
// @param     c: A Classification object
// @edits     No edits
// @return    True if we want to report c as hazard, false otherwise.
bool HazardMgr::decisionRule(Classification c){
  /*
  // TODO: Update this parameter as a decision rule:
  m_penalty contains: .double missed_hazard, .nonopt_hazard, .false_alarm, .max_time_over, .max_time_rate;
  */
    double p1 = c.getProb() * m_penalty.false_alarm;
    double p2 = (1-c.getProb()) * m_penalty.missed_hazard;

    // If we are more certain than not that this is a hazard, return true
    if(p1 < p2){
      return(true);
    }
    return(false);
}

//---------------------------------------------------------
// Procedure: createHazardString
// PURPOSE:   Used to get specification of object in m_hazard_set based on a 
//            string produced from the corresponding Classification object
// @param     c: A Classification object
// @edits     No edits
// @return    String with XYHazard specifications
string HazardMgr::createHazardString(Classification c){

  // By current label, get spec from classification object
  string label = to_string(c.getLabel());
  int index = m_hazard_set.findHazard(label); 
  XYHazard new_hazard = m_hazard_set.getHazard(index);
  return(new_hazard.getSpec(""));
}

//---------------------------------------------------------
// Procedure: decideHazards
// PURPOSE:   Returns the final report that is sent out via 
//            handleMailReportRequest() by only looking at our known 
//            Classifications, chosen via decisionRule()
// @param     No inputs
// @edits     No edits
// @return    String with HAZARDSET_REPORT for shoreside / uFldHazardMetric
string HazardMgr::decideHazards(){

  sortClassifications(true);

  XYHazardSet new_set;
  new_set.setSource(m_host_community);
  new_set.setName(m_report_name);
  new_set.setRegion(m_search_region);

  vector<Classification>::iterator it;
  for(it = m_classifications.begin(); it != m_classifications.end(); ++it){
    
    // If the current object fulfills requirement for reporting it as a hazard:
    if( decisionRule((*it)) ){
      
      // Use getSpec to create inputstring to string2Hazard from classification
      string str = createHazardString((*it));
      XYHazard new_hazard = string2Hazard(str);

      // Setting type to make sure of that we are reporting this as hazard
      new_hazard.setType("hazard");
      string hazlabel = new_hazard.getLabel();
      
      // TODO: Uneccessary check?
      if(hazlabel == "") {
        reportRunWarning("Add hazard in decideHazards() w/out label");
        return("");
      }

      // Add or overwrite XYHazard to new_set
      int ix = new_set.findHazard(hazlabel);
      if(ix == -1)
        new_set.addHazard(new_hazard);
      else{
        // Overwrite if already in the set, but perhaps as a benign
        new_set.setHazard(ix, new_hazard);
      }
    }
  }
  return(new_set.getSpec("final_report"));
}