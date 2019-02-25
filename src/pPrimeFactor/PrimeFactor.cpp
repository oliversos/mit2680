/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT Cambridge MA                                */
/*    FILE: PrimeFactor.cpp                                 */
/*    DATE: 21.02.2019                                      */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "PrimeFactor.h"
#include "PrimeEntry.h"
#include <string>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <iostream>

using namespace std;

//---------------------------------------------------------
// Constructor

PrimeFactor::PrimeFactor()
{
  m_numbers = {};
}

//---------------------------------------------------------
// Destructor

PrimeFactor::~PrimeFactor()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()
//   Purpose: Processes new incoming mails. Converts the incoming string into //            a number, and then creates a PrimeEntry object from that number. 
//            The object is placed in fromt of the m_numbers list, which //            contains all the PrimeEntry objects which is to be processed.  
//   Returns: True.

bool PrimeFactor::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
   
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    //If a message with key NUM_VALUE is recieved, create a primeEntry from the message, and place the primeEntry in the from of the m_numbers list.

    if(msg.GetKey() == "NUM_VALUE"){
      string out = msg.GetString();
      uint64_t num = strtoul(out.c_str(),NULL,0);
      PrimeEntry prime(num);      
      m_numbers.push_front(prime);
    }

#if 0 // Keep these around just for template
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()
//   Purpose: When the app is connected to server, RegisterVariables() //            is called.
//    Return: true.
bool PrimeFactor::OnConnectToServer()
{
	
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//   Purpose: Continously calculates the prime factors of the numbers which //            the objects in m_numbers contain. If a calculation is complete,  //            The app post the report from the number to MOOSDB. If the prime //            factor calculations of a number is not completed within the set //            amount of iterations, the result is saved, and the number is //            placed behind in the m_numbers, and will therefore be processed //            in a later iterate() loop after the other numbers it contains.

bool PrimeFactor::Iterate()
{
  // Indicates weather the current number was completed or not
  bool complete;

  // The output to the notify function
  string out;

  // The maximum modulus operations for the current iterate loop
  int iterations = 500000;

  while(!m_numbers.empty()){

    // Pulls out a PrimeEntry from the back of m_numbers
    PrimeEntry prime = m_numbers.back();
    
    // Tries to calculate it's prime factors, and returns weather the task was successfull
    complete = prime.factor(iterations);

    // If the prime factorization was completed, remove the current PrimeEntry from the m_numbers list, get the remaining iterations and post the report.
    if (complete == true){
      m_numbers.pop_back();
      out = prime.getReport();
      iterations = prime.getIterations();
      Notify("PRIME_RESULT",out);
    }

    // If the prime factorization was not completed, take out the current PrimeEntry from the back of the list, and place it in the front. This way, it will be processed after all the other PrimeEntries in the list have been given a try to be completely primefactorized.
    else if (complete == false){
      m_numbers.pop_back();
      m_numbers.push_front(prime);
      return (true);
    }

    // If there is no iterations left, return true
    if (iterations <= 0){
      return(true);
    }
  }
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//   Purpose: Is called once the app has started, in turns calls the //            RegisterVariables() function

bool PrimeFactor::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string line  = *p;
      string param = tolower(biteStringX(line, '='));
      string value = line;
      
      if(param == "foo") {
        //handled
      }
      else if(param == "bar") {
        //handled
      }
    }
  }
  RegisterVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables
//   Purpose: Registers the app for NUM_VALUE to get updates on which numbers  //            that should be prime factorized. The 0 indicates that it will //            insantly recieve updates once NUM_VALUE is posted from MOOSDB
void PrimeFactor::RegisterVariables()
{
  Register("NUM_VALUE",0);
}
