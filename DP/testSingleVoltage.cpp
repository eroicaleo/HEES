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
	deadline = vec_tvt.begin()->getScaledCeilLength(0.8, 10);

	if (scheduling_deadline > 0) {
		deadline = scheduling_deadline;
	}

	for (double vol = 0.8; vol < 1.3; vol += 0.1) {
		VoltageTable tmp = GenerateSingleVoltageTable(vol);
		BuildTaskVoltageTableVectorFromFile("TasksOrig.txt", vec_tvt, tmp);

		dynProg taskSet0(vec_tvt.size(), deadline, vec_tvt);
		taskSet0.dynamicProgrammingWithIdleTasks();
	}

	return 0;
}
