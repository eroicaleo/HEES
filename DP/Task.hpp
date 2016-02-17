#ifndef __TASK_HPP__
#define __TASK_HPP__

#include "../DCCon_in.hpp"
#include "../TaskHandoff.hpp"

class Task {

public:
	Task(double v, double c, size_t l):
		voltage(v),
		current(c),
		length(l) {

			dcconvertIN dcLoad;
			double dc_load_vin(1.0), dc_load_vout(voltage), dc_load_iout(current);
			double dc_load_iin(0.0), dc_load_power(0.0);
			dcLoad.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
			dcdcPower = dc_load_power;

		}

	double getPower() const;
	size_t getLength() const;
	double getVoltage() const;
	double getCurrent() const;
	double getPowerWithDCDC() const;
	TaskHandoff toTaskHandoff() const;
	TaskHandoffHEES toTaskHandoffHEES() const;
	std::vector<double> toPowerTrace() const;
	std::vector<double> toVoltageTrace() const;
	std::vector<double> toCurrentTrace() const;

private:
	double voltage;
	double current;
	size_t length;
	double dcdcPower;
};

#endif
