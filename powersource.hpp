#ifndef _POWER_H_
#define _POWER_H_

double powersource(double stHour, double stMin, double Tdur);

double solar_power_source_sec(double m_Tdur); 

class ConstantPowerSource {
public:
	ConstantPowerSource() : power_value(0.0) {}
	void SetPowerValue(double p) { power_value = p; }
	double GetConstantPower(double s) const { return power_value; }
private:
	double power_value;
};

#endif
