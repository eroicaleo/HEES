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
 * Compare which task should be done first, task i or task i+1
 */
int SwapScheduling::compareTwoTasks(size_t i) {
	assert (i < realTaskVoltageTable.size()-1);

	// Extract the solar power trace for task i and i+1
	int start = 0;
	int end = 0;
	for (size_t k = 0; k < i; ++k) {
		start += realTaskVoltageTable[k].getScaledCeilLength((size_t)0, 1);
	}
	end = start + realTaskVoltageTable[i].getScaledCeilLength((size_t)0, 1)
				+ realTaskVoltageTable[i+1].getScaledCeilLength((size_t)0, 1);

	vector<double>::iterator startIter = solarPowerTrace.begin() + start;
	vector<double>::iterator endIter = solarPowerTrace.begin() + end;
	vector<double> solarPowerInterval(startIter, endIter);

	// Convert 2 tasks to power trace
	vector<double> taskPowerInterval;
	vector<double> trace = taskToPowerTrace(realTaskVoltageTable[i]);
	taskPowerInterval.insert(taskPowerInterval.end(), trace.begin(), trace.end());
	trace = taskToPowerTrace(realTaskVoltageTable[i+1]);
	taskPowerInterval.insert(taskPowerInterval.end(), trace.begin(), trace.end());

	// Make sure the two vectors are of the same size
	assert (solarPowerInterval.size() == taskPowerInterval.size());

	// Add the Load DC-DC power on top of task power trace
	addDCDCPower(taskPowerInterval);

	// Get real charge power
	vector<double> chargePowerInterval;
	transform(solarPowerInterval.begin(), solarPowerInterval.end(), taskPowerInterval.begin(), back_inserter(chargePowerInterval), minus<double>());

	// Make prediction for the power trace
	double energy = predictPowerInterval(chargePowerInterval);

	return 0;
}
 
double SwapScheduling::predictPowerInterval(const vector<double> &chargeTrace) const {

	double energy = 0.0;
	vector<double>::const_iterator start = chargeTrace.begin();

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
