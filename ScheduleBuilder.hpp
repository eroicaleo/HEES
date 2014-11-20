#ifndef _SCHEDULE_BUILDER_HPP_
#define _SCHEDULE_BUILDER_HPP_

#include <iostream>
#include <vector>

#include "DP/taskScheduling.hpp"

using std::vector;
using std::ostream;

class ScheduleBuilder {
public:
	ScheduleBuilder() : m_inaccurateFlag(false) {}
	// Leave all destructor, copy/move operations default
	void BuildScheduleFromFile(const char *filename);
	void PredictEnergyForSchedule(double initEnergy);
	void DumpSchedule(ostream &os = std::cout) const;
private:
	vector<dpTableEntry> m_schedule;
	bool m_inaccurateFlag;
	double getScheduleEnergy() const;
	int getScheduleFinishTime() const;
	double m_initialEnergy;
};

#endif
