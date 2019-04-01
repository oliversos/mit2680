/************************************************************/
/*    NAME: Oliver Os                                              */
/*    ORGN: MIT                                             */
/*    FILE: PointAssign.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef PointAssign_HEADER
#define PointAssign_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <map>
#include "Point.h"
#include <string>

using namespace std;

class PointAssign : public CMOOSApp
{
	public:
		PointAssign();
		~PointAssign();

	protected: // Standard MOOSApp functions to overload  
		bool OnNewMail(MOOSMSG_LIST &NewMail);
		bool Iterate();
		bool OnConnectToServer();
		bool OnStartUp();

	protected:
		void RegisterVariables();
		void assignByRegion(string,string);
		void assignByAlternate(string,string);
		void postViewPoint(double,double,int,string);

	private: // Configuration variables
		vector<string> m_vname;
		bool m_assign_by_region;
		map<int,Point> m_visit_points;

	private: // State variables
		bool m_send_points;
};

#endif 
