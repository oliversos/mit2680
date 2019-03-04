/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT Cambridge MA                                */
/*    FILE: PrimeFactor.cpp                                 */
/*    DATE: 21.02.2019                                      */
/************************************************************/


#include "MOOS/libMOOS/MOOSLib.h"
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "math.h"
#include "PrimeEntry.h"


using namespace std;


//------------------------------------------------------------
// Procedure: PrimeEntry()
// Purpose: Constructor. Creates a primeEntry object and initializes values.

PrimeEntry::PrimeEntry(uint64_t number, unsigned int received)
{
	setOriginalVal(number);
	setDone(false);
	setStartindex(2);
	setIterations(0);
	setReceived(received);
	setStartTime(MOOSTime());
}


//------------------------------------------------------------
// Procedure: factor()
//   Purpose: Calculates the prime factors of the number the PrimeEntry object //            is initializes with. 
//     Input: The maximum amount of modulus operations allowed for the factor //            calculations. If the function has not calculated all the prime //            factors in the given amount max_steps, the progress will be //            saved in the PrimeEntry Object
//   Returns: True if the function successfully calculated all prime factors

bool PrimeEntry::factor(unsigned long int max_steps)
{
	// Sets the iterations allowed for this loop to the input max_steps 
	setIterations(max_steps);

	// Initializes the current number to be factorized
	uint64_t current = m_orig;

	// Calculates the current number to be factorized by dividing it with all the prime factors that is already found.
	for (unsigned int i = 0; i < m_factors.size(); i++){
		unsigned int num = m_factors[i];
		current = current/num;
	}

	// Defines the start and end index for the for loop which aims to calculate further prime numbers. 
	unsigned long int start = m_start_index;
	// The maximum number worth checking is the minimum of start + max steps and sqrt(m_orig). 
	unsigned long int for_end = min((start + max_steps),(unsigned long int)(sqrt(m_orig)) +1);

 	for (unsigned long int i = start; i <= for_end ; i++){

 		// Checks if the current number in the calcilations can be devided by m_start_index. 
		if (current % m_start_index == 0){

			// If the current number = m_start_index, the algorithm is done
			if (current == m_start_index){
				setDone(true);
			}

			// Put the m_start index number in the list of calculated factors, and devide the current number by m_start index. 
			m_factors.push_back(m_start_index);
			current = current/(m_start_index);

			// If the algorithm is completed, modify the amount of iterations remaining for the current iterate() loop, and return true.
			if (m_done == true){
				setIterations(m_iterations + i - start);
				return m_done;
			}
		}

		// If the current is cannot be devided by m_start index, increase m_start index.
		else{
			m_start_index ++;
		}	
	}

	// If m_start index is higher than sqrt(m_orig), there is no point in further searching, as we then know that the current number is a prime.
	if (m_start_index >= sqrt(m_orig)){
		m_done = true;
		m_factors.push_back(current);
		setIterations(0);
		return m_done;
	}

	// If the algorithm has come this far, it has been unable to find the prime factorizations. Set the start index for later calculations and return false
	setStartindex(m_start_index);
	return false;
}

//------------------------------------------------------------
// Procedure: getReport()
//   Purpose: Print a report of the original number and all iẗ́'s prime factors
//   Returns: A string of the original number and all it's prime factors

string PrimeEntry::getReport() const
{
	stringstream ss;

	// Set up the stringstream with the original number and text for primes
	ss << "orig=" << m_orig << ",received=" << m_received << ",calculated=" << m_calculated << ",solve_time=" << m_calculationtime << ",primes=";

	// For all the primes in m_factors that has been calculated in factors, add them to the print statement
	for (int i = 0; i < m_factors.size() ; i++){
		ss << m_factors[i] << ",";
	}

	ss << "username=oliveros" << endl;

	// Convert the stringstream to a string
	string out = ss.str();

	// return the completed string
	return out;
}




