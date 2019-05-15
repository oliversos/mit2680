/************************************************************/
/*    NAME: Simen Sem Oevereng                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_GradTrack.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_GradTrack.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"
#include <math.h>

#include "XYRangePulse.h" // for XYRangePulse 

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_GradTrack::BHV_GradTrack(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "GradTrack");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y");
  addInfoVars("NAV_HEADING");
  addInfoVars("FOO", "no_warning");
  addInfoVars("COLD_DIR","no_warning");     // 
  addInfoVars("FRONT_GRADIENT","no_warning");
  addInfoVars("UCTD_MSMNT_REPORT","no_warning");
  addInfoVars("AVERAGE_TEMP","no_warning");
  addInfoVars("TEMP_DIFF","no_warning");
  addInfoVars("START_TIME","no_warning");
  addInfoVars("MSMNT_REPORT","no_warning");
  postMessage("CONCURRENT","true");

  // TODO: Initialize all member variables
  m_nav_x = 0;
  m_nav_y = 0;
  m_nav_h = 0;

  m_temp_threshold = 0.5; // TODO: Check this size from runs
  m_measured_temp_avg = 20;
  m_global_temp_avg = 20;
  m_last_temp = -100.;
  m_current_temp = -100;
  m_global_front_gradient = 0.0; // assumed to be angle in radians, moving in in x,y coordsyst
  m_gradient_heading = 0.;
  m_cold_direction = -1; // initialized to give left turn if sailing east
  m_turn_rate = 1;
  m_started = false;
  m_last_update = getBufferCurrTime();

  m_priority_wt = 100;
  m_start_time = 0;

  m_objective_function = false;
  m_change_direction = false;
  m_cold_initiated = false;
  m_drive_east = true;

  m_last_message_time = getBufferCurrTime();
  m_last_report = "";
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_GradTrack::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if (param == "pwt") {
    if(double_val >= 0){
      m_priority_wt = double_val;
    }
    else {
      postWMessage("Priority weight (pwt) is negative. Setting it to 1");
      m_priority_wt = 1;
    }
    return(true);
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_GradTrack::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_GradTrack::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_GradTrack::onIdleState()
{
  // TODO: Not do anything, right?
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_GradTrack::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_GradTrack::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_GradTrack::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_GradTrack::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_GradTrack::onRunState()
{
	if (m_start_time == 0.){
		bool okstart;
		double start = getBufferDoubleVal("START_TIME",okstart);
		if (okstart){
			m_start_time = start;
			postMessage("START_TIME",m_start_time);
		}
	}

	if (getBufferCurrTime() - m_start_time >  460.){
		postMessage("RETURN","true");
	}
  // Build the IvP functions that represents no objective function and an objective function defined through ZAIC. See buildFunctionWithZAIC()
  // Vehicle params from InfoBuffer. Post warning if problem is encountered

  bool oknx,okny,oknh,okwpti,oktemp,oktempavg,okgrad,okcold,okstart,okdiff;
  m_nav_x = getBufferDoubleVal("NAV_X", oknx);
  m_nav_y = getBufferDoubleVal("NAV_Y", okny);
  m_nav_h = getBufferDoubleVal("NAV_HEADING", oknh);
  m_global_temp_avg = getBufferDoubleVal("AVERAGE_TEMP",oktempavg);
  m_temp_diff = getBufferDoubleVal("TEMP_DIFF",okdiff);

  if ((m_nav_x > 150 || m_nav_x < -40) && !m_change_direction) {
    m_change_direction = true;
    m_drive_east = !m_drive_east;
    postMessage("TURN_X",m_nav_x);
    postMessage("DRIVE_EAST",m_drive_east);
  }

  postMessage("AVG_TEMP",m_global_temp_avg);

  m_global_front_gradient = getBufferDoubleVal("FRONT_GRADIENT",okgrad);
  double cold_direction = getBufferDoubleVal("COLD_DIR",okcold);
  string measurement = getBufferStringVal("UCTD_MSMNT_REPORT", oktemp);

  double timeSinceTempReading = getBufferTimeVal("UCTD_MSMNT_REPORT"); 

  if (timeSinceTempReading == 0.){
    Measurement new_m = stringToMeasurement(measurement);
    postMessage("TEMP_REP",measurement);
    if (testMeasurement(new_m)){
      string sending_msg;
      sending_msg += "src_node=" + m_us_name;
      sending_msg += ",dest_node=all";
      sending_msg += ",var_name=MSMNT_REPORT";
      sending_msg += ",string_val=";
      sending_msg += measurement;
      postMessage("NODE_MESSAGE_LOCAL",sending_msg);
    }
  }

  IvPFunction *ipf = 0;
  ZAIC_PEAK crs_zaic(m_domain,"course");

  if(okcold && !m_cold_initiated){
    m_cold_direction = cold_direction; // 1 if cold is north, -1 if south
    m_cold_initiated = true;
  }

  double heading = m_nav_h;

  if (m_change_direction){
    if (m_nav_x > 50){
      heading = 270;
    }
    else{
      heading = 90;
    }
    if (fabs(m_nav_h - heading) < 5 && (m_nav_x < 150 && m_nav_x > -40)){
      m_change_direction = false;
      m_cold_direction = - m_cold_direction;
    }
  }

  else if(oktemp){
    double temp = measurementToTemp(measurement);
    if (m_last_temp == -100.){
      m_current_temp = temp;
      m_last_temp = temp;
    }
    if (m_current_temp != temp){
    	m_last_temp = m_current_temp;
    	m_current_temp = temp;
    }

    double delta_heading_prop = 0;

    delta_heading_prop = (temp - m_global_temp_avg)*(180/m_temp_diff)*m_cold_direction;

    double delta_heading_deriv = (m_current_temp - m_last_temp)*(120/m_temp_diff)*m_cold_direction;

    postMessage("CURR_TEMP",m_current_temp);
    postMessage("LAST_TEMP",m_last_temp);
    postMessage("DELTA_PROP",delta_heading_prop);
    postMessage("DELTA_DER",delta_heading_deriv);

    heading += delta_heading_prop;
    heading += delta_heading_deriv;
  }





  if (!m_change_direction && m_drive_east ){
    if (m_nav_h > 170){
      heading = 160;
    }
    if (m_nav_h < 10){
      heading = 20;
    }
  }

  if (!m_change_direction && !m_drive_east){
    if (m_nav_h > 345){
      heading = 335;
    }
    if (m_nav_h < 195){
      heading = 205;
    }
  }

  if(!oknx || !okny || !oknh) {
    postWMessage("No ownship X/Y/Heading info in info_buffer.");
    return(0);
  }

  crs_zaic.setSummit(heading);
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);  
  crs_zaic.setValueWrap(true);
  if(crs_zaic.stateOK() == false) {
      string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
      postWMessage(warnings);
      return(0);
  }

  ipf = crs_zaic.extractIvPFunction();

  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

// Based on the first part of lab. Kept for later reference
double BHV_GradTrack::measurementToTemp(string m){
  // String will be like:
  // "vname=v_name,utc=XXX.0,x=123.2,y=412.2,temp=22.34"
  // TODO: Test to see if the four former parses is necessary
  string a = tokStringParse(m, "vname", ',', '=');
  string b = tokStringParse(m, "utc", ',', '=');
  string c = tokStringParse(m, "x", ',', '=');
  string d = tokStringParse(m, "y", ',', '=');
  string t = tokStringParse(m, "temp", ',', '=');
  
  // return temperature as a doublensert(m_last_temps.begin(),temp);
  return(std::stod(t));// stod is from the string library
}



bool BHV_GradTrack::testMeasurement(Measurement m){
  double max = m_global_temp_avg + m_temp_diff/4.;
  double min = m_global_temp_avg - m_temp_diff/4.;
  if (m.t < max && m.t > min){
    return true;
  } 
  return false;
}

Measurement BHV_GradTrack::stringToMeasurement(string m){

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


//-----------------------------------------------------------
// Procedure: buildFunctionWithZAIC
// Purpose:   Builds an objective function to be used for sailing on a certain
//            heading away from a designated heading from Waypoint behaviour
// Params:    no input
// Edits:     no edits
// Returns:   an IvPFunction, used in OnRunState()

/*
IvPFunction *BHV_GradTrack::buildFunctionWithZAIC() 
{

  // double offset = m_zigleg_heading + m_zigleg_offset;
  double offset = 0;
  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(offset);
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);  
  crs_zaic.setValueWrap(true);
  if(crs_zaic.stateOK() == false) {
    string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }

  IvPFunction *crs_ipf = crs_zaic.extractIvPFunction();
  return(crs_ipf);
}
*/