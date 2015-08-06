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

	vector<TaskVoltageTable> vecTvt;
	hees_parse_command_line(argc, argv);
	/*
	 * In this testcase, we only care about scheduling, so there is no need to
	 * do DVFS, we just need one 1.0v level
	 */
	VoltageTableDFS vt(vector<double>(1, 1.0), 1.0);

	BuildTaskVoltageTableVectorFromFile("TasksSched.txt", vecTvt, vt);
	for (size_t i = 0; i < vecTvt.size(); ++i) {
		deadline += vecTvt[i].getScaledCeilLength(1.0, 1);
		cout << "len: " << vecTvt[i].getScaledCeilLength(1.0, 1) << endl;
	}

	dynProg taskSet0(vecTvt.size(), deadline, vecTvt);
	taskSet0.dynamicProgrammingWithIdleTasks();

	return 0;
}
