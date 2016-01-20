#ifndef _SWAP_SCHEDULING_
#define _SWAP_SCHEDULING_

#include <vector>
#include "../VoltageTable.hpp"
#include "../nnet/nnetmultitask.hpp"
#include "../TaskHandoff.hpp"

class SwapScheduling {

public:

	void buildTaskTable(char *filename);
	void buildSolarPowerTrace();
	std::vector<double> generateChargeTrace();
	int compareTwoTasks(size_t i);
	void exhaustiveSwapping();
	void genScheduleForEES(std::string ees, std::string dp) const;

private:

	std::vector<TaskVoltageTable> realTaskVoltageTable;
	std::vector<double> solarPowerTrace;
	std::vector<double> taskPowerTrace;
	nnetmultitask nnetPredictor;

	std::vector<double> taskToPowerTrace(const TaskVoltageTable &tvt) const;
	std::vector<double> extractSolarPowerInterval(size_t i) const;
	std::vector<double> extractTaskPowerInterval(size_t i, size_t j) const;

	double predictTwoTasksEnergyInterval(const vector<double> &solarPowerInterval, size_t i, size_t j);
	void addDCDCPower(vector<double> &powerTrace) const;
	double predictPowerInterval(const vector<double> &chargeTrace);
	
};

#endif
