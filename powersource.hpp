#ifndef _POWER_H_
#define _POWER_H_

#include <deque>
#include <string>

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

class PowerPeriod;

class VariablePowerSource {
public:
	VariablePowerSource(std::string name) :
		LastTimeAccessInSeconds(-1.0), solarPowerFileName(name), solarPowerLength(0.0) {}
	void ReadVariablePowerSource(std::string filename);
	double AdvanceVariablePowerSource(double t);
	void resetVariablePowerSource(std::string filename);
	void resetVariablePowerSource();

	/**
	 * Get the total solar power length, it's the summation of length in all
	 * interval.
	 * @return the total power length, can be used to initialize DVFS task set
	 */
	double getSolarPowerLength() const { return solarPowerLength; }

private:
	double AdvanceVariablePowerSourceCore(double len);
	std::deque<PowerPeriod> PowerPeriodQueue;
	double LastTimeAccessInSeconds;
	std::string solarPowerFileName;
	double solarPowerLength;
};

extern VariablePowerSource vps;

class PowerPeriod {
	friend class VariablePowerSource;
public:
	PowerPeriod(double power, double len) :
		PowerValue(power), 
		Duration(len),
		RemainingTime(len) {}
private:
	double PowerValue;
	double Duration;
	double RemainingTime;
};

#endif
