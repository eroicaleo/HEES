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

dynProg::dynProg(int numOfTask, int deadline, const vector<TaskVoltageTable> &vec_tvt) {

	m_numOfTask = numOfTask;
	m_deadline = deadline;

	volSel = vec_tvt.begin()->getVoltageTable();
	m_numOfVolt = volSel.size();

	m_taskDuration = vector<vector<int> >(numOfTask, vector<int>(m_numOfVolt, 0));
	m_taskCurrent = vector<vector<double> >(numOfTask, vector<double>(m_numOfVolt, 0.0));

	m_scheduleEnergy = vector<vector<double> >(numOfTask, vector<double>(m_deadline + 1 , 0.0));
	m_scheduleVolt =  vector<vector<double> >(numOfTask, vector<double>(m_deadline + 1, 0.0));
	m_lastStepDuration = vector<vector<double> >(numOfTask, vector<double>(m_deadline + 1, 0.0));

	for (int i = 0; i < m_numOfTask; i++) {
		// cout<<"Task "<<i<<":";
		for (int k = 0; k < m_numOfVolt; k++) {
			m_taskDuration[i][k] = vec_tvt[i].getScaledCeilLength(volSel[k], 1);
			m_taskCurrent[i][k] = vec_tvt[i].getCurrent(volSel[k]);
		}
	}

	// System model related parameters
	m_initialEnergy = 0.5 * (supcap_init_charge*supcap_init_charge) / 40.0;
	m_solarPower = power_source_func(0.0);

	// Initialize DP table with idle task
	m_scheduleWithIdleTask = vector<vector<dpTableEntry> >(m_numOfTask+m_numOfTask+1, vector<dpTableEntry>(m_deadline+1, dpTableEntry()));
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
		double inputPower = getExtraChargePower(idleTaskVoltageTable, idleTaskVoltageTable.getNominalVoltageIndex());

		if (inputPower < 0)
			return;

		iter->totalEnergy = energyCalculatorWrapper(inputPower, m_initialEnergy, len);
		iter->len = len;
	}
	return;
}

void dynProg::populateIdleTask(const vector<dpTableEntry> &lastRealRow, vector<dpTableEntry> &thisIdleRow) {

	double inputPower = getExtraChargePower(idleTaskVoltageTable, idleTaskVoltageTable.getNominalVoltageIndex());
	if (inputPower < 0)
		return;

	for (vector<dpTableEntry>::const_iterator iter = lastRealRow.begin(); iter != lastRealRow.end(); ++iter) {
		if (iter->totalEnergy < 0)
			continue;
		tableEntryIter iterIdleHead = thisIdleRow.begin()+(iter-lastRealRow.begin());
		for (tableEntryIter iterIdle = iterIdleHead; iterIdle != thisIdleRow.end(); ++iterIdle) {
			int taskDur = iterIdle - iterIdleHead;
#ifdef DEBUG_VERBOSE
			cout << "I am predicting idle task " << iterIdle->taskID << " from time " << iterIdleHead - thisIdleRow.begin() << " to " << iterIdle - thisIdleRow.begin() << "." << " inputPower: " << inputPower << ". taskDur: " << taskDur << ". iter->totalEnergy: " << iter->totalEnergy << "." << endl;
#endif
			double energy = energyCalculatorWrapper(inputPower, iter->totalEnergy, taskDur);
			if (energy > iterIdle->totalEnergy) {
#ifdef DEBUG_VERBOSE
				cout << "Taskid: idle " << iterIdle->taskID << " entry " << iterIdle-thisIdleRow.begin() << " has been updated from "
					<< iterIdle->totalEnergy << " to " << energy << endl;;
#endif
				// Update the table entry
				iterIdle->setAllFields(energy, 1.0, 0.0, -1, iterIdle->taskID, taskDur, iterIdleHead-thisIdleRow.begin());
			}
		}
	}
	return;
}

void dynProg::populateRealTask(const vector<dpTableEntry> &lastIdleRow, vector<dpTableEntry> &thisRealRow) {
	for (vector<dpTableEntry>::const_iterator iter = lastIdleRow.begin(); iter != lastIdleRow.end(); ++iter) {
		if (iter->totalEnergy < 0)
			continue;
		for (int k = 0; k < m_numOfVolt; ++k) {
			int taskID = thisRealRow[0].taskID;
			int taskDur = m_taskDuration[taskID][k];
			double inputPower = getExtraChargePower(taskID, k);
			size_t taskiFinishTime = (iter-lastIdleRow.begin()) + taskDur;
			tableEntryIter realTaskIter = thisRealRow.begin() + taskiFinishTime;
			// Must guarantee there is enough power for charging
			if ((inputPower > 0.0) && (realTaskIter < thisRealRow.end())) {
#ifdef DEBUG_VERBOSE
				cout << "I am predicting real task " << taskID << " from time " << (iter-lastIdleRow.begin()) << " to " << taskiFinishTime << "." << endl;
#endif
				double energy = energyCalculatorWrapper(inputPower, iter->totalEnergy, taskDur);

				// Must guarantee that the schedule is feasible
				if (realTaskIter->totalEnergy < energy) {
#ifdef DEBUG_VERBOSE
					cout << "Taskid: " << taskID << " entry " << taskiFinishTime << " has been updated from "
						<< realTaskIter->totalEnergy << " to " << energy << endl;;
#endif
					// Update the table entry
					realTaskIter->setAllFields(energy, volSel[k], m_taskCurrent[taskID][k], k, taskID, taskDur, taskiFinishTime-taskDur);
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

void dynProg::taskTimeline() {
	for (int i = 0; i < m_numOfTask; i++) {
		for (size_t j = 0; j < m_scheduleVolt[0].size(); ++j) {
			m_scheduleVolt[i][j] = -1;
			m_scheduleEnergy[i][j] = CRAZY_ENERGY;
			m_lastStepDuration[i][j] = -1;
		}
	}
	for (int k = 0; k < m_numOfVolt; ++k) {
		size_t taskDur = m_taskDuration[0][k];
		double inputPower = getExtraChargePower(0, k);
		if ((inputPower > 0) && (taskDur < m_scheduleVolt[0].size())) {
			double energy = energyCalculator(inputPower, m_initialEnergy, taskDur);
			m_scheduleVolt[0][taskDur] = volSel[k];
			m_scheduleEnergy[0][taskDur] = energy;
		}
	}

	for (int i = 1; i < m_numOfTask; i++) {
		for (size_t j = 0; j < m_scheduleVolt[0].size(); ++j) {
			if (m_scheduleVolt[i-1][j] > 0) {
				for (int k = 0; k < m_numOfVolt; ++k) {

					int taskDur = m_taskDuration[i][k];

					double inputPower = getExtraChargePower(i, k);
					// Must guarantee there is enough power for charging
					if (inputPower > 0) {
						double energy = energyCalculator(inputPower, m_scheduleEnergy[i-1][j], taskDur);

						size_t taskiFinishTime = j + taskDur;

						// Must guarantee that the schedule is feasible
						if ((m_scheduleEnergy[i][taskiFinishTime] < energy) && (taskiFinishTime < m_scheduleVolt[i].size())) {
							// Reset the energy
							m_scheduleVolt[i][taskiFinishTime] = volSel[k];
							m_scheduleEnergy[i][taskiFinishTime] = energy;
							m_lastStepDuration[i][taskiFinishTime] = j;
						}
					}
				} // for k
			} // if last task schedule
		} // for j
	} // for i

	#ifdef DEBUG_VERBOSE
	for (int i = 0; i < m_numOfTask; i++) {
			cout<<"Task "<<i<<":";
		for (int j = 0; j <= m_deadline; j++) {
			cout<<"Time step "<< j <<": ("<< m_scheduleEnergy[i][j]<<","<<m_scheduleVolt[i][j]<<","<<m_lastStepDuration[i][j]<<")-->"<<endl;
		}
		cout<<endl;
	}
	#endif
}

double dynProg::getExtraChargePower(int taskIdx, int volLevel) {

	double dc_load_vin(1.0), dc_load_vout(volSel[volLevel]), dc_load_iout(m_taskCurrent[taskIdx][volLevel]);
	double dc_load_iin(0.0), dc_load_power(0.0);

	m_dcLoad.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
	double chargingPower =  m_solarPower - dc_load_vin*dc_load_iin;
	return chargingPower;
}

double dynProg::getExtraChargePower(const TaskVoltageTable &tvt, size_t volLevel) {

	double dc_load_vin(1.0), dc_load_vout(tvt.getVoltage(volLevel)), dc_load_iout(tvt.getCurrent(volLevel));
	double dc_load_iin(0.0), dc_load_power(0.0);

	m_dcLoad.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
	double chargingPower =  m_solarPower - dc_load_vin*dc_load_iin;
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

void dynProg::backTracing() {
	size_t optIdx = max_element(m_scheduleEnergy.back().begin(), m_scheduleEnergy.back().end()) - m_scheduleEnergy.back().begin();
	for (int i = m_numOfTask-1; i >= 0; --i) {
		dpTableEntry entry;
		entry.totalEnergy = m_scheduleEnergy[i][optIdx];
		entry.voltage = m_scheduleVolt[i][optIdx];
		entry.volLevel = find(volSel.begin(), volSel.end(), entry.voltage) - volSel.begin();
		entry.taskID = i;
		entry.len = (i > 0) ? optIdx - (int)m_lastStepDuration[i][optIdx] : optIdx;
		entry.current = m_taskCurrent[i][entry.volLevel];
		optimalSchedule.push(entry);

		optIdx = (i > 0) ? (int)m_lastStepDuration[i][optIdx] : 0;
	}
	return;
}

int dynProg::genScheduleForEES() {
	ofstream outfile;
	int totalLength = 0;
	outfile.open("TasksDP.txt");
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

void dynProg::taskScheduling(){
	taskTimeline();
	int i = m_numOfTask - 1;
	int j = (int)(m_deadline * 10.0);

	//tranverse from the last task at the deadline
	m_durationSet.push_back(m_deadline);
	m_voltSet.push_back(m_scheduleVolt[i][j]);
	j = (int)(m_lastStepDuration[i][j] * 10.0);
	i --;

	while(i >= 0){
		m_durationSet.push_back((double)j / 10.0);
		m_voltSet.push_back(m_scheduleVolt[i][j]);
		j = (int)(m_lastStepDuration[i][j] * 10.0);
		i --;
	}

}
vector<double> dynProg::getDurationSet(){
	taskScheduling();
	return m_durationSet;
}
vector<double> dynProg::getVoltSet(){
	taskScheduling();
    return m_voltSet;
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
	lastTaskFinishTime(0) {}

ostream& operator<<(ostream& os, const dpTableEntry &e) {
	os << "In this entry, ";
	os.precision(4);
	os << "totalEnergy: " << std::fixed << std::setw(12) << e.totalEnergy;
	os << ", voltage: " << std::setprecision(2) << std::fixed << std::setw(8) << e.voltage;
	os << ", current: " << std::setw(8) << e.current;
	os << ", volLevel: " << std::setw(4) << e.volLevel;
	os << ", taskID: " << std::setw(4) << e.taskID;
	os << ", len: " << std::setw(4) << e.len;
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

	if (!above_min_valid_input_power(inputPower) || !below_max_valid_input_power(inputPower)) {
		energyCalculator = bind(energyCalculatorApproximation, placeholders::_1, placeholders::_2, placeholders::_3);
	} else {
		energyCalculator = bind(&nnetmultitask::predictWithEnergyLength, nnetPredictor, placeholders::_1, placeholders::_2, placeholders::_3);
	}

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
	double deadline = 25.0;
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
	dynProg taskSet1 (InDuration.size(), deadline, vec_tvt);
	taskSet1.dynamicProgrammingWithIdleTasks();
	ScheduleBuilder sb;
	sb.BuildScheduleFromFile("TasksDP.txt");
	sb.PredictEnergyForSchedule(0.5 * (supcap_init_charge*supcap_init_charge) / 40.0);
	sb.DumpSchedule();
	// taskSet1.taskTimeline();
	// taskSet1.backTracing();
	// outDuration = taskSet1.getDurationSet();
	// outVolt = taskSet1.getVoltSet();
	// for(int i = 0; i < numOfTask; i ++){
	// 	cout<<outDuration[i]<<", ";
	// 	cout<<outVolt[i]<<endl;
	// }
}

#endif
