#include <iostream>

#include "Task.hpp"

using namespace std;


double Task::getPower() const {
	return voltage * current;
}

size_t Task::getLength() const {
	return length;
}

double Task::getVoltage() const {
	return voltage;
}

double Task::getCurrent() const {
	return current;
}

double Task::getPowerWithDCDC() const {
	return voltage * current + dcdcPower;
}

/**
 * Convert the Task to a simpler TaskHandoff
 * @return a TaskHandoff object which only contains length, power and energy
 * info
 */
TaskHandoff Task::toTaskHandoff() const {
	size_t l = length;
	double p = getPower();
	double e = p * l;
	TaskHandoff t(l, p, e);

	return t;
}

/**
 * Convert the TaskVoltageTable to a simpler TaskHandoffHEES object
 * @return a TaskHandoffHEES object which contains an TaskHandoff object plus
 * voltage and current info
 */
TaskHandoffHEES Task::toTaskHandoffHEES() const {
	size_t l = length;
	double v = voltage;
	double c = current;
	double p = c * v;
	double e = p * l;
	TaskHandoffHEES t(l, p, e, v, c);

	return t;
}


/**
 * Convert the Task object to a vector of power trace
 * @return a vector of double types
 */
vector<double> Task::toPowerTrace() const {
	return vector<double>(length, current*voltage);
}

/**
 * Convert the Task object to a vector of voltage trace
 * @return a vector of double types
 */
vector<double> Task::toVoltageTrace() const {
	return vector<double>(length, voltage);
}

/**
 * Convert the Task object to a vector of current trace
 * @return a vector of double types
 */
vector<double> Task::toCurrentTrace() const {
	return vector<double>(length, current);
}
