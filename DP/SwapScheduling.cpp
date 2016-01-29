#include <iomanip>
#include <iostream>
#include <typeinfo>

#include <tr1/functional>

#include "SwapScheduling.hpp"
#include "../DCCon_in.hpp"
#include "../ParseCommandLine.hpp"

using namespace std;
using namespace std::tr1;

void SwapScheduling::buildTaskTable(char *filename) {

	VoltageTableDFS vt(vector<double>(1, 1.0), 1.0);
	BuildTaskVoltageTableVectorFromFile(filename, this->realTaskVoltageTable, vt);

	return;
}

vector<double> SwapScheduling::taskToPowerTrace(const TaskVoltageTable &tvt) const {
	size_t level = 0;
	double power = tvt.getPower(level);
	return vector<double>(tvt.getScaledCeilLength(level, 1), power);
}

/**
 * Extract the solar power trace for tasks specified in coll.
 * These tasks have to be adjacent. Currently only support up to 2 tasks.
 *
 * @param coll the index of tasks for which sum power will be extracted
 * @return the solar power interval
 */
vector<double> SwapScheduling::extractSolarPowerInterval(const vector<size_t> &coll) const {
	assert ((coll.size() <= 2) && (coll.size() > 0));

	if (coll.size() == 2)
		assert (((coll[1] - coll[0]) == 1) && (coll[0] < realTaskVoltageTable.size()-1));

	int start = 0;
	int end = 0;
	for (size_t k = 0; k < coll[0]; ++k) {
		start += realTaskVoltageTable[k].getScaledCeilLength((size_t)0, 1);
	}
	end = start;
	for (size_t k = 0; k < coll.size(); ++k) {
		end = end + realTaskVoltageTable[coll[k]].getScaledCeilLength((size_t)0, 1);
	}

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
vector<double> SwapScheduling::extractTaskPowerInterval(const vector<size_t> &taskIndexColl) const {
	vector<double> taskPowerInterval, trace;

	vector<size_t>::const_iterator pos(taskIndexColl.begin());
	for ( ; pos != taskIndexColl.end(); ++pos) {
		trace = taskToPowerTrace(realTaskVoltageTable[*pos]);
		taskPowerInterval.insert(taskPowerInterval.end(), trace.begin(), trace.end());
	}

	return taskPowerInterval;
}

/**
 * Given a collection of tasks, and the corresponding solar power trace,
 * predicts the energy after finish these two tasks.
 *
 * @param solarPowerTrace solor power trace
 * @param taskIndexColl the task index collection
 * @return predicted energy
 */
double SwapScheduling::predictTasksEnergyInterval(const vector<double> &solarPowerInterval, const vector<size_t> &taskIndexColl) {

	vector<double> taskPowerInterval = extractTaskPowerInterval(taskIndexColl);

	// Make sure the two vectors are of the same size
	assert (solarPowerInterval.size() == taskPowerInterval.size());

	// Add the Load DC-DC power on top of task power trace
	addDCDCPower(taskPowerInterval);

	// Get real charge power
	vector<double> chargePowerInterval;
	transform(solarPowerInterval.begin(), solarPowerInterval.end(), taskPowerInterval.begin(), back_inserter(chargePowerInterval), minus<double>());

	// Make prediction for the power trace
	// Note that since the taskIndexColl might be swapped
	// We need to find the minimum task index to get the start energy
	size_t minIndex = *min_element(taskIndexColl.begin(), taskIndexColl.end());
	return predictPowerInterval(chargePowerInterval, taskStartEnergy[minIndex]);
}

/**
 * Compare which task should be done first, task i or task i+1.
 * @param i task i.
 * @return -1 if doing i+1 is better, +1 if doing i is better, 0 if tie.
 */
int SwapScheduling::compareTwoTasks(size_t i) {
	assert (i < realTaskVoltageTable.size()-1);

	vector<size_t> taskPair(1, i);
	taskPair.push_back(i+1);

	vector<double> solarPowerInterval = extractSolarPowerInterval(taskPair);

	int hwlf = highWorkLoadFirstTwoTasks(solarPowerInterval, taskPair);
	if (hwlf != 0)
		return hwlf;

	double res0 = predictTasksEnergyInterval(solarPowerInterval, taskPair);
	swap(taskPair[0], taskPair[1]);
	double res1 = predictTasksEnergyInterval(solarPowerInterval, taskPair);

	#ifdef DEBUG_VERBOSE
		cout << "Doing task " << i << " first, energy: " << res0 << endl;
		cout << "Doing task " << i << " later, energy: " << res1 << endl;
	#endif

	double res =  res0 - res1;

	if (res < 0)
		return -1;
	else if (res > 0)
		return 1;

	return 0;
}

/**
 * Exhaustively search all adjacent pairs of tasks,
 * if swapping the task can be beneficial to energy, then swap them.
 */
void SwapScheduling::exhaustiveSwapping() {

	int numOfSwap;
	do {
		numOfSwap = 0;
		for (size_t i = 0; i < realTaskVoltageTable.size()-1; ++i) {
			int res = compareTwoTasks(i);
			if (res < 0) {
				swap(realTaskVoltageTable[i], realTaskVoltageTable[i+1]);
				// updates the start energy for task i+1 and i+2
				taskStartEnergy[i+1] = predictOneTask(i);
				if (i < realTaskVoltageTable.size()-2)
					taskStartEnergy[i+2] = predictOneTask(i+1);

				++numOfSwap;
			}
		}
	} while (numOfSwap > 0);

	dumpOptimalSchedule();

	return;
}

/**
 * Generate the TasksSCHED.txt for HEES simulator or handoff to dynamic
 * programming DVFS algorithm
 */
void SwapScheduling::genScheduleForEES(string ees, string dp) const {
	vector<TaskHandoffHEES> taskHandoffSetHEES;
	transform(realTaskVoltageTable.begin(), realTaskVoltageTable.end(),
		back_inserter(taskHandoffSetHEES), bind(&TaskVoltageTable::toTaskHandoffHEES, placeholders::_1, 0));

	genScheduleTaskHandoffSet(taskHandoffSetHEES, ees);

	vector<TaskHandoff> taskHandoffSet;
	transform(realTaskVoltageTable.begin(), realTaskVoltageTable.end(),
		back_inserter(taskHandoffSet), bind(&TaskVoltageTable::toTaskHandoff, placeholders::_1, 0));

	genScheduleTaskHandoffSet(taskHandoffSet, dp);

	return;
}

void SwapScheduling::dumpOptimalSchedule() {
	cout.precision(4);
	for (size_t i = 0; i < taskStartEnergy.size(); ++i) {
		cout << "Task: " << fixed << setw(4) << i
			<< " Energy: " << fixed << setw(8) << taskStartEnergy[i] << endl;
	}
	cout << "Final Energy: " << fixed << setw(8) << predictOneTask(taskStartEnergy.size()-1) << endl;
}

/**
 * Given a vector of charge trace, predict the energy stored in the EES bank
 * after this charging trace
 * @param chargeTrace the charging power trace
 * @return the energy stored in the EES bank
 */
double SwapScheduling::predictPowerInterval(const vector<double> &chargeTrace, double startEnergy) {

	double energy = startEnergy;
	vector<double>::const_iterator start = chargeTrace.begin();
	vector<double>::const_iterator end;
	while (start != chargeTrace.end()) {
		end = find_if(start, chargeTrace.end(), bind1st(not_equal_to<double>(), *start));
		energy = nnetPredictor.predictWithEnergyLength(*start, energy, end-start);
		start = end;
	}

	return energy;
}

/**
 * Based on the intial schedule, predict the start energy of each task.
 * And fill the taskStartEnergy data member.
 */
void SwapScheduling::buildTaskStartEnergy() {

	assert (taskStartEnergy.size() == 1);

	for (size_t i = 0; i < realTaskVoltageTable.size()-1; ++i) {
		double res = predictOneTask(i);
		taskStartEnergy.push_back(res);
	}

	return;
}

/**
 * Given a task index, predict the energy after finishing this task
 *
 * @param taskIndex
 * @return the predicted energy
 */
double SwapScheduling::predictOneTask(size_t taskIndex) {

		vector<size_t> taskIndexColl(1, taskIndex);
		vector<double> solarPowerInterval = extractSolarPowerInterval(taskIndexColl);
		double res = predictTasksEnergyInterval(solarPowerInterval, taskIndexColl);

		return res;
}

/**
 * Given two tasks, and the corresponding solar power trace,
 * if solar power is monotonous non-decreasing, check if the
 * two tasks are high work load first
 *
 * @param solarPowerTrace solor power trace
 * @param taskIndexColl the task index collection
 * @return 1 if high workload first or not monotonous non-decreasing
 * else return -1
 */
int SwapScheduling::highWorkLoadFirstTwoTasks(const std::vector<double> &solarPowerInterval, const std::vector<size_t> &taskIndexColl) {

	assert (taskIndexColl.size() == 2);

	bool isSorted = (adjacent_find(solarPowerInterval.begin(),
									solarPowerInterval.end(),
									greater<double>()) == solarPowerInterval.end());

	if (!isSorted) {
		#ifdef DEBUG_VERBOSE
			cout << "solar power is NOT monotonous non-decreasing, leave it to predictor!" << endl;
		#endif
		return 0;
	}

	if (realTaskVoltageTable[taskIndexColl[0]].getPower(0)
		< realTaskVoltageTable[taskIndexColl[1]].getPower(0)) {
		#ifdef DEBUG_VERBOSE
			cout << "solar power is monotonous non-decreasing, but not HWLF, need to swap "
				<< taskIndexColl[0] << " and " << taskIndexColl[1] << endl;
		#endif
		return -1;
	} else {
		#ifdef DEBUG_VERBOSE
			cout << "solar power is monotonous non-decreasing, and is HWLF, no need to swap "
				<< taskIndexColl[0] << " and " << taskIndexColl[1] << endl;
		#endif
		return 1;
	}
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

	buildTaskStartEnergy();

	return;
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
		end = find_if(start, powerTrace.end(), bind1st(not_equal_to<double>(), *start));
		transform(start, end, start, bind2nd(plus<double>(), dc_load_power));
		start = end;
	}

	return;
}

#ifdef SWAP_MAIN
int main(int argc, char *argv[]) {

	cout << "Hello swap!" << endl;

	hees_parse_command_line(argc, argv);

	double cap = supcap_cap * supcap_parallel_conf / supcap_serial_conf;
	double initEnergy = 0.5 * (supcap_init_charge * supcap_init_charge) / cap;
	SwapScheduling ss(initEnergy);

	ss.buildTaskTable("TasksSolar.txt");
	ss.genScheduleForEES("TasksSCHEDForEES.txt.init", "TasksSCHEDForDP.txt.init");
	ss.buildSolarPowerTrace();
	ss.exhaustiveSwapping();
	ss.genScheduleForEES("TasksSCHEDForEES.txt", "TasksSCHEDForDP.txt");
	return 0;
}
#endif
