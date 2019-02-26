/************************************************************/
/*    NAME: Oliver Os                                       */
/*    ORGN: MIT Cambridge MA                                */
/*    FILE: PrimeFactor.cpp                                 */
/*    DATE: 21.02.2019                                      */
/************************************************************/

#include <string>
#include <vector>
#include <cstdint>

#ifndef PRIME_ENTRY_HEADER
#define PRIME_ENTRY_HEADER

using namespace std;

//This class holds a number, and performs a prime factorization on it. 
//Is used to save the process of calculating the factors, due to limited 
//Time available on each iteration in the iterate() function in PrimeFactor.cpp
class PrimeEntry
{
public: //Constructor and destructor
	PrimeEntry(uint64_t number, unsigned int received);
	~PrimeEntry() {};

public: //Setters
	void setOriginalVal(uint64_t v)	          {m_orig = v;};
	void setDone(bool v)											{m_done = v;};
	void setStartindex(uint64_t v)            {m_start_index = v;};
	void setIterations(unsigned int v)        {m_iterations = v;};
	void setReceived(unsigned int v)          {m_received = v;};
	void setCalculated(double v)              {m_calculated = v;}; 
	void setCalculationTime(double v)         {m_calculationtime = v - m_start_time;};
	void setStartTime(unsigned int v)         {m_start_time = v;};

public: 
	// Returns true if the factorization is completed, false otherwise
	bool done() const {return (m_done);};

	// Performs a prime number factorization on the number
	bool factor(unsigned long int max_steps);

	// Getters 
	unsigned int getIterations() const         {return m_iterations;};
	unsigned int getStartIndex() const         {return m_start_index;};

	// Returns a string that shows the original number and its prime factors
	string getReport() const;

private: 
	// Indicates the number that the prime factorization algorithm should start the search after the process is interupted.
	uint64_t m_start_index;

	// Holds the number which is to be prime factorized
	uint64_t m_orig;

	// Indicatea wheather the factorization is done or not
	bool m_done;

	// Holds the amount of iterations that is remaining in the current iterate() loop in PrimeFactor.cpp
	unsigned int m_iterations;

	// Holds the number of which if was received by the app
	unsigned int m_received;

	// Holds the number of which it was calculated by the app
	unsigned int m_calculated;

	// Holds the time at which the calculations started. 
	double m_start_time;

	// Holds the total caluclation time for the number
	double m_calculationtime;

	// Vector that contains all the prime factors that is currently calculated from m_orig
	vector<uint64_t> m_factors;
};
#endif