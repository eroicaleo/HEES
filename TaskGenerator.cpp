#include <vector>

#include "ParseCommandLine.hpp"
#include "TaskGenerator.hpp"
#include "VoltageTable.hpp"

using namespace std;

double randomTaskSetGenerator(vector<double> &InDuration, vector<double> &InEnergy, vector<TaskVoltageTable> &vec_tvt) {

	InDuration.clear();
	InEnergy.clear();
	vec_tvt.clear();

    vector<double> InPower;
	VoltageTable vt(vector<double>(syntheticVoltageTable, syntheticVoltageTable+syntheticVoltageLevel), 1.0);

	double m_deadline(0.0);

	ofstream outfile;
	// Random the seed
	srand(time(NULL));
	for (int i = 0; i < number_of_tasks; ++i) {
		// Each task is between [10 100] with a timestamp 10
		int tasklen = (1 + rand() % 9);
		// Each task power is between 0.6 ~ 2.0
		double taskpower = min_task_power + (rand() % 100) * (max_task_power - min_task_power) / 100.0;
		double taskEnergy = tasklen * taskpower;
		InDuration.push_back(tasklen);
		InEnergy.push_back(taskEnergy);
		InPower.push_back(taskpower);
		vec_tvt.push_back(TaskVoltageTable(vt, taskpower / vt.GetNominalVoltage(), tasklen));
		m_deadline += tasklen;
	}

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

	m_deadline /= ratio_runtime_and_deadline;

	return m_deadline;
}
