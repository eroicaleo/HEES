#include <string>
#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "taskScheduling.hpp"

using namespace std;

static const double CRAZY_ENERGY(1000.0);

dynProg::dynProg(int numOfTask, double deadline, vector<double> taskDuration, vector<double> taskEnergy) {

	m_numOfTask = numOfTask;
	m_numOfVolt = 5;
	m_deadline = (int)deadline;
	m_inputDuration = taskDuration;
	m_inputEnergy = taskEnergy;

	// volSel[] = {1.2, 1.1, 1.0, 0.9, 0.8};
	volSel[0] = 1.2;
	volSel[1] = 1.1;
	volSel[2] = 1.0;
	volSel[3] = 0.9;
	volSel[4] = 0.8;

	m_taskDuration = vector<vector<int> >(numOfTask, vector<int>(m_numOfVolt, 0));
	m_taskEnergy = vector<vector<double> >(numOfTask, vector<double>(m_numOfVolt, 0.0));
	m_scheduleEnergy = vector<vector<double> >(numOfTask, vector<double>(m_deadline * 10 + 1 , 0.0));
	m_scheduleVolt =  vector<vector<double> >(numOfTask, vector<double>(m_deadline * 10 + 1, 0.0));
	m_lastStepDuration = vector<vector<double> >(numOfTask, vector<double>(m_deadline * 10 + 1, 0.0));

	// for(int i = 0; i < numOfTask; i ++){
	// 	cout<<"Task "<<i<<" :"<<m_inputDuration[i]<<", "<<m_inputEnergy[i]<<endl;
	// }
	// memory the task energy and duration at each voltage;
	for (int i = 0; i < m_numOfTask; i++) {
		// cout<<"Task "<<i<<":";
		for (int k = 0; k < m_numOfVolt; k++) {
			m_taskDuration[i][k] = (int)ceil(m_inputDuration[i] * 10.0 / volSel[k]);
			m_taskEnergy[i][k] = m_inputEnergy[i] * volSel[k] * volSel[k];
			// cout<<m_taskDuration[i][k]<<","<<m_taskEnergy[i][k]<<endl;
		}
	}
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
	for (int j = 0; j < m_numOfVolt; ++j) {
		int taskDur = m_taskDuration[0][j];
		m_scheduleVolt[0][taskDur] = volSel[j];
		m_scheduleEnergy[0][taskDur] = m_taskEnergy[0][j];
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
					double energy = m_scheduleEnergy[i-1][j] + m_taskEnergy[i][k];
					int taskDur = m_taskDuration[i][k];
					int taskiFinishTime = j + taskDur;
					if (m_scheduleEnergy[i][taskiFinishTime] > energy) {
						// Reset the energy
						m_scheduleVolt[i][taskiFinishTime] = volSel[k];
						m_scheduleEnergy[i][taskiFinishTime] = energy;
						m_lastStepDuration[i][taskiFinishTime] = j;
					}
				} // for k
			} // if last task schedule
		} // for j
	} // for i

	for (int i = 0; i < m_numOfTask; i++) {
			cout<<"Task "<<i<<":";
		for (int j = 0; j <= m_deadline * 10; j++) {
			cout<<"Time step "<< j <<": ("<< m_scheduleEnergy[i][j]<<","<<m_scheduleVolt[i][j]<<","<<m_lastStepDuration[i][j]<<")-->"<<endl;
		}
		cout<<endl;
	}
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

#ifdef DP_BINARY

/* Testbench */
int main(){
	double deadline = 25.0;
	int numOfTask = 3;
	vector<double>InDuration;
	InDuration.push_back(6.0);
	InDuration.push_back(7.0);
	InDuration.push_back(8.0);

	vector<double>InEnergy;
	InEnergy.push_back(6.0);
	InEnergy.push_back(7.0);
	InEnergy.push_back(8.0);

	vector<double>outDuration;
	vector<double>outVolt;
    dynProg taskSet1 (numOfTask, deadline, InDuration, InEnergy);
	// taskSet1.taskTimeline();
	outDuration = taskSet1.getDurationSet();
	outVolt = taskSet1.getVoltSet();
	for(int i = 0; i < numOfTask; i ++){
		cout<<outDuration[i]<<", ";
		cout<<outVolt[i]<<endl;
	}
}

#endif
