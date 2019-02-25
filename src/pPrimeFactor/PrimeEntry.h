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
	PrimeEntry(uint64_t number);
	~PrimeEntry() {};

public: //Setters
	void setOriginalVal(uint64_t v)	          {m_orig = v;};
	void setDone(bool v)											{m_done = v;};
	void setStartindex(uint64_t v)            {m_start_index = v;};
	void setIterations(unsigned int it)       {m_iterations = it;};

public: 
	// Returns true if the factorization is completed, false otherwise
	bool done() {return (m_done);};

	// Performs a prime number factorization on the number
	bool factor(unsigned long int max_steps);

	// Getters 
	unsigned int getIterations()              {return m_iterations;};
	unsigned int getStartIndex()              {return m_start_index;};

	// Returns a string that shows the original number and its prime factors
	string getReport();

private: 
	// Indicates the number that the prime factorization algorithm should start the search after the process is interupted.
	uint64_t m_start_index;

	// Holds the number which is to be prime factorized
	uint64_t m_orig;

	// Indicatea wheather the factorization is done or not
	bool m_done;

	// Holds the amount of iterations that is remaining in the current iterate() loop in PrimeFactor.cpp
	unsigned int m_iterations;

	// Vector that contains all the prime factors that is currently calculated from m_orig
	vector<uint64_t> m_factors;
};
#endif