#ifndef _SCHEDULE_BUILDER_HPP_
#define _SCHEDULE_BUILDER_HPP_

#include <iostream>
#include <vector>

#include "DP/taskScheduling.hpp"

using std::vector;
using std::ostream;

class ScheduleBuilder {
public:
	// Leave all constructor/destructor, copy/move operations default
	void BuildScheduleFromFile(const char *filename);
	void PredictEnergyForSchedule(double initEnergy);
	void DumpSchedule(ostream &os = std::cout) const;
private:
	vector<dpTableEntry> m_schedule;
	double getScheduleEnergy() const;
	int getScheduleFinishTime() const;
};

#endif
