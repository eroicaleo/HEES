#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ca_ts.hpp"

using namespace std;
using std::vector;

int minEnergyScheduleFixed(int m_numOfTask, int m_numOfVolt, double m_deadline, vector<double>m_inputDuration, vector<double>m_inputEnergy) {
	vector<double> volSel;
	volSel = vector<double> (5, 0.0);	
	volSel[0] = 1.2;
	volSel[1] = 1.1;
	volSel[2] = 1.0;
	volSel[3] = 0.9;
	volSel[4] = 0.8;

//	vector< vector<double> > m_taskDuration;
//	vector< vector<double> > m_taskEnergy;
//	m_taskDuration = vector<vector<double> >(m_numOfTask, vector<double>(m_numOfVolt, 0.0));
//	m_taskEnergy = vector<vector<double> >(m_numOfTask, vector<double>(m_numOfVolt, 0.0));
	
//	for (int i = 0; i < m_numOfTask; i++) {
//		cout<<"Task "<<i<<":";
//		for (int k = 0; k < m_numOfVolt; k++) {
//			m_taskDuration[i][k] = m_inputDuration[i] / volSel[k];
//			m_taskEnergy[i][k] = m_inputEnergy[i] * volSel[k] * volSel[k];
//			cout<<m_taskDuration[i][k]<<","<<m_taskEnergy[i][k]<<endl;
//		}
//	}

	double selVolt = 0.0;
	double allDuration = 0.0;
	for (int i = 0; i < m_numOfTask; i++) {
		allDuration += m_inputDuration[i];
	}
	selVolt = allDuration / m_deadline;
	cout<<"the continuous voltage: "<<selVolt<<endl;
	
	if (selVolt > volSel[0]) {
		cout<<"ERROR: Continuous Voltage is larger than the Max Voltage Level!"<<endl;
		return 1;
	} else if (selVolt < volSel[m_numOfVolt -1]){
		cout<<"WARNING: Continuous Voltage is less than the Min Voltage Level!"<<endl;
	}

	int up = 0, bottom = m_numOfVolt -1;
	for (int k = 0; k < m_numOfVolt; k++) {
		if (selVolt - volSel[k] == 0.0) up = bottom = k;
		else {
			if ((selVolt - volSel[k] > 0.0) && ((selVolt - volSel[k]) < (selVolt - volSel[bottom]))) 
				bottom = k;
			else if ((selVolt - volSel[k] < 0.0) && ((selVolt - volSel[k]) > (selVolt - volSel[up])))
				up = k;
		}
	}
	cout<< "the boundary voltage:"<< volSel[bottom]<<" ~ "<<volSel[up]<<endl;
	double totalEnergy = 0.0;
	double totalDuration = 0.0;
	for (int i = 0; i < m_numOfTask; i ++){
		totalEnergy += m_inputEnergy[i];
		totalDuration += (m_inputDuration[i] / selVolt);
		cout << "the total duration @ ideal voltage is:" << totalDuration <<endl;
	}

	double changePoint = 0.0;
	if (bottom == up) {
		changePoint = 1;
	} else {
		changePoint = (selVolt - volSel[up]) / (volSel[bottom] - volSel[up]);
	}
	cout<< "the changePoint is:"<< changePoint << endl;

	double firstHalfDuration = 0.0;
	int passFlag = 0;
	firstHalfDuration +=  m_inputDuration[0] *  volSel[up] * volSel[up];
	for (int i = 0; i < m_numOfTask ; i ++) {
		if( firstHalfDuration < totalDuration * changePoint){
			cout<< volSel[up] << " ";
			cout<< m_inputEnergy[i] / m_inputDuration[i] * volSel[up] * volSel[up] << " ";
			cout<< m_inputDuration[i] / volSel[up]<<endl ;//* changePoint <<endl;
			firstHalfDuration +=  m_inputDuration[i] *  volSel[up] * volSel[up];
		}else if(passFlag == 0){
			cout<< volSel[up] << " ";
			cout<< m_inputEnergy[i] / m_inputDuration[i] * volSel[up] * volSel[up] << " ";
			cout<< m_inputDuration[i] / volSel[up]<<endl;// * (1 - changePoint) <<endl;
			firstHalfDuration +=  m_inputDuration[i] *  volSel[up] * volSel[up];
			passFlag = 1;
		}else if(passFlag == 1){
			cout<< volSel[bottom] << " ";
			cout<< m_inputEnergy[i] / m_inputDuration[i] * volSel[bottom] * volSel[bottom] << " ";
			cout<< m_inputDuration[i] / volSel[bottom]<<endl;// * (1 - changePoint) <<endl;
		}
	}

	ofstream outfile;
	outfile.open("TasksCATSFixed.txt");
	if (!outfile) {
		cerr << "Can not open TaskCATSFixed.txt for write!" << endl;
		exit(66);
	}
	firstHalfDuration = 0.0;
	passFlag = 0;
	firstHalfDuration +=  m_inputDuration[0] *  volSel[up] * volSel[up];
	for (int i = 0; i < m_numOfTask ; i ++) {
		if (firstHalfDuration < totalDuration * changePoint) {
			outfile << volSel[up] << " ";
			outfile << m_inputEnergy[i] / m_inputDuration[i] * volSel[up] * volSel[up] << " ";
			outfile << m_inputDuration[i] / volSel[up] * 10 << endl;//* changePoint <<endl;
			firstHalfDuration +=  m_inputDuration[i] *  volSel[up] * volSel[up];
		} else if (passFlag == 0) {
			outfile << volSel[up] << " ";
			outfile << m_inputEnergy[i] / m_inputDuration[i] * volSel[up] * volSel[up] << " ";
			outfile << m_inputDuration[i] / volSel[up] * 10 << endl;// * (1 - changePoint) <<endl;
			firstHalfDuration +=  m_inputDuration[i] *  volSel[up] * volSel[up];
			passFlag = 1;
		} else if (passFlag == 1) {
			outfile << volSel[bottom] << " ";
			outfile << m_inputEnergy[i] / m_inputDuration[i] * volSel[bottom] * volSel[bottom] << " ";
			outfile << m_inputDuration[i] / volSel[bottom] * 10 <<endl;// * (1 - changePoint) <<endl;
		}
	}
	outfile.close();

 	return 0;
}

#ifdef CATS_BINARY

int main() {
	double deadline = 24;
	int numOfTask = 3;
	int numOfVoltage = 5;
	vector<double>InDuration;
	InDuration.push_back(6.0);
	InDuration.push_back(7.0);
	InDuration.push_back(8.0);

	vector<double>InEnergy;
	InEnergy.push_back(6.0);
	InEnergy.push_back(7.0);
	InEnergy.push_back(8.0);

	minEnergyScheduleFixed(numOfTask, numOfVoltage, deadline, InDuration, InEnergy);	

	return 0;
}

#endif
