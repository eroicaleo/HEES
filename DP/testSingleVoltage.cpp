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

using namespace std;
using namespace std::tr1;

int main(int argc, char *argv[]) {

	double deadline(0.0);

	vector<TaskVoltageTable> vec_tvt;
	hees_parse_command_line(argc, argv);
	VoltageTable tmp = GenerateSingleVoltageTable(1.0);
	BuildTaskVoltageTableVectorFromFile("TasksOrig.txt", vec_tvt, tmp);

	if (scheduling_deadline > 0) {
		deadline = scheduling_deadline;
	}

	dynProg taskSet1(vec_tvt.size(), deadline, vec_tvt);
	taskSet1.dynamicProgrammingWithIdleTasks();

	return 0;
}
