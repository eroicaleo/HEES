#ifndef _SWAP_SCHEDULING_
#define _SWAP_SCHEDULING_

#include <vector>
#include "../VoltageTable.hpp"
#include "../nnet/nnetmultitask.hpp"

class SwapScheduling {

public:

	void buildTaskTable(char *filename);
	void buildSolarPowerTrace();
	std::vector<double> generateChargeTrace();

private:

	std::vector<TaskVoltageTable> realTaskVoltageTable;
	std::vector<double> solarPowerTrace;
	std::vector<double> taskPowerTrace;
	nnetmultitask nnetPredictor;

	std::vector<double> taskToPowerTrace(const TaskVoltageTable &tvt) const;
	std::vector<double> extractSolarPowerInterval(size_t i) const;
	std::vector<double> extractTaskPowerInterval(size_t i, size_t j) const;

	double predictTwoTasksEnergyInterval(const vector<double> &solarPowerInterval, size_t i, size_t j);
	int compareTwoTasks(size_t i);
	void addDCDCPower(vector<double> &powerTrace) const;
	double predictPowerInterval(const vector<double> &chargeTrace);
	
};

#endif
