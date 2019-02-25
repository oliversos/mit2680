/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT                                             */
/*    FILE: PrimeFactor.h                                   */
/*    DATE: 21.02.2019                                      */
/************************************************************/

#ifndef PrimeFactor_HEADER
#define PrimeFactor_HEADER

#include "PrimeEntry.h"
#include "MOOS/libMOOS/MOOSLib.h"

using namespace std;

class PrimeFactor : public CMOOSApp
{
 public: // Constructor and destructor
   PrimeFactor();
   ~PrimeFactor();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();


 protected: 
	 list<PrimeEntry> m_numbers;
    unsigned int m_received;
    unsigned int m_calculated;
};

#endif 
