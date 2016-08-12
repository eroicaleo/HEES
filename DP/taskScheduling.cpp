#include <string>
#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stack>
#include <vector>
#include <algorithm>

#include "taskScheduling.hpp"
#include "../nnet/nnetmultitask.hpp"
#include "../ScheduleBuilder.hpp"
#include "../ParseCommandLine.hpp"

using namespace std;
using namespace std::tr1;

extern const double CRAZY_ENERGY(-1000.0);

dynProg::dynProg(int numOfTask, int deadline, const vector<TaskVoltageTable> &vec_tvt) :
	realTaskVoltageTable(vec_tvt),
	m_scheduleWithIdleTask(vector<vector<dpTableEntry> >(numOfTask+numOfTask+1, vector<dpTableEntry>(deadline+1)))
{

	m_numOfTask = numOfTask;
	m_deadline = deadline;

	volSel = vec_tvt.begin()->getVoltageTable();
	m_numOfVolt = volSel.size();

	m_taskDuration = vector<vector<int> >(numOfTask, vector<int>(m_numOfVolt, 0));
	m_taskCurrent = vector<vector<double> >(numOfTask, vector<double>(m_numOfVolt, 0.0));

	m_scheduleEnergy = vector<vector<double> >(numOfTask, vector<double>(m_deadline + 1 , 0.0));

	for (int i = 0; i < m_numOfTask; i++) {
		// cout<<"Task "<<i<<":";
		for (int k = 0; k < m_numOfVolt; k++) {
			m_taskDuration[i][k] = vec_tvt[i].getScaledCeilLength(volSel[k], 1);
			m_taskCurrent[i][k] = vec_tvt[i].getCurrent(volSel[k]);
		}
	}

	// System model related parameters
	m_initialEnergy = 0.5 * (supcap_init_charge*supcap_init_charge) / 40.0;

	// Initialize DP table with idle task
	for (size_t i = 0; i < m_scheduleWithIdleTask.size(); i += 2) {
		for (size_t j = 0; j != m_scheduleWithIdleTask[i].size(); ++j) {
			// Idle tasks
			m_scheduleWithIdleTask[i][j].taskID = i/2;
			// Real tasks
			if (i+1 < m_scheduleWithIdleTask.size()) {
				m_scheduleWithIdleTask[i+1][j].taskID = i/2;
			}
		}
	}

	// Initialize solar power for each dp table entry
	initDPSolarPower();
}

void dynProg::initDPSolarPower() {

	for (size_t j = 0; j < m_scheduleWithIdleTask[0].size(); ++j) {
		double sp = power_source_func(j);
		m_solarPower = sp;
		for (size_t i = 0; i < m_scheduleWithIdleTask.size(); ++i) {
			m_scheduleWithIdleTask[i][j].solarPower = sp;
		}
	}

#ifdef DEBUG_VERBOSE
	dumpDPTable();
#endif
}

void dynProg::taskTimelineWithIdle() {
	// initialize the first task -- idle task
	populateFirstIdleTask(m_scheduleWithIdleTask[0]);
	for (size_t i = 0; i < m_scheduleWithIdleTask.size()-1; i += 2) {
		// populating for the even number tasks -- real tasks
		populateRealTask(m_scheduleWithIdleTask[i], m_scheduleWithIdleTask[i+1]);
		// populating for the odd number tasks -- idle tasks
		populateIdleTask(m_scheduleWithIdleTask[i+1], m_scheduleWithIdleTask[i+2]);
	}
#ifdef DEBUG_VERBOSE
	dumpDPTable();
#endif
}

void dynProg::populateFirstIdleTask(vector<dpTableEntry> &firstIdleRow) {
	for (tableEntryIter iter = firstIdleRow.begin(); iter != firstIdleRow.end(); ++iter) {
		size_t len = (iter-firstIdleRow.begin());

		iter->totalEnergy = energyCalVarSolarPowerWrapper(1+firstIdleRow.begin(),
								1+iter, idleTaskVoltageTable,
								idleTaskVoltageTable.getNominalVoltageIndex(),
								m_initialEnergy);
		iter->len = len;
	}
	return;
}

void dynProg::populateIdleTask(const vector<dpTableEntry> &lastRealRow, vector<dpTableEntry> &thisIdleRow) {

	for (vector<dpTableEntry>::const_iterator iter = lastRealRow.begin(); iter != lastRealRow.end(); ++iter) {
		if (iter->totalEnergy < 0)
			continue;

		/** Here is a diagram for how to pass parameters to energyCalVarSolarPowerWrapper
		 *
		 *  |________|________|________|________|________|________|________|________|
		 *          iter
		 *  |________|________|________|________|________|________|________|________|
		 *            iterIdleHead(begin)            iterIdle   (end)
		 */

		tableEntryIter iterIdleHead = thisIdleRow.begin()+(iter-lastRealRow.begin());
		// Update the idle table entry if current energy is less
		if (iter->totalEnergy > iterIdleHead->totalEnergy) {
			iterIdleHead->setAllFields(iter->totalEnergy, 1.0, 0.0, -1, iterIdleHead->taskID, 0, iter-lastRealRow.begin());
#ifdef DEBUG_VERBOSE
			cout << "Idle task: " << iterIdleHead->taskID << " @ time "
				<< iterIdleHead - thisIdleRow.begin() << " initEnergy: "
				<< iter->totalEnergy << "." << endl;
#endif
		}

		// Move the iterator one right, that's the end of the first second of the idle task
		iterIdleHead += 1;
		for (tableEntryIter iterIdle = iterIdleHead; iterIdle != thisIdleRow.end(); ++iterIdle) {
			int taskDur = 1+iterIdle-iterIdleHead;
#ifdef DEBUG_VERBOSE
			cout << "Idle task: I am predicting idle task " << iterIdle->taskID << " from time " << iterIdleHead - thisIdleRow.begin()
				<< " to time " << iterIdle - thisIdleRow.begin() << ", Idle task length is " << taskDur << endl;
#endif
			double energy = energyCalVarSolarPowerWrapper(iterIdleHead, iterIdle+1,
							idleTaskVoltageTable, idleTaskVoltageTable.getNominalVoltageIndex(),
							iter->totalEnergy);

			if (energy > iterIdle->totalEnergy) {
#ifdef DEBUG_VERBOSE
				cout << "Idle task: Taskid: idle " << iterIdle->taskID << " entry " << iterIdle-thisIdleRow.begin() << " has been updated from "
					<< iterIdle->totalEnergy << " to " << energy << endl;
#endif
				// Update the table entry
				iterIdle->setAllFields(energy, 1.0, 0.0, -1, iterIdle->taskID, taskDur, iter-lastRealRow.begin());
			}
		}
	}
	return;
}

/**
 * This function predicts the energy for a task even when the solar power varies
 * across the task execution time.
 * Input:
 *   begin: iterator points to the dp table entry when task begins
 *   end  : iterator points to the dp table entry when task ends
 *   volLevel : the voltage level which the task is executed
 *   initEnergy : the energy stored in the EES bank before running this task
 * Output:
 *   The predicted energy stored in the EES bank
 *
 * Here is an example on how to pass begin and end parameters
 * Say a task lasts 4 seconds from 0 to 4, if we want to predict the energy
 * We need to pass (begin = 1, end = 5), **not** (begin = 0, end = 4)
 *
 * |__|__|__|__|__|
 * 0  1  2  3  4  5
 *
 */
double dynProg::energyCalVarSolarPowerWrapper(tableEntryIter begin, tableEntryIter end, const TaskVoltageTable &tvt, size_t volLevel, double initEnergy) {
	double endEnergy(initEnergy);
	tableEntryIter next(begin);
	while (begin < end) {
		next = find_if(begin, end, dpSolarComparator(begin->solarPower));
		double inputPower = getExtraChargePower(tvt, volLevel, begin->solarPower);
		if (inputPower < 0) {
			endEnergy = CRAZY_ENERGY;
			break;
		}
		// the user need to carefully pass the begin and end iterator
		// See the example above and below
#ifdef DEBUG_VERBOSE
		cout << "VSP: "
			<< ", inputPower: " << inputPower
			<< ", solarPower: " << begin->solarPower
			<< ", length: " << (next-begin)
			<< ", startEnergy: " << endEnergy;
#endif
		endEnergy = energyCalculatorWrapper(inputPower, endEnergy, next-begin);
		begin = next;

#ifdef DEBUG_VERBOSE
		cout << ", endEnergy: " << endEnergy << endl;
#endif
	}
	return endEnergy;
}

void dynProg::populateRealTask(const vector<dpTableEntry> &lastIdleRow, vector<dpTableEntry> &thisRealRow) {
	for (vector<dpTableEntry>::const_iterator iter = lastIdleRow.begin(); iter != lastIdleRow.end(); ++iter) {
		if (iter->totalEnergy < 0)
			continue;

		tableEntryIter realTaskIterBegin = thisRealRow.begin() + (iter-lastIdleRow.begin()) + 1;
		for (int k = 0; k < m_numOfVolt; ++k) {
			int taskID = thisRealRow[0].taskID;
			int taskDur = m_taskDuration[taskID][k];
			size_t taskiFinishTime = (iter-lastIdleRow.begin()) + taskDur;
			tableEntryIter realTaskIterEnd = thisRealRow.begin() + taskiFinishTime;
			double inputPower = getExtraChargePower(realTaskVoltageTable[taskID], k, realTaskIterBegin->solarPower);

			// Must guarantee there is enough power for charging
			if ((inputPower > 0.0) && (realTaskIterEnd < thisRealRow.end())) {
#ifdef DEBUG_VERBOSE
				cout << "I am predicting real task " << taskID << " from time " << (iter-lastIdleRow.begin()) << " to " << taskiFinishTime << "."
					<< " The start energy is: " << iter->totalEnergy << "." << endl;
#endif

				/** Here is a diagram for how to pass parameters to energyCalVarSolarPowerWrapper
				 *
				 *  |________|________|________|________|________|________|________|________|
				 *          iter
				 *  |________|________|________|________|________|________|________|________|
				 *            realBegin(begin)                realEnd   (end)
				 */

				double energy = energyCalVarSolarPowerWrapper(realTaskIterBegin, realTaskIterEnd+1,
																realTaskVoltageTable[taskID], k,
																iter->totalEnergy);

				// Must guarantee that the schedule is feasible
				if (realTaskIterEnd->totalEnergy < energy) {
#ifdef DEBUG_VERBOSE
					cout << "Taskid: " << taskID << " entry " << taskiFinishTime << " has been updated from "
						<< realTaskIterEnd->totalEnergy << " to " << energy << endl;;
#endif
					// Update the table entry
					realTaskIterEnd->setAllFields(energy, volSel[k], m_taskCurrent[taskID][k], k, taskID, taskDur, taskiFinishTime-taskDur);
				}
			}
#ifdef DEBUG_VERBOSE
			else if (!above_min_valid_input_power(inputPower)) {
				cout << "I am predicting real task " << taskID << " from time " << (iter-lastIdleRow.begin()) << " to " << taskiFinishTime << ". ";
				cout << "However power is below valid range: " << inputPower << "." << endl;
			}
#endif
		} // iterate over 'k': number of voltage
	}
	return;
}

double dynProg::getExtraChargePower(int taskIdx, int volLevel, double solarPower) {

	double dc_load_vin(1.0), dc_load_vout(volSel[volLevel]), dc_load_iout(m_taskCurrent[taskIdx][volLevel]);
	double dc_load_iin(0.0), dc_load_power(0.0);

	m_dcLoad.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
	double chargingPower = solarPower - dc_load_vin*dc_load_iin;
	return chargingPower;
}

double dynProg::getExtraChargePower(const TaskVoltageTable &tvt, size_t volLevel, double solarPower) {

	double dc_load_vin(1.0), dc_load_vout(tvt.getVoltage(volLevel)), dc_load_iout(tvt.getCurrent(volLevel));
	double dc_load_iin(0.0), dc_load_power(0.0);

	m_dcLoad.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
	double chargingPower = solarPower - dc_load_vin*dc_load_iin;
	return chargingPower;
}

void dynProg::findMaxEnergyTableEntry(tableRowRIter &row, tableEntryIter &col) {

	row = m_scheduleWithIdleTask.rbegin();

	tableEntryIter optIdxIdleTask = max_element(row->begin(), row->end(), dpTableEntryComp);
	++row;
	tableEntryIter optIdxRealTask = max_element(row->begin(), row->end(), dpTableEntryComp);
	if (*optIdxRealTask < *optIdxIdleTask) {
		--row;
		col = optIdxIdleTask;
	} else {
		col = optIdxRealTask;
	}

	return;
}

void dynProg::backTracingWithIdleTasks() {

	// Find the entry in the table with optimal energy
	tableRowRIter iter;
	tableEntryIter entryIter;
	findMaxEnergyTableEntry(iter, entryIter);

	// Probably there is no feasible schedule.
	// We just return.
	if (entryIter->totalEnergy < 0.0)
		return;

	// Backing tracing loop
	optimalSchedule.push(*entryIter);
	for (++iter; iter != m_scheduleWithIdleTask.rend(); ++iter) {
		entryIter = iter->begin() + optimalSchedule.top().lastTaskFinishTime;
		optimalSchedule.push(*entryIter);
	}
}

void dynProg::dumpDPTable() {

	cout << endl;
	cout << "#########################################################" << endl;
	cout << "############### Begin to dump DP Table! #################" << endl;
	cout << "#########################################################" << endl;
	for (size_t i = 0; i != m_scheduleWithIdleTask.size(); ++i) {
		for (size_t j = 0; j != m_scheduleWithIdleTask[i].size(); ++j) {
			cout << "row: " << std::setw(4) << i << " col: " << std::setw(4) << j << " ";
			cout << m_scheduleWithIdleTask[i][j] << endl;
		}
	}
	cout << "#########################################################" << endl;
	cout << "###############  End to dump DP Table!  #################" << endl;
	cout << "#########################################################" << endl;
	cout << endl;

}

void dynProg::dumpOptimalSchedule() {

	stack<dpTableEntry> tmpSchedule(optimalSchedule);
	cout << endl;
	cout << "#########################################################" << endl;
	cout << "############### Begin to dump schedule! #################" << endl;
	cout << "#########################################################" << endl;
	if (tmpSchedule.empty()) {
		cout << "Error: the schedule is infeasible!" << endl;
	}
	while (!tmpSchedule.empty()) {
		cout << tmpSchedule.top() << endl;
		tmpSchedule.pop();
	}
	cout << "Final Energy: " << getOptimalScheduleEnergy();
	cout << " Finish Time: " << getOptimalScheduleFinishTime() << endl;
	cout << "#########################################################" << endl;
	cout << "###############  End to dump schedule!  #################" << endl;
	cout << "#########################################################" << endl;
	cout << endl;

}

int dynProg::getOptimalScheduleFinishTime() const {
	int t = -1;
	stack<dpTableEntry> tmpSchedule(optimalSchedule);
	while (!tmpSchedule.empty()) {
		t = tmpSchedule.top().len + tmpSchedule.top().lastTaskFinishTime;
		tmpSchedule.pop();
	}
	return t;
}

double dynProg::getOptimalScheduleEnergy() const {
	double e = 0.0;
	stack<dpTableEntry> tmpSchedule(optimalSchedule);
	while (!tmpSchedule.empty()) {
		e = tmpSchedule.top().totalEnergy;
		tmpSchedule.pop();
	}
	return e;
}

int dynProg::genScheduleForEES() {
	ofstream outfile;
	int totalLength = 0;
	outfile.open("TasksDPForEES.txt");
	if (!outfile) {
		cerr << "can not open TasksDP.txt for write" << endl;
		exit(66);
	}
	stack<dpTableEntry> tmpSchedule(optimalSchedule);
	while(!tmpSchedule.empty()) {
		dpTableEntry tmpTaskEntry = tmpSchedule.top();
		if (tmpTaskEntry.len > 0) {
			outfile << tmpTaskEntry.voltage << " ";
			outfile << tmpTaskEntry.current << " ";
			outfile << (tmpTaskEntry.len) << endl;
		}
		tmpSchedule.pop();
		totalLength += tmpTaskEntry.len;
	}
	outfile.close();

	return totalLength;
}

static nnetmultitask nnetPredictor;
void dynProg::dynamicProgrammingWithIdleTasks() {

	energyCalculator = bind(&nnetmultitask::predictWithEnergyLength, nnetPredictor, placeholders::_1, placeholders::_2, placeholders::_3);
	taskTimelineWithIdle();
	backTracingWithIdleTasks();
	genScheduleForEES();
	dumpOptimalSchedule();

}

const size_t pxaVoltageLevel = 4;
const double pxaVoltageTable[pxaVoltageLevel] = {0.75, 1.0, 1.3, 1.6};

dpTableEntry::dpTableEntry():
	totalEnergy(CRAZY_ENERGY),
	voltage(0.0),
	current(0.0),
	volLevel(-1),
	taskID(-1),
	len(-1),
	lastTaskFinishTime(0),
	solarPower(0.0) {}

dpTableEntry::dpTableEntry(const dpTableEntry &e):
	totalEnergy(e.totalEnergy),
	voltage(e.voltage),
	current(e.current),
	volLevel(e.volLevel),
	taskID(e.taskID),
	len(e.len),
	lastTaskFinishTime(e.lastTaskFinishTime),
	solarPower(e.solarPower) {}

ostream& operator<<(ostream& os, const dpTableEntry &e) {
	os << "In this entry, ";
	os.precision(4);
	os << "totalEnergy: " << std::fixed << std::setw(12) << e.totalEnergy;
	os << ", voltage: " << std::setprecision(2) << std::fixed << std::setw(8) << e.voltage;
	os << ", current: " << std::setw(8) << e.current;
	os << ", volLevel: " << std::setw(4) << e.volLevel;
	os << ", taskID: " << std::setw(4) << e.taskID;
	os << ", len: " << std::setw(4) << e.len;
	os << ", solarPower: " << std::setw(8) << e.solarPower;
	os << ", lastTaskFinishTime: " << e.lastTaskFinishTime;
	os << ".";
	return os;
}

bool dpTableEntryComp(const dpTableEntry &a, const dpTableEntry &b) {
	return a.totalEnergy < b.totalEnergy;
}

double dynProg::energyCalculatorWrapper(double inputPower, double initialEnergy, double len) {
	if (inputPower <= 0.0)
		return 0.0;

	energyCalculator = bind(&nnetmultitask::predictWithEnergyLength, nnetPredictor, placeholders::_1, placeholders::_2, placeholders::_3);

	return energyCalculator(inputPower, initialEnergy, len);
}

double energyCalculatorApproximation(double inputPower, double initialEnergy, double len) {
	return initialEnergy + inputPower * len * 0.8;
}

#ifdef DP_BINARY

void readInput(vector<double> &InDuration, vector<double> &InEnergy, double &deadline) {
	using namespace std;
	ifstream infile;
	double tasklen, power, energy;
	deadline = 0.0;
	infile.open("TasksOrig.txt");
	if (!infile) {
		cerr << "Can not open TasksOrig.txt for read!" << endl;
		exit(66);
	}

	while ((infile >> tasklen >> power >> energy).good()) {
		InDuration.push_back(tasklen);
		InEnergy.push_back(energy);
		deadline += tasklen;
	}

	deadline /= ratio_runtime_and_deadline;

	infile.close();
	return;
}

/* Testbench */
int main(int argc, char *argv[]){
	double deadline = 25;
	vector<double>InDuration;
	// InDuration.push_back(6.0);
	// InDuration.push_back(7.0);
	// InDuration.push_back(8.0);

	vector<double>InEnergy;
	// InEnergy.push_back(6.0);
	// InEnergy.push_back(7.0);
	// InEnergy.push_back(8.0);

	vector<TaskVoltageTable> vec_tvt;

	hees_parse_command_line(argc, argv);
	readInput(InDuration, InEnergy, deadline);
	BuildTaskVoltageTableVectorFromFile("TasksOrig.txt", vec_tvt, *cpu_voltage_table_ptr);

	if (scheduling_deadline > 0) {
		deadline = scheduling_deadline;
	}

	vector<double>outDuration;
	vector<double>outVolt;
	dynProg taskSet1 (InDuration.size(), (int)deadline, vec_tvt);
	taskSet1.dynamicProgrammingWithIdleTasks();
	ScheduleBuilder sb;
	sb.BuildScheduleFromFile("TasksDP.txt");
	sb.PredictEnergyForSchedule(0.5 * (supcap_init_charge*supcap_init_charge) / 40.0);
	sb.DumpSchedule();
}

#endif
