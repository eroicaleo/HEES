#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ca_ts.hpp"
#include "../ParseCommandLine.hpp"
#include "../ScheduleBuilder.hpp"

using namespace std;

int minEnergyScheduleFixed(int m_numOfTask, int m_numOfVolt, double m_deadline, vector<double>m_inputDuration, vector<double>m_inputEnergy, const vector<TaskVoltageTable> &vec_tvt) {
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
	cout << "deadline: " << m_deadline << endl;
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
		changePoint = (volSel[bottom] - selVolt) / (volSel[bottom] - volSel[up]);
	}
	cout<< "the changePoint is:"<< changePoint << endl;

	double firstHalfDuration = 0.0;
	for (int i = 0; i < m_numOfTask ; i ++) {
		if ( firstHalfDuration < totalDuration * changePoint){
			cout<< volSel[up] << " ";
			cout<< m_inputEnergy[i] / m_inputDuration[i] * volSel[up] * volSel[up] << " ";
			cout<< m_inputDuration[i] / volSel[up]<<endl ;//* changePoint <<endl;
			firstHalfDuration +=  m_inputDuration[i] /  volSel[up];
		} else {
			cout<< volSel[bottom] << " ";
			cout<< m_inputEnergy[i] / m_inputDuration[i] * volSel[bottom] * volSel[bottom] << " ";
			cout<< m_inputDuration[i] / volSel[bottom]<<endl;// * (1 - changePoint) <<endl;
		}
	}

	firstHalfDuration = 0.0;
	for (int i = 0; i < m_numOfTask ; i ++) {
		if ( firstHalfDuration < totalDuration * changePoint){
			cout<< volSel[up] << " ";
			cout<< vec_tvt[i].getCurrent(volSel[up]) << " ";
			cout<< vec_tvt[i].getScaledCeilLength(volSel[up], 10) <<endl ;//* changePoint <<endl;
			firstHalfDuration +=  m_inputDuration[i] /  volSel[up];
		} else {
			cout<< volSel[bottom] << " ";
			cout<< vec_tvt[i].getCurrent(volSel[bottom]) << " ";
			cout<< vec_tvt[i].getScaledCeilLength(volSel[bottom], 10) <<endl;// * (1 - changePoint) <<endl;
		}
	}

	ofstream outfile;
	outfile.open("TasksCATSFixed.txt");
	if (!outfile) {
		cerr << "Can not open TasksCATSFixed.txt for write!" << endl;
		exit(66);
	}
	firstHalfDuration = 0.0;
	int timeUsed = 0;
	for (int i = 0; i < m_numOfTask ; i ++) {
		if (firstHalfDuration < totalDuration * changePoint) {
			outfile << volSel[up] << " ";
			outfile << vec_tvt[i].getCurrent(volSel[up]) << " ";
			outfile << vec_tvt[i].getScaledCeilLength(volSel[up], 10) <<endl ;//* changePoint <<endl;
			firstHalfDuration +=  m_inputDuration[i] /  volSel[up];
			timeUsed += vec_tvt[i].getScaledCeilLength(volSel[up], 10);
		} else {
			outfile << volSel[bottom] << " ";
			outfile << vec_tvt[i].getCurrent(volSel[bottom]) << " ";
			outfile << vec_tvt[i].getScaledCeilLength(volSel[bottom], 10) <<endl;// * (1 - changePoint) <<endl;
			timeUsed += vec_tvt[i].getScaledCeilLength(volSel[bottom], 10);
		}
	}
	int timeRemained = round(m_deadline*10 - timeUsed);
	double doubletimeRemained = m_deadline*10 - timeUsed;
	cout << "doubletimeRemained " << doubletimeRemained << endl;
	cout << "10*m_deadline " << int(m_deadline*10) << endl;
	cout << "timeUsed: " << timeUsed << endl;
	cout << "timeRemained: " << timeRemained << endl;
	// If there is sometime remains, we charge it with idle task
	if (timeRemained > 0) {
		outfile << vec_tvt[0].getNominalVoltage() << " ";
		outfile << 0.0 << " ";
		outfile << timeRemained << endl;
	}
	outfile.close();

	ScheduleBuilder sb;
	sb.BuildScheduleFromFile("TasksCATSFixed.txt");
	sb.PredictEnergyForSchedule(20.0);
	sb.DumpSchedule();

	return max(int(round(m_deadline*10)), timeUsed);
}

#ifdef CATS_BINARY

VoltageTable vt(vector<double>(syntheticVoltageTable, syntheticVoltageTable+syntheticVoltageLevel), 1.0);
vector<TaskVoltageTable> vec_tvt;

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
		vec_tvt.push_back(TaskVoltageTable(vt, power, tasklen));
	}

	deadline /= 1.1;

	infile.close();
	return;
}

int main(int argc, char *argv[]) {
	double deadline = 24;
	int numOfTask = 3;
	int numOfVoltage = 5;
	vector<double>InDuration;
	vector<double>InEnergy;

	hees_parse_command_line(argc, argv);
	readInput(InDuration, InEnergy, deadline);
	minEnergyScheduleFixed(numOfTask, numOfVoltage, 14.2, InDuration, InEnergy, vec_tvt);

	return 0;
}

#endif
