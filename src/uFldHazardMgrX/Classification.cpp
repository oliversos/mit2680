
// SIMEN SEM OEVERENG

#include "Classification.h"
#include <sstream>
#include <iomanip>
using namespace std;

// Returns a string of the classification (currently without showing if it has been shared or not). Used mainly for debugging purposes.
string Classification::printClassification(){
	string out = "id=";
	out += to_string(m_label);
	out += ".p=";
	stringstream ss;
	ss << std::setprecision(3) << m_prob;
	out += ss.str();	
	out += ".t=";
	if (m_hazard)
		out += "h";
	else
		out += "b";
  return out;
}

// Calculate probability of object being a hazard based on new classification received and the certainty of that classification. Only called if object already has been classified once.
void Classification::calculateProb(bool newClass, double p_in){
  // use setProb(double p) after calculating

  // CASE: After previous classifications, we have a certain prob of this label being a hazard. Given a new observation and a following classification report from UHZ_HAZARD_REPORT (or as Oliver is working with, a collaborating vehicle), what is the current probability?

  double probH = 0.01;
  double probB = 0.01;
  // If the new classification is a hazard
  if(newClass){
    // Classification is p_in certain of object being a hazard. Therefore, we multiply that probability with our current belief of m_label being H
    probH = p_in * m_prob; 

    // Probability of the object being benign is then (1-prob_believedToBeHaz) * (1-p_WrongClassification)
    probB = (1 - m_prob) * (1 - p_in);
  }
  else{
    // Classification is p_in certain of object being a benign. Therefore, the prob of object being a hazard is m_prob * (1 - p_in)
    probH = (1 - p_in) * m_prob;

    // Probability of the object being benign is then (1-m_prob) * p_in
    probB = (1 - m_prob) * p_in;
  }

  // Scale probability due to now knowledge of exact distribution and number of hazards
  m_prob = probH / (probH + probB);
}