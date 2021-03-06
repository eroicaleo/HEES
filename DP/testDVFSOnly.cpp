#include <iostream>

#include "SwapScheduling.hpp"
#include "taskScheduling.hpp"
#include "../ParseCommandLine.hpp"

using namespace std;

int main(int argc, char *argv[]) {

	cout << "Hello swap!" << endl;

	hees_parse_command_line(argc, argv);

	// DVFS phase
	// Reset the solar power source
	vps.resetVariablePowerSource();

	vector<TaskVoltageTable> vecTaskVoltageTable;
	BuildTaskVoltageTableVectorFromFile("TasksSCHEDForDP.txt", vecTaskVoltageTable, *cpu_voltage_table_ptr);

	// deadline is from the solar power length
	dynProg taskSet(vecTaskVoltageTable.size(), (int)vps.getSolarPowerLength(), vecTaskVoltageTable);
	taskSet.dynamicProgrammingWithIdleTasks();

	return 0;
}
