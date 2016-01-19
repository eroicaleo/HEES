#include <fstream>
#include <iomanip>
#include <iterator>

#include "TaskHandoff.hpp"

using namespace std;

ostream& operator<<(ostream &os, const TaskHandoff &t) {
	os << std::fixed << std::setw(8)  << t.length;
	os << std::fixed << std::setw(8)  << std::setprecision(2) << t.power;
	os << std::fixed << std::setw(12) << std::setprecision(2) << t.energy;

	return os;
}

void genScheduleTaskHandoffSet(const vector<TaskHandoff> &taskHandoffSet, string fileName) {

	ofstream outfile(fileName.c_str());
	if (!outfile) {
		cerr << "genScheduleTaskHandoffSet can not open " << fileName
			<< " for write! Exit!" << endl;
		exit(66);
	}

	copy(taskHandoffSet.begin(), taskHandoffSet.end(), ostream_iterator<TaskHandoff>(outfile, "\n"));
	outfile.close();

	return;
}

