#include <string>
#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stack>
#include <vector>

#include "taskScheduling.hpp"
#include "../nnet/nnetmultitask.hpp"

using namespace std;
using namespace std::tr1;

static const double CRAZY_ENERGY(-1000.0);

dynProg::dynProg(int numOfTask, vector<double> voltageTable, double deadline, vector<double> taskDuration, vector<double> taskEnergy) {

	m_numOfTask = numOfTask;
	m_numOfVolt = voltageTable.size();
	m_deadline = (int)(deadline*10);
	m_inputDuration = taskDuration;
	m_inputEnergy = taskEnergy;

	volSel = voltageTable;

	m_taskDuration = vector<vector<int> >(numOfTask, vector<int>(m_numOfVolt, 0));
	m_taskEnergy = vector<vector<double> >(numOfTask, vector<double>(m_numOfVolt, 0.0));
	m_taskCurrent = vector<vector<double> >(numOfTask, vector<double>(m_numOfVolt, 0.0));

	m_scheduleEnergy = vector<vector<double> >(numOfTask, vector<double>(m_deadline + 1 , 0.0));
	m_scheduleVolt =  vector<vector<double> >(numOfTask, vector<double>(m_deadline + 1, 0.0));
	m_lastStepDuration = vector<vector<double> >(numOfTask, vector<double>(m_deadline + 1, 0.0));

	// for(int i = 0; i < numOfTask; i ++){
	// 	cout<<"Task "<<i<<" :"<<m_inputDuration[i]<<", "<<m_inputEnergy[i]<<endl;
	// }
	// memory the task energy and duration at each voltage;
	for (int i = 0; i < m_numOfTask; i++) {
		// cout<<"Task "<<i<<":";
		for (int k = 0; k < m_numOfVolt; k++) {
			m_taskDuration[i][k] = (int)ceil(m_inputDuration[i] * 10.0 / volSel[k]);
			m_taskEnergy[i][k] = m_inputEnergy[i] * volSel[k] * volSel[k];
			m_taskCurrent[i][k] = m_inputEnergy[i] / m_inputDuration[i] * volSel[k] * volSel[k] ;
			// cout<<m_taskDuration[i][k]<<","<<m_taskEnergy[i][k]<<endl;
		}
	}

	// System model related parameters
	m_initialEnergy = 20;
	m_solarPower = 3.0;
}

void dynProg::taskTimeline() {
	// memory the voltage and the energy of each task at every time step:
	// The first task
	// for (int j = 0; j <= (int)(m_deadline * 10.0); j ++) { //Timeline
	// 	// The time is less than the duration at the max voltage
	// 	if (j - (int)(m_taskDuration[0][0] * 10.0) < 0) {
	// 		// cout<<j<<",";
	// 		m_scheduleEnergy[0][j] = -1;
	// 		m_scheduleVolt[0][j] = -1;
	// 		m_lastStepDuration[0][j] = -1;
	// 	} else if (j - (int)(m_taskDuration[0][0] * 10.0) >= 0) {
	// 	//	double tmpEnergyAtVol[5];
	// 		vector<double>tmpEnergyAtVol;
	// 		tmpEnergyAtVol = vector<double>( m_numOfVolt, -1);
	// 		// The time is larger than the duration at the max voltage;
	// 		tmpEnergyAtVol[0] = m_taskEnergy[0][0];
	// 		for(int k = 1; k < m_numOfVolt; k++){//Voltage
	// 			if(j - (int)(m_taskDuration[0][k] * 10.0) < 0)
	// 				tmpEnergyAtVol[k] = -1;
	// 			else
	// 				tmpEnergyAtVol[k] = m_taskEnergy[0][k];
	// 		 }
	// 		int minTemp = 0;
	// 		//choose voltage at which energy is min
	// 		for(int k = 1; k < m_numOfVolt; k++){
	// 			if((tmpEnergyAtVol[k] < tmpEnergyAtVol[minTemp]) && (tmpEnergyAtVol[k] > 0))
	// 				minTemp = k;
	// 		}
	// 		m_scheduleEnergy[0][j] = tmpEnergyAtVol[minTemp];
	// 		m_scheduleVolt[0][j] = volSel[minTemp];
	// 		m_lastStepDuration[0][j] = -1;
	// 	}
	// }
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

	//	for(int j = 0; j <= (int)(m_deadline * 10.0); j ++){
	//		cout<<"time step "<<j<<":"<<m_scheduleEnergy[0][j]<<endl;
	//	}

	//The left task set
	//	for (int i = 1; i < m_numOfTask; i++) { // Task
	//		for (int j = 0; j <= (int)(m_deadline * 10.0); j ++) { // Timeline
	//			// The predecessors have not been processed
	//			if (j <= (int)(m_taskDuration[i][0] * 10.0)) {
	//				m_scheduleEnergy[i][j] = -1;
	//				m_scheduleVolt[i][j] = -1;
	//				m_lastStepDuration[i][j] = -1;
	//			}else if(m_scheduleEnergy[i-1][j - (int)(m_taskDuration[i][0] * 10.0)] == -1){

	//					//The left time is less than current task duration at max voltage
	//					m_scheduleEnergy[i][j] = -1;
	//					m_scheduleVolt[i][j] = -1;
	//					m_lastStepDuration[i][j] = -1;
	//			}else{
	//				//double tmpEnergyAtVol[5];
	//				vector<double>tmpEnergyAtVol;
	//				tmpEnergyAtVol = vector<double>( m_numOfVolt, -1);
	//				for(int k = 0; k < m_numOfVolt; k++){ // Voltage
	//					if((j - m_taskDuration[i][k] <= 0) || (m_scheduleEnergy[i-1][j - (int)(m_taskDuration[i][k] * 10.0)] == -1))
	//						tmpEnergyAtVol[k] = -1;
	//					else
	//						tmpEnergyAtVol[k] = m_taskEnergy[i][k] + m_scheduleEnergy[i-1][j - (int)(m_taskDuration[i][k] *10.0)];
	//				}
	//				int minTemp = 0;
	//				//choose voltage at which energy is min
	//				for(int k = 1; k < m_numOfVolt; k++){
	//					if((tmpEnergyAtVol[k] < tmpEnergyAtVol[minTemp]) && (tmpEnergyAtVol[k] > 0))
	//						minTemp = k;
	//				}
	//						cout<<"Task "<<i<<" 's minTemp:"<<minTemp<<" @"<<j<<endl;

	//				m_scheduleEnergy[i][j] = tmpEnergyAtVol[minTemp];
	//				m_scheduleVolt[i][j] = volSel[minTemp];
	//				m_lastStepDuration[i][j] = (double)(j - (int)(m_taskDuration[i][minTemp] * 10.0)) / 10.0;
	//			}
	//		}
	//	}

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
	return (chargingPower < 0) ? 0.001 : chargingPower;
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
		outfile << tmpTaskEntry.voltage << " ";
		outfile << tmpTaskEntry.current << " ";
		outfile << (tmpTaskEntry.len) << endl;
		#ifdef DEBUG_VERBOSE
			cout << "Task id: " << tmpTaskEntry.taskID
				<< " run for " << tmpTaskEntry.len << " timestamp (0.1s) "
				<< " voltage: " << tmpTaskEntry.voltage
				<< " current: " << tmpTaskEntry.current
				<< " final energy: " << tmpTaskEntry.totalEnergy << endl;
		#endif
		tmpSchedule.pop();
		totalLength += tmpTaskEntry.len;
	}
	outfile.close();

	return totalLength;
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

const size_t syntheticVoltageLevel = 5;
const double syntheticVoltageTable[syntheticVoltageLevel] = {0.8, 0.9, 1.0, 1.1, 1.2};
const size_t pxaVoltageLevel = 4;
const double pxaVoltageTable[pxaVoltageLevel] = {0.75, 1.0, 1.3, 1.6};

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

	deadline /= 1.05;

	infile.close();
	return;
}

/* Testbench */
int main(){
	double deadline = 25.0;
	vector<double>InDuration;
	// InDuration.push_back(6.0);
	// InDuration.push_back(7.0);
	// InDuration.push_back(8.0);

	vector<double>InEnergy;
	// InEnergy.push_back(6.0);
	// InEnergy.push_back(7.0);
	// InEnergy.push_back(8.0);
	
	readInput(InDuration, InEnergy, deadline);
	vector<double>outDuration;
	vector<double>outVolt;
    dynProg taskSet1 (InDuration.size(), vector<double>(syntheticVoltageTable, syntheticVoltageTable+syntheticVoltageLevel), deadline, InDuration, InEnergy);
	nnetmultitask nnetPredictor;
	taskSet1.energyCalculator = bind(&nnetmultitask::predictWithEnergyLength, nnetPredictor, placeholders::_1, placeholders::_2, placeholders::_3);
	taskSet1.taskTimeline();
	taskSet1.backTracing();
	taskSet1.genScheduleForEES();
	// outDuration = taskSet1.getDurationSet();
	// outVolt = taskSet1.getVoltSet();
	// for(int i = 0; i < numOfTask; i ++){
	// 	cout<<outDuration[i]<<", ";
	// 	cout<<outVolt[i]<<endl;
	// }
}

#endif
