#include <iostream>

#include "SwapScheduling.hpp"

using namespace std;

void SwapScheduling::buildTaskTable(char *filename) {

	VoltageTableDFS vt(vector<double>(1, 1.0), 1.0);
	BuildTaskVoltageTableVectorFromFile(filename, this->realTaskVoltageTable, vt);

	return;
}

void SwapScheduling::buildSolarPowerTrace(char *filename) {
}

#ifdef SWAP_MAIN
int main(int argc, char *argv[]) {

	cout << "Hello swap!" << endl;

	SwapScheduling ss;
	ss.buildTaskTable("TasksSolar.txt.example");
	return 0;
}
#endif
