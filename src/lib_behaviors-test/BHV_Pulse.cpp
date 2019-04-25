/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Pulse.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Pulse.h"
#include "XYRangePulse.h"
#include "ZAIC_PEAK.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_Pulse::BHV_Pulse(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  m_waypoint_time = getBufferCurrTime();
  m_waypoint_index = -1;
  m_pulse_range = 1;
  m_pulse_duration = 1;
  m_post_variable = false;

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, WPT_INDEX");
  postMessage("WPT_INDEX",0);
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Pulse::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "foo") && isNumber(val)) {
    // Set local member variables here
    return(true);
  }

  else if ((param == "pulse_duration") && (double_val > 0) && (isNumber(val))){
  	m_pulse_duration = double_val;
  	return(true);
  }

  else if ((param == "pulse_range") && (double_val > 0) && (isNumber(val))) {
  	m_pulse_range = double_val;
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

void BHV_Pulse::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Pulse::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Pulse::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Pulse::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Pulse::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Pulse::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Pulse::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_Pulse::onRunState()
{
	bool boo;
	double index = getBufferDoubleVal("WPT_INDEX",boo);
	postMessage("CURR_INDEX",index);

	if (m_waypoint_index == -1){
		m_waypoint_index = index;
		m_waypoint_time = getBufferCurrTime();
	}
	else{
		if (m_waypoint_index != index){
			m_waypoint_index = index;
			m_post_variable = true;
			postMessage("WAYPOINT_POST","true");
			m_waypoint_time = getBufferCurrTime();
		}
	}

	if (m_post_variable == true){
		if (getBufferCurrTime() - m_waypoint_time >= 5.0){
			postPulse();
			m_post_variable = false;
		}
	}



  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;



  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

void BHV_Pulse::postPulse()
{	
	bool nx,ny;
	double nav_x = getBufferDoubleVal("NAV_X",nx);
	double nav_y = getBufferDoubleVal("NAV_Y",ny);
	if (!nx || !ny){
		postWMessage("No ownship X/Y in info_buffer.");
		return;
	}
	postMessage("X_GOT",nav_x);
	postMessage("Y_GOT",nav_y);
	postMessage("RANGE",m_pulse_range);
	postMessage("DURATION",m_pulse_duration);
	XYRangePulse pulse;
	pulse.set_x((int)nav_x);
	pulse.set_y((int)nav_y);
	pulse.set_label("bhv_pulse");
  pulse.set_rad(m_pulse_range);
  pulse.set_duration(m_pulse_duration);
  pulse.set_color("edge", "yellow");
  string spec = pulse.get_spec();
  postMessage("VIEW_RANGE_PULSE",spec);
}

