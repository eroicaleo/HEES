#ifndef _SWAP_SCHEDULING_
#define _SWAP_SCHEDULING_

#include <vector>
#include "../VoltageTable.hpp"
#include "../nnet/nnetmultitask.hpp"
#include "../TaskHandoff.hpp"

class SwapScheduling {

public:

	SwapScheduling(double initBankEnergy) :
		taskStartEnergy(1, initBankEnergy) {
		}

	void buildTaskTable(char *filename);
	void buildSolarPowerTrace();
	std::vector<double> generateChargeTrace();
	int compareTwoTasks(size_t i);
	void exhaustiveSwapping();
	void genScheduleForEES(std::string ees, std::string dp) const;
	void dumpOptimalSchedule();

	/**
	 * @return The number of tasks in this task set
	 */
	int getTaskNumber() const { return realTaskVoltageTable.size(); }

private:

	std::vector<TaskVoltageTable> realTaskVoltageTable;
	std::vector<double> solarPowerTrace;
	std::vector<double> taskPowerTrace;
	nnetmultitask nnetPredictor;

	/**
	 * The bank energy before each task starts
	 */
	std::vector<double> taskStartEnergy;

	std::vector<double> taskToPowerTrace(const TaskVoltageTable &tvt) const;
	std::vector<double> extractSolarPowerInterval(const std::vector<size_t> &coll) const;
	std::vector<double> extractTaskPowerInterval(const std::vector<size_t> &coll) const;

	double predictTasksEnergyInterval(const vector<double> &solarPowerInterval, const std::vector<size_t> &taskIndexColl);
	void addDCDCPower(vector<double> &powerTrace) const;
	double predictPowerInterval(const vector<double> &chargeTrace, double startEnergy);
	void buildTaskStartEnergy();
	double predictOneTask(size_t taskIndex);
	
};

#endif
