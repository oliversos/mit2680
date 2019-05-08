/************************************************************/
/*    NAME: Simen Sem Oevereng                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_GradTrack.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef GradTrack_HEADER
#define GradTrack_HEADER

#include <string>
#include <vector>
#include "IvPBehavior.h"

class BHV_GradTrack : public IvPBehavior {
public:
  BHV_GradTrack(IvPDomain);
  ~BHV_GradTrack() {};

  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();

protected: // Local Utility functions
  IvPFunction* buildFunctionWithZAIC();

  double measurementToTemp(string s);
  void followGradient();
  void updateTempAvg();
  void postTurnDir(double change);

protected: // Configuration parameters

protected: // State variables
  // Vessel x,y,heading
  double m_nav_x;
  double m_nav_y;
  double m_nav_h;

  // Store x number of previous measurements. Is initialized
  std::vector<double> m_last_temps;
  double m_temp_threshold;
  double m_measured_temp_avg;
  double m_global_temp_avg;
  double m_global_front_gradient;
  int m_cold_direction;
  double m_turn_rate;
  bool m_started;
  double m_priority_wt;
  double m_last_update;

  // stored for eventual use
  bool m_objective_function;// Is the zigleg governed by a non-zero obj func?
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_GradTrack(domain);}
}
#endif