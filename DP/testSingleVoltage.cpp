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
#include "../TaskGenerator.hpp"
#include "../compSet/ca_ts.hpp"

using namespace std;
using namespace std::tr1;

int main(int argc, char *argv[]) {

	double deadline(0.0);

	vector<double> InDuration;
	vector<double> InEnergy;

	vector<TaskVoltageTable> vec_tvt;
	hees_parse_command_line(argc, argv);

	deadline = randomTaskSetGenerator(InDuration, InEnergy, vec_tvt);
	deadline *= 10;

	if (scheduling_deadline > 0) {
		deadline = scheduling_deadline;
	}

	VoltageTable tmp = GenerateSingleVoltageTable(1.0);
	BuildTaskVoltageTableVectorFromFile("TasksOrig.txt", vec_tvt, tmp);

	dynProg taskSet0(vec_tvt.size(), deadline, vec_tvt);
	taskSet0.dynamicProgrammingWithIdleTasks();

	tmp = GenerateSingleVoltageTable(1.1);
	BuildTaskVoltageTableVectorFromFile("TasksOrig.txt", vec_tvt, tmp);

	dynProg taskSet1(vec_tvt.size(), deadline, vec_tvt);
	taskSet1.dynamicProgrammingWithIdleTasks();

	tmp = GenerateSingleVoltageTable(1.2);
	BuildTaskVoltageTableVectorFromFile("TasksOrig.txt", vec_tvt, tmp);

	dynProg taskSet2(vec_tvt.size(), deadline, vec_tvt);
	taskSet2.dynamicProgrammingWithIdleTasks();

	return 0;
}
