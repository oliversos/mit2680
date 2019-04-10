// SIMEN SEM OEVERENG

#ifndef CLASSIFICATION_HEADER
#define CLASSIFICATION_HEADER

#include <string>
using namespace std;

// TODO: NEW
class Classification{
   public:
      Classification(){};
      Classification(int l, bool h, double p, bool s){m_label=l;m_hazard=h;m_prob=p; m_shared=s;};
      Classification(int l, bool h){m_label=l; m_hazard=h; m_prob=0.99;m_shared = false;};
      ~Classification(){};

      // Overloaded to use std::sort on vector as sort(v.begin(), v.begin(), v.end())
      bool operator< (const Classification& p) const {return(m_prob<p.m_prob);}
      int getLabel() const {return m_label;};
      double getProb()  {return m_prob; };
      string printClassification();

      void setLabel(int l)    {m_label = l;};
      void setProb(double p)  {m_prob  = p;};
      void calculateProb(bool newClass, double p);

      void setShared(bool s) {m_shared = s;};
      bool getShared() const {return m_shared;};

   protected:
      int m_label;
      bool m_hazard;   // or string classification;
      double m_prob;   // storing prob for m_label being a hazard
      bool m_shared;   // stores if classification is sent to collaborator
};
#endif 