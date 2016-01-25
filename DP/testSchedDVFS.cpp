#include <iostream>

#include "SwapScheduling.hpp"
#include "taskScheduling.hpp"
#include "../ParseCommandLine.hpp"

using namespace std;

int main(int argc, char *argv[]) {

	cout << "Hello swap!" << endl;

	hees_parse_command_line(argc, argv);

	// Scheduling phase
	double cap = supcap_cap * supcap_parallel_conf / supcap_serial_conf;
	double initEnergy = 0.5 * (supcap_init_charge * supcap_init_charge) / cap;
	SwapScheduling ss(initEnergy);

	ss.buildTaskTable("TasksSolar.txt");
	ss.genScheduleForEES("TasksSCHEDForEES.txt.init", "TasksSCHEDForDP.txt.init");
	ss.buildSolarPowerTrace();
	ss.exhaustiveSwapping();
	ss.genScheduleForEES("TasksSCHEDForEES.txt", "TasksSCHEDForDP.txt");

	// DVFS phase
	// Reset the solar power source
	vps.resetVariablePowerSource();

	vector<TaskVoltageTable> vecTaskVoltageTable;
	BuildTaskVoltageTableVectorFromFile("TasksSCHEDForDP.txt", vecTaskVoltageTable, *cpu_voltage_table_ptr);

	// deadline is from the solar power length
	dynProg taskSet(ss.getTaskNumber(), (int)vps.getSolarPowerLength(), vecTaskVoltageTable);
	taskSet.dynamicProgrammingWithIdleTasks();

	return 0;
}
