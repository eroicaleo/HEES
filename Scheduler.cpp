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
#include "ParseCommandLine.hpp"
#include "VoltageTable.hpp"

using namespace std;
using namespace std::tr1;

int hees_parse_command_line(int argc, char *argv[]);

static double randomRatio;
extern double ratio_runtime_and_deadline;

static const size_t pxaVoltageLevel = 4;
static const double pxaVoltageTable[pxaVoltageLevel] = {0.75, 1.0, 1.3, 1.6};

int main(int argc, char *argv[]) {

	hees_parse_command_line(argc, argv);
	randomRatio = ratio_runtime_and_deadline;
	cout << randomRatio << endl;
	generateSchedule();
	return 0;
}

vector<TaskVoltageTable> vec_tvt;

int generateSchedule() {

	int m_numOfVolt;
	double m_deadline;
	vector<double> m_inputDuration, m_inputEnergy;

	randomTaskSetGenerator(number_of_tasks, m_numOfVolt, m_deadline, m_inputDuration, m_inputEnergy);

	// Baseline policy
	minEnergyScheduleFixed(number_of_tasks, m_numOfVolt, m_deadline, m_inputDuration, m_inputEnergy, vec_tvt);

	// Then use dynamic programming to generate optimal schedule
	dynProg taskSet1(vec_tvt.size(), m_deadline, vec_tvt);
	taskSet1.dynamicProgrammingWithIdleTasks();
	m_deadline = taskSet1.getDeadline();
	m_deadline /= 10.0;

	return 0;
}

void randomTaskSetGenerator(int &m_numOfTask, int &m_numOfVolt, double &m_deadline, vector<double> &m_inputDuration, vector<double> &m_inputEnergy) {
	// Generate 10 tasks
	m_deadline = 0;
	m_numOfVolt = 5;

    vector<double> InDuration;
    vector<double> InEnergy;
    vector<double> InPower;
	ofstream outfile;
	// Random the seed
	srand(time(NULL));
	for (int i = 0; i < m_numOfTask; ++i) {
		// Each task is between [10 100] with a timestamp 10
		int tasklen = (10 + rand() % 200);
		// Each task power is between 0.6 ~ 2.0
		double taskpower = min_task_power + (rand() % 100) * (max_task_power - min_task_power) / 100.0;
		double taskEnergy = tasklen * taskpower;
		InDuration.push_back(tasklen);
		InEnergy.push_back(taskEnergy);
		InPower.push_back(taskpower);

		m_deadline += tasklen;

		vec_tvt.push_back(TaskVoltageTable(syntheticCPUVoltageTable, taskpower, tasklen));

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
