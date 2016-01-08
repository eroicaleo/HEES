#include <iostream>

#include "SwapScheduling.hpp"
#include "../ParseCommandLine.hpp"

using namespace std;

void SwapScheduling::buildTaskTable(char *filename) {

	VoltageTableDFS vt(vector<double>(1, 1.0), 1.0);
	BuildTaskVoltageTableVectorFromFile(filename, this->realTaskVoltageTable, vt);

	return;
}

vector<double> SwapScheduling::taskToPowerTrace(const TaskVoltageTable &tvt) const {
	size_t level = tvt.getNominalVoltageIndex();
	double power = tvt.getVoltage(level) * tvt.getCurrent(level);
	return vector<double>(tvt.getScaledCeilLength(level, 1), power);
}

void SwapScheduling::buildSolarPowerTrace() {

	vector<double> taskPowerTrace;
	for (vector<TaskVoltageTable>::const_iterator it = realTaskVoltageTable.begin(); it != realTaskVoltageTable.end(); ++it) {
		vector<double> trace = taskToPowerTrace(*it);
		taskPowerTrace.insert(taskPowerTrace.end(), trace.begin(), trace.end());
	}

	solarPowerTrace.resize(taskPowerTrace.size());
	solarPowerTrace[0] = power_source_func(0);
	for (size_t i = 0; i < taskPowerTrace.size(); ++i) {
		solarPowerTrace[i] = power_source_func(i+1);
	}

}

#ifdef SWAP_MAIN
int main(int argc, char *argv[]) {

	cout << "Hello swap!" << endl;

	hees_parse_command_line(argc, argv);
	SwapScheduling ss;
	ss.buildTaskTable("TasksSolar.txt.example");
	ss.buildSolarPowerTrace();
	return 0;
}
#endif
