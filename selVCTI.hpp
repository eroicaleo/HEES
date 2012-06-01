#ifndef _DC_VCTI
#define _DC_VCTI

#include <vector>
#include <string>

using std::vector;
using std::string;

class supcapacitor;
class ees_bank;

class selVcti{
		public:
			selVcti();
			double bestVCTI(double I, double dccon1_Iout, double dccon1_Vout, string selMode, lionbat *lb, ees_bank *sp);
		private:
			double Vmax, Vmin;
			double alpha, epsilon;

};
#endif
