#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ca_ts.hpp"
#include "../DP/taskScheduling.hpp"

using namespace std;
using std::vector;

int minEnergySchedule(int m_numOfTask, int m_numOfVolt, double m_deadline, vector<double>m_inputDuration, vector<double>m_inputEnergy) {
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
	cout << "allDuration: " << allDuration << endl;
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

	double changePoint = 0.0;
	if (bottom == up) {
		changePoint = 1;
	} else {
		changePoint = (volSel[bottom] - selVolt) / (volSel[bottom] - volSel[up]);
	}
	cout<< "the changePoint is:"<< changePoint << endl;

	vector<double>currentSel;
	
	for (int i = 0; i < m_numOfTask; i++) {
		cout<< volSel[up] << " ";
		cout<< m_inputEnergy[i] / m_inputDuration[i] * volSel[up] * volSel[up] << " ";
		cout<< m_inputDuration[i] / selVolt * changePoint <<endl;

		cout<< volSel[bottom] << " ";
		cout<< m_inputEnergy[i] / m_inputDuration[i] * volSel[bottom] * volSel[bottom] << " ";
		cout<< m_inputDuration[i] / selVolt * (1 - changePoint) <<endl;
	}

	ofstream outfile;
	outfile.open("TasksCATS.txt");
	if (!outfile) {
		cerr << "Can not open TaskCATS.txt for write!" << endl;
		exit(66);
	}
	for (int i = 0; i < m_numOfTask; i++) {
		double tasklen = ceil(m_inputDuration[i] / selVolt * changePoint * 100) / 10.0;
		if (tasklen > 0.01) {
			outfile << volSel[up] << " ";
			outfile << m_inputEnergy[i] / m_inputDuration[i] * volSel[up] * volSel[up] << " ";
			outfile << tasklen << endl;
		}

		tasklen = floor(m_inputDuration[i] / selVolt * (1 - changePoint) * 100) / 10.0;
		if (tasklen > 0.01) {
			outfile << volSel[bottom] << " ";
			outfile << m_inputEnergy[i] / m_inputDuration[i] * volSel[bottom] * volSel[bottom] << " ";
			outfile <<  tasklen << endl;
		}
	}
	outfile.close();

 	return 0;
}

#ifdef CATS_BINARY

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

	deadline /= 0.89;

	infile.close();
	return;
}

int main() {
	double deadline = 25;
	int numOfTask = 3;
	int numOfVoltage = 5;
	vector<double>InDuration;
	// InDuration.push_back(6.0);
	// InDuration.push_back(7.0);
	// InDuration.push_back(8.0);
	// InDuration.push_back();
	// InDuration.push_back();

	vector<double>InEnergy;
	// InEnergy.push_back(6.0);
	// InEnergy.push_back(7.0);
	// InEnergy.push_back(8.0);
//	InEnergy.push_back();
//	InEnergy.push_back();

	readInput(InDuration, InEnergy, deadline);
	deadline = 13.3;
	minEnergySchedule(numOfTask, numOfVoltage, deadline, InDuration, InEnergy);	

	return 0;
}

#endif
