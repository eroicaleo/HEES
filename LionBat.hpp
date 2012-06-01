#ifndef _LION_BAT_H_
#define _LION_BAT_H_

#include <vector>
using std::vector;

class ees_bank;

class lionbat {
	public:
		//Default Constructor
		lionbat();
		// Battery Model
		void BatteryModel(double Iin, double Tdur, double &Voc, double &Vsoc);
		//Checking State of Charge
		double VsocCheck(double Iin, double Tdur);
		// Set the battery state of charge (soc)
		void BatterySetVsoc(double Vsoc);
		// Get the battery state of charge (soc)
		double BatteryGetVsoc(void);
		// Get the open circuit voltage
		double BatteryGetVoc(double Vsoc);
	private:
		double m_VSOC;
		double m_Rsd;
		vector<double> m_b;
		double m_Cb, m_Cinit;
};
#endif
