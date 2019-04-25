/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ZigLeg.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_ZigLeg.h"
#include "ZAIC_PEAK.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_ZigLeg::BHV_ZigLeg(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  m_waypoint_time = getBufferCurrTime();
  m_waypoint_index = -1;
  m_post_variable = false;
  m_zig_duration = 10;
  m_zig_angle = 45;
  m_start_time = getBufferCurrTime();
  m_zigzag = false;
  m_heading = 0;

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, NAV_HEADING, WPT_INDEX, ZIGLEG");
  postMessage("WPT_INDEX",0);
  postMessage("ZIGLEG","false");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_ZigLeg::setParam(string param, string val)
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
  else if (param == "zig_duration"){
    m_zig_duration = double_val;
    return (true);
  }
  else if (param == "zig_angle"){
    m_zig_angle = double_val;
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

void BHV_ZigLeg::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_ZigLeg::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_ZigLeg::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_ZigLeg::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_ZigLeg::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_ZigLeg::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_ZigLeg::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_ZigLeg::onRunState()
{

  bool boo;
  double index = getBufferDoubleVal("WPT_INDEX",boo);

  if (m_waypoint_index == -1){
    m_waypoint_index = index;
    m_waypoint_time = getBufferCurrTime();
  }
  else{
    if (m_waypoint_index != index){
      m_waypoint_index = index;
      m_post_variable = true;
      m_waypoint_time = getBufferCurrTime();
    }
  }

  

  // Part 1: Build the IvP function

  if (m_post_variable == true){
    if (getBufferCurrTime() - m_waypoint_time >= 5.0){
      m_post_variable = false;
      m_start_time = getBufferCurrTime();
      m_zigzag = true;
      bool h;
      double heading = getBufferDoubleVal("NAV_HEADING",h);
      if (!h){
        return (0);
      }
      m_heading = heading + m_zig_angle;
    }
  }

  IvPFunction *ipf = 0;

  if (m_zigzag){
    if (getBufferCurrTime() - m_start_time < m_zig_duration){
      ZAIC_PEAK crs_zaic(m_domain, "course");
      crs_zaic.setSummit(m_heading);
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
      // Part N: Prior to returning the IvP function, apply the priority wt
      // Actual weight applied may be some value different than the configured
      // m_priority_wt, depending on the behavior author's insite.
      if(ipf)
        ipf->setPWT(m_priority_wt);
      return(ipf);
    }
    else{
      m_zigzag = false;
      if (ipf)
        ipf->setPWT(0);
      return(ipf);
    }
  }
}

