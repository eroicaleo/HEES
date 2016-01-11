#ifndef _SWAP_SCHEDULING_
#define _SWAP_SCHEDULING_

#include <vector>
#include "../VoltageTable.hpp"

class SwapScheduling {

public:

	void buildTaskTable(char *filename);
	void buildSolarPowerTrace();
	std::vector<double> generateChargeTrace();

private:

	std::vector<TaskVoltageTable> realTaskVoltageTable;
	std::vector<double> solarPowerTrace;
	std::vector<double> taskPowerTrace;

	std::vector<double> taskToPowerTrace(const TaskVoltageTable &tvt) const;
	int compareTwoTasks(size_t i);
	void addDCDCPower(vector<double> &powerTrace) const;
	double predictPowerInterval(const vector<double> &chargeTrace) const;

};

#endif
