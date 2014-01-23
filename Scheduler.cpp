#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <vector>
#include <tr1/functional>

#include "Scheduler.hpp"
#include "DP/taskScheduling.hpp"
#include "compSet/ca_ts.hpp"
#include "nnet/nnetmultitask.hpp"

using namespace std;
using namespace std::tr1;

static double randomRatio;

static const size_t syntheticVoltageLevel = 5;
static const double syntheticVoltageTable[syntheticVoltageLevel] = {0.8, 0.9, 1.0, 1.1, 1.2};
static const size_t pxaVoltageLevel = 4;
static const double pxaVoltageTable[pxaVoltageLevel] = {0.75, 1.0, 1.3, 1.6};

int main(int argc, char *argv[]) {
	if (argc != 2) {
		cerr << "Must provide a float number between between 0.8 and 1.1" << endl;
		exit(88);
	}
	randomRatio = atof(argv[1]);
	cout << randomRatio << endl;
	generateSchedule();
	return 0;
}

int generateSchedule() {

	int m_numOfTask, m_numOfVolt; 
	double m_deadline;
	vector<double> m_inputDuration, m_inputEnergy;

	// dummyTaskSetGenerator(int &m_numOfTask, int &m_numOfVolt, double &m_deadline, vector<double> &m_inputDuration, vector<double> &m_inputEnergy);
	randomTaskSetGenerator(m_numOfTask, m_numOfVolt, m_deadline, m_inputDuration, m_inputEnergy);

	// Then use 
    dynProg taskSet1(m_numOfTask, vector<double>(syntheticVoltageTable, syntheticVoltageTable+syntheticVoltageLevel), m_deadline, m_inputDuration, m_inputEnergy);
	nnetmultitask nnetPredictor;
	taskSet1.energyCalculator = bind(&nnetmultitask::predictWithEnergyLength, nnetPredictor, placeholders::_1, placeholders::_2, placeholders::_3);
	taskSet1.taskTimeline();
	taskSet1.backTracing();
	m_deadline = taskSet1.genScheduleForEES();
	m_deadline /= 10.0;

	// Baseline policy
	minEnergySchedule(m_numOfTask, m_numOfVolt, m_deadline, m_inputDuration, m_inputEnergy);
	minEnergyScheduleFixed(m_numOfTask, m_numOfVolt, m_deadline, m_inputDuration, m_inputEnergy);

	return 0;
}

void randomTaskSetGenerator(int &m_numOfTask, int &m_numOfVolt, double &m_deadline, vector<double> &m_inputDuration, vector<double> &m_inputEnergy) {
	// Generate 10 tasks
	m_deadline = 0;
    m_numOfTask = 3;
	m_numOfVolt = 5;

    vector<double> InDuration;
    vector<double> InEnergy;
    vector<double> InPower;
	ofstream outfile;
	// Random the seed
	srand(time(NULL));
	for (int i = 0; i < m_numOfTask; ++i) {
		// Each task is between [10 100] with a timestamp 10
		int tasklen = (1 + rand() % 9);
		// Each task power is between 0.6 ~ 2.0
		double taskpower = 0.8 + (rand() % 100) * (1.5 - 0.8) / 100.0;
		double taskEnergy = tasklen * taskpower;
		InDuration.push_back(tasklen);
		InEnergy.push_back(taskEnergy);
		InPower.push_back(taskpower);

		m_deadline += tasklen;

	}
	outfile.open("TasksOrigNoSorting.txt");
	if (!outfile) {
		cerr << "Can not open TasksOrigNoSorting.txt for write!" << endl;
	}
	for (size_t i = 0; i < InEnergy.size(); ++i) {
		outfile << InDuration[i] << " "
		<< InPower[i] << " "
		<< InEnergy[i] << endl;
	}
	outfile.close();

	// highPowerLongTask(InDuration, InEnergy, InPower);
	outfile.open("TasksOrig.txt");
	if (!outfile) {
		cerr << "Can not open TasksOrig.txt for write!" << endl;
	}
	for (size_t i = 0; i < InEnergy.size(); ++i) {
		outfile << InDuration[i] << " "
		<< InPower[i] << " "
		<< InEnergy[i] << endl;
	}
	outfile.close();

	m_deadline /= randomRatio;
	m_inputDuration = InDuration;
	m_inputEnergy = InEnergy;

	return;
}

void dummyTaskSetGenerator(int &m_numOfTask, int &m_numOfVolt, double &m_deadline, vector<double> &m_inputDuration, vector<double> &m_inputEnergy) {

    m_deadline = 25;
    m_numOfTask = 3;
	m_numOfVolt = 5;

    vector<double>InDuration;
    InDuration.push_back(6.0);
    InDuration.push_back(7.0);
    InDuration.push_back(8.0);

	m_inputDuration = InDuration;

    vector<double>InEnergy;
    InEnergy.push_back(6.0);
    InEnergy.push_back(7.0);
    InEnergy.push_back(8.0);

	m_inputEnergy = InEnergy;

	return;
}

void highPowerLongTask(vector<double> &m_taskDuration, vector<double> &m_taskEnergy, vector<double> &m_taskPower) {
	vector<double> taskDuration(m_taskDuration);
	vector<double> taskPower(m_taskPower);
	map<double, double> taskMap;

	// Sort the task len and power vector in descending order
	sort(taskDuration.begin(), taskDuration.end(), greater<double>());
	sort(taskPower.begin(), taskPower.end(), greater<double>());

	for (size_t i = 0; i < taskDuration.size(); ++i) {
		taskMap[taskDuration[i]] = taskPower[i];
	}

	for (size_t i = 0; i < taskDuration.size(); ++i) {
		m_taskEnergy[i] = taskMap[m_taskDuration[i]] * m_taskDuration[i];
		m_taskPower[i] = taskMap[m_taskDuration[i]];
	}

	return;
}