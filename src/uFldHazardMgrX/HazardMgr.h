/*****************************************************************/
/*    NAME: Oliver Os and Simen Sem Oevereng                     */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: HazardMgr.h                                          */
/*    DATE: Apr 12 2019                                          */
/*    This app expands the functionality of the MOOS-IvP app     */
/*    uFldHazardMgr by adjusting it according to specification   */
/*    in lab 12 and 13 of the 2.680 class at MIT, Spring 2019    */
/*****************************************************************/

#ifndef UFLD_HAZARD_MGR_HEADER
#define UFLD_HAZARD_MGR_HEADER
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYHazardSet.h"
#include "XYPolygon.h"
#include "NodeMessage.h"

#include <vector>
#include "Classification.h"

// Self declared struct for storing penalty values
struct Penalty{
   double missed_hazard, nonopt_hazard, false_alarm, max_time_over, max_time_rate;
}; 

class HazardMgr : public AppCastingMOOSApp
{
 public:
   HazardMgr();
   ~HazardMgr() {}

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected: // Registration, Configuration, Mail handling utils
   void registerVariables();
   bool handleMailSensorConfigAck(std::string);
   bool handleMailSensorOptionsSummary(std::string) {return(true);}
   bool handleMailDetectionReport(std::string);
   bool handleMailHazardReport(std::string) {return(true);}
   void handleMailReportRequest();
   void handleMailMissionParams(std::string);   
   void handleAddName(std::string);
   void handleHazardReport(std::string);

 protected: 
   void postSensorConfigRequest();
   void postSensorInfoRequest();
   void postHazardSetReport();

   // Self implemented
   void postHazardMessage();
   void handleClassificationReport(std::string);
   void setClassificationStatus(int,bool);
   bool classificationExist(int);
   void updateNewClassification(int,double,bool);
   void updateIncomingClassification(int,double);
   void sortClassifications(bool);
   std::string sortedClassificationsToString();
   bool decisionRule(Classification c);
   string createHazardString(Classification c);
   string decideHazards();

   
 private: // Configuration variables
   double      m_swath_width_desired;
   double      m_pd_desired;
   std::string m_report_name;

 private: // State variables
   bool   m_sensor_config_requested;
   bool   m_sensor_config_set;
   bool   m_send_report;

   unsigned int m_sensor_config_reqs;
   unsigned int m_sensor_config_acks;

   unsigned int m_sensor_report_reqs;
   unsigned int m_detection_reports;

   unsigned int m_summary_reports;

   double m_swath_width_granted;
   double m_pd_granted;

   XYHazardSet m_hazard_set;
   XYPolygon   m_search_region;
   
   double      m_transit_path_width;

   // Self implemented 
   bool m_initiated;

   double m_last_msg_sent;       // time since last message
   std::string m_name;           // name of current vehicle
   XYHazardSet m_hazard_sent;    // the set of sent hazards, not add to msg
   std::string m_msg;            // stores the hazard message

   Penalty m_penalty;            // Stores value of penalties
   double m_max_time;            // Stores specified maximum time
   double m_mission_start_time;  // Stores time since mission was deployed
   std::string m_search_region_str; // Search region in shape of a polygon

   // A vector of Classification object for storing its own Classification reports, and to update probabilities as different Classification reports come in
   std::vector<Classification> m_classifications; 

   string m_classification_msg;  
   double m_pclass;              // Probability of correct classification
};

#endif 







