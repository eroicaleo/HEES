#include <fstream>
#include <iomanip>
#include <iterator>
#include <typeinfo>

#include <tr1/functional>

#include "TaskHandoff.hpp"

using namespace std;
using namespace std::tr1;
using namespace boost;

void TaskHandoff::print(ostream &os) const {
	os << std::fixed << std::setw(8)  << length;
	os << std::fixed << std::setw(8)  << std::setprecision(2) << power;
	os << std::fixed << std::setw(12) << std::setprecision(2) << energy;

	return;
}

ostream &operator<<(ostream &os, const TaskHandoff &t) {
	t.print(os);

	return os;
}

void TaskHandoffHEES::print(ostream &os) const {
	os << std::fixed << std::setw(8) << std::setprecision(2) << voltage;
	os << std::fixed << std::setw(8) << std::setprecision(2) << current;
	os << std::fixed << std::setw(8) << getLength();

	return;
}

ostream &operator<<(ostream &os, const TaskHandoffHEES &t) {
	t.print(os);

	return os;
}

void genScheduleTaskHandoffSet(const vector<TaskHandoffHEES> &taskHandoffSet, string fileName) {

	ofstream outfile(fileName.c_str());
	if (!outfile) {
		cerr << "genScheduleTaskHandoffSet can not open " << fileName
			<< " for write! Exit!" << endl;
		exit(66);
	}

	copy(taskHandoffSet.begin(), taskHandoffSet.end(), ostream_iterator<TaskHandoffHEES>(outfile, "\n"));
	outfile.close();

	return;
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
