#ifndef _SWAP_SCHEDULING_
#define _SWAP_SCHEDULING_

#include <vector>
#include "../VoltageTable.hpp"

class SwapScheduling {

public:

	void buildTaskTable(char *filename);
	void buildSolarPowerTrace();

private:

	std::vector<TaskVoltageTable> realTaskVoltageTable;
	std::vector<double> solarPowerTrace;
	std::vector<double> taskToPowerTrace(const TaskVoltageTable &tvt) const;

};

#endif
