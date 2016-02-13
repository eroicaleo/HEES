#ifndef __TASK_HPP__
#define __TASK_HPP__

#include "../TaskHandoff.hpp"

class Task {

public:
	Task(double v, double c, size_t l):
		voltage(v),
		current(c),
		length(l) { }

	double getPower() const;
	size_t getLength() const;
	double getVoltage() const;
	double getCurrent() const;
	TaskHandoff toTaskHandoff() const;
	TaskHandoffHEES toTaskHandoffHEES() const;
	std::vector<double> toPowerTrace() const;
	std::vector<double> toVoltageTrace() const;
	std::vector<double> toCurrentTrace() const;

private:
	double voltage;
	double current;
	size_t length;
};

#endif
