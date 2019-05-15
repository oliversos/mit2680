/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_CircleSearch.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef CircleSearch_HEADER
#define CircleSearch_HEADER

#include <string>
#include "IvPBehavior.h"

struct Measurement{
  double x,y,t;
};

class BHV_CircleSearch : public IvPBehavior {
public:
  BHV_CircleSearch(IvPDomain);
  ~BHV_CircleSearch() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  void         handleAddName(string s);
  bool         testMeasurement(Measurement m);
  string       measurementToString(Measurement m);
  Measurement  stringToMeasurement(string m);
  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters
  double m_radius;
  double m_middle_temp;
  double m_temp_diff;
  bool m_cold_top; //Send -1 for cold top, 1 for cold bot
  double m_init_heading;
  bool m_initiated_heading;
  bool m_prepare_finish;
  bool m_finish_turn;
  bool m_clockwise;
  double m_delta_heading;
  double m_last_sample_time;
  double m_start_time;
  double m_last_message_time;
  string m_name;
  string m_last_report;
  vector<Measurement> m_measurements;

protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_CircleSearch(domain);}
}
#endif
