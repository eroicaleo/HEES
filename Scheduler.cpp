#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <tr1/functional>

#include "Scheduler.hpp"
#include "DP/taskScheduling.hpp"
#include "compSet/ca_ts.hpp"
#include "nnet/nnetmultitask.hpp"

using namespace std;
using namespace std::tr1;

static double randomRatio;

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

	minEnergySchedule(m_numOfTask, m_numOfVolt, m_deadline, m_inputDuration, m_inputEnergy);
	minEnergyScheduleFixed(m_numOfTask, m_numOfVolt, m_deadline, m_inputDuration, m_inputEnergy);

	// Then use 
    dynProg taskSet1(m_numOfTask, m_numOfVolt, m_deadline, m_inputDuration, m_inputEnergy);
	nnetmultitask nnetPredictor;
	taskSet1.energyCalculator = bind(&nnetmultitask::predictWithEnergyLength, nnetPredictor, placeholders::_1, placeholders::_2, placeholders::_3);
	taskSet1.taskTimeline();
	taskSet1.backTracing();
	taskSet1.genScheduleForEES();

	return 0;
}

void randomTaskSetGenerator(int &m_numOfTask, int &m_numOfVolt, double &m_deadline, vector<double> &m_inputDuration, vector<double> &m_inputEnergy) {
	// Generate 10 tasks
	m_deadline = 0;
    m_numOfTask = 5;
	m_numOfVolt = 5;

    vector<double> InDuration;
    vector<double> InEnergy;
	ofstream outfile;
	outfile.open("TasksOrig.txt");
	if (!outfile) {
		cerr << "Can not open TasksOrig.txt for write!" << endl;
	}
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

		m_deadline += tasklen;

		outfile << tasklen << " "
			<< taskpower << " "
			<< taskEnergy << endl;
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
