#include <iostream>

#include "SwapScheduling.hpp"
#include "../DCCon_in.hpp"
#include "../ParseCommandLine.hpp"

using namespace std;

void SwapScheduling::buildTaskTable(char *filename) {

	VoltageTableDFS vt(vector<double>(1, 1.0), 1.0);
	BuildTaskVoltageTableVectorFromFile(filename, this->realTaskVoltageTable, vt);

	return;
}

vector<double> SwapScheduling::taskToPowerTrace(const TaskVoltageTable &tvt) const {
	size_t level = tvt.getNominalVoltageIndex();
	double power = tvt.getVoltage(level) * tvt.getCurrent(level);
	return vector<double>(tvt.getScaledCeilLength(level, 1), power);
}

/* 
 * Extract the solar power trace for task i and i+1
 */
vector<double> SwapScheduling::extractSolarPowerInterval(size_t i) const {
	int start = 0;
	int end = 0;
	for (size_t k = 0; k < i; ++k) {
		start += realTaskVoltageTable[k].getScaledCeilLength((size_t)0, 1);
	}
	end = start + realTaskVoltageTable[i].getScaledCeilLength((size_t)0, 1)
				+ realTaskVoltageTable[i+1].getScaledCeilLength((size_t)0, 1);

	vector<double>::const_iterator startIter = solarPowerTrace.begin() + start;
	vector<double>::const_iterator endIter = solarPowerTrace.begin() + end;
	return vector<double>(startIter, endIter);
}

/*
 * Convert 2 tasks to power trace
 * @param i the first task index
 * @param j the second task index
 * @return converted power trace for the 2 tasks
 */
vector<double> SwapScheduling::extractTaskPowerInterval(size_t i, size_t j) const {
	vector<double> taskPowerInterval;
	vector<double> trace = taskToPowerTrace(realTaskVoltageTable[i]);
	taskPowerInterval.insert(taskPowerInterval.end(), trace.begin(), trace.end());
	trace = taskToPowerTrace(realTaskVoltageTable[j]);
	taskPowerInterval.insert(taskPowerInterval.end(), trace.begin(), trace.end());
	return taskPowerInterval;
}

/**
 * Given two tasks i and j, and the corresponding solar power trace,
 * predicts the energy after finish these two tasks.
 * @param solarPowerTrace solor power trace
 * @param i the first task index
 * @param j the second task index
 * @return predicted energy
 */
double SwapScheduling::predictTwoTasksEnergyInterval(const vector<double> &solarPowerInterval, size_t i, size_t j) {

	vector<double> taskPowerInterval = extractTaskPowerInterval(i, j);

	// Make sure the two vectors are of the same size
	assert (solarPowerInterval.size() == taskPowerInterval.size());

	// Add the Load DC-DC power on top of task power trace
	addDCDCPower(taskPowerInterval);

	// Get real charge power
	vector<double> chargePowerInterval;
	transform(solarPowerInterval.begin(), solarPowerInterval.end(), taskPowerInterval.begin(), back_inserter(chargePowerInterval), minus<double>());

	// Make prediction for the power trace
	return predictPowerInterval(chargePowerInterval);
}

/**
 * Compare which task should be done first, task i or task i+1.
 * @param i task i.
 */
int SwapScheduling::compareTwoTasks(size_t i) {
	assert (i < realTaskVoltageTable.size()-1);

	vector<double> solarPowerInterval = extractSolarPowerInterval(i);

	double res = predictTwoTasksEnergyInterval(solarPowerInterval, i, i+1) -
					predictTwoTasksEnergyInterval(solarPowerInterval, i+1, i);

	if (res < 0)
		return -1;
	else if (res > 0)		
		return 1;

	return 0;
}
 
double SwapScheduling::predictPowerInterval(const vector<double> &chargeTrace) {

	// Assume the bank voltage is 1.0v
	double energy = 20.0;
	vector<double>::const_iterator start = chargeTrace.begin();
	vector<double>::const_iterator end;
	while (start != chargeTrace.end()) {
		end = adjacent_find(start, chargeTrace.end(), not_equal_to<double>());
		energy = nnetPredictor.predictWithEnergyLength(*start, energy, end-start);
	}

	return energy;
}

void SwapScheduling::buildSolarPowerTrace() {

	vector<double> taskPowerTrace;
	for (vector<TaskVoltageTable>::const_iterator it = realTaskVoltageTable.begin(); it != realTaskVoltageTable.end(); ++it) {
		vector<double> trace = taskToPowerTrace(*it);
		taskPowerTrace.insert(taskPowerTrace.end(), trace.begin(), trace.end());
	}

	solarPowerTrace.resize(taskPowerTrace.size());
	solarPowerTrace[0] = power_source_func(0);
	for (size_t i = 0; i < taskPowerTrace.size(); ++i) {
		solarPowerTrace[i] = power_source_func(i+1);
	}

}

void SwapScheduling::addDCDCPower(vector<double> &powerTrace) const {
	dcconvertIN dcLoad;

	double dc_load_vin(1.0), dc_load_vout(1.0), dc_load_iout(0.0);
	double dc_load_iin(0.0), dc_load_power(0.0);

	vector<double>::iterator start = powerTrace.begin();
	vector<double>::iterator end;

	while (start != powerTrace.end()) {
		dc_load_iout = *start;
		dcLoad.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
		end = adjacent_find(start, powerTrace.end(), not_equal_to<double>());
		transform(start, end, start, bind2nd(plus<double>(), dc_load_power));
		start = end;
	}

	return;
}

#ifdef SWAP_MAIN
int main(int argc, char *argv[]) {

	cout << "Hello swap!" << endl;

	hees_parse_command_line(argc, argv);
	SwapScheduling ss;
	ss.buildTaskTable("TasksSolar.txt.example");
	ss.buildSolarPowerTrace();
	return 0;
}
#endif
