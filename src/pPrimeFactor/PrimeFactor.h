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
  // Registers the app for the relevant variables
   void RegisterVariables();

 protected: 
  // Represents the numbers that is yet to be prime factorized
	 list<PrimeEntry> m_numbers;
  // Represents the amount of numbers received
    unsigned int m_received;
  // Represents the amount of calculated numbers
    unsigned int m_calculated;
  // Represents the max mathematical operations per iterate loop
    unsigned int m_iterations;
};

#endif 
