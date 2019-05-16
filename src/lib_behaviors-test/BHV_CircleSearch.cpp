/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_CircleSearch.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_CircleSearch.h"
#include "ZAIC_PEAK.h"
#include <math.h>

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_CircleSearch::BHV_CircleSearch(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");
  m_last_sample_time = getBufferCurrTime();
  m_start_time = getBufferCurrTime();
  m_prepare_finish = false;
  m_radius = 0.;
  m_init_heading = 0.;
  m_initiated_heading = false;
  m_clockwise = false;
  m_delta_heading = 0.;
  m_cold_top = false;
  m_finish_turn = false;
  m_middle_temp = 0.;
  m_temp_diff = 0.;

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, NAV_HEADING,UCTD_MSMNT_REPORT, NAV_SPEED,VNAME");
  postMessage("CONCURRENT","false");
}

//---------------------------------------------------------------
// Procedure: setParam()


bool BHV_CircleSearch::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "foo") && isNumber(val)) {
    // Set local member variables here
    return(true);
  }
  else if (param == "bar") {
    // return(setBooleanOnString(m_my_bool, val));
  }

  else if (param == "init_heading"){
    m_init_heading = double_val;
    return (true);
  }
  else if (param == "radius"){
    m_radius = double_val;
    return (true);
  }
  else if (param == "clockwise"){
    val = tolower(val);
    if (val == "true"){
      m_clockwise = true;
    }
    else{
      m_clockwise = false;
    }
    return (true);
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_CircleSearch::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_CircleSearch::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_CircleSearch::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_CircleSearch::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_CircleSearch::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_CircleSearch::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_CircleSearch::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_CircleSearch::onRunState()
{ 
  if (!m_initiated_heading){
    m_start_time = getBufferCurrTime();
  }

  
  if (getBufferCurrTime() - m_last_sample_time > 1.){
    bool t_report;
    string temp_report = getBufferStringVal("UCTD_MSMNT_REPORT",t_report);
    if (!t_report){
      return(0);
    }
    Measurement new_m = stringToMeasurement(temp_report);
    m_measurements.push_back(new_m);
    m_last_sample_time = getBufferCurrTime();

    if (m_prepare_finish && !m_finish_turn){
      postMessage("AVERAGE_TEMP",m_middle_temp);
      postMessage("TEMP_DIFF",m_temp_diff);
      postMessage("CURR_TEMP",new_m.t);
      postMessage("DIFFERENCE",new_m.t - m_middle_temp);
      if (fabs(new_m.t - m_middle_temp) < 3.0){      
        if (m_cold_top){
          postMessage("COLD_DIR",-1);
        }
        else{
          postMessage("COLD_DIR",1);
        }
        m_finish_turn = true;
      }
    }
  }


  if ((getBufferCurrTime() - m_start_time > 170) && !m_prepare_finish){
    postMessage("BUFFTIME",getBufferCurrTime());
    postMessage("START_TIME",m_start_time);
    m_prepare_finish = true;
    double top_temp = -1000;
    double min_temp = 1000;
    double top_y = 0;
    double min_y = 0;
    for (unsigned int i=0; i < m_measurements.size(); i++) {
      Measurement m = m_measurements[i];
      if (m.t > top_temp){
        top_temp = m.t;
        top_y = m.y;
      }
      if (m.t < min_temp){
        min_temp = m.t;
        min_y = m.y;
      }
    }
    m_middle_temp = (top_temp + min_temp)/2.;
    m_temp_diff = top_temp - min_temp;
    if (top_y > min_y){
      m_cold_top = false;
    }
    else{
      m_cold_top = true;
    }
  }

  if (m_delta_heading == 0.){
    m_delta_heading = 700/m_radius;
    postMessage("DELTA_H",m_delta_heading);
  }
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;
  ZAIC_PEAK crs_zaic(m_domain,"course");

  if(crs_zaic.stateOK() == false) {
      string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
      postWMessage(warnings);
      return(0);
  }

  bool h; bool i; bool j;

  double heading = getBufferDoubleVal("NAV_HEADING",h);
  double speed = getBufferDoubleVal("NAV_SPEED",i);
  double nav_x = getBufferDoubleVal("NAV_X",j);


  if (!h || !i){
    return (0);
  }


  if (!m_initiated_heading || m_finish_turn){
    crs_zaic.setSummit(m_init_heading);
    crs_zaic.setPeakWidth(3.);
    crs_zaic.setBaseWidth(180.0); 
    crs_zaic.setSummitDelta(0);  
    crs_zaic.setValueWrap(true);
    if(crs_zaic.stateOK() == false) {
        string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
        postWMessage(warnings);
        return(0);
    }
    postMessage("HEADING_DIFF",fabs(heading - m_init_heading));
    if (fabs(heading - m_init_heading) < 9.){
      if (nav_x > 35.)
        m_initiated_heading = true;
      if (m_finish_turn){
        postMessage("CIRCLE","false");
        postMessage("TRACK","true");
      }
    }
  }
  else{
    double new_heading;
    if (m_clockwise == true){
      new_heading = heading + m_delta_heading;
      postMessage("HER","JA");
      postMessage("DELTA_HEADING",m_delta_heading);
      postMessage("NEW_HEADING",new_heading);
    }
    else{
      new_heading = heading - m_delta_heading;
      postMessage("NEW",new_heading);
    }
    crs_zaic.setSummit(new_heading);
    crs_zaic.setPeakWidth(0);
    crs_zaic.setBaseWidth(180.0);
    crs_zaic.setSummitDelta(0);  
    crs_zaic.setValueWrap(true);
    if(crs_zaic.stateOK() == false) {
        string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
        postWMessage(warnings);
        return(0);
    }

  }
  ipf = crs_zaic.extractIvPFunction();

  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

Measurement BHV_CircleSearch::stringToMeasurement(string m){

  // String will be like:
  // "vname=v_name,utc=XXX.0,x=123.2,y=412.2,temp=22.34"

  // TODO: Test to see if the four former parses is necessary
  string a = tokStringParse(m, "vname", ',', '=');
  string b = tokStringParse(m, "utc", ',', '=');
  string c = tokStringParse(m, "x", ',', '=');
  string d = tokStringParse(m, "y", ',', '=');
  string t = tokStringParse(m, "temp", ',', '=');

  double x = stod(c);
  double y = stod(d);
  double temp = stod(t);

  Measurement measure = {x,y,temp};
  // return temperature as a double
  return(measure);// stod is from the string library
}

