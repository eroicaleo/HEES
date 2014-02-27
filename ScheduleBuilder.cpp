#include <fstream>
#include <tr1/functional>

#include "nnet/nnetmultitask.hpp"
#include "powersource.hpp"
#include "ScheduleBuilder.hpp"

using namespace std;
using namespace std::tr1;

extern const double CRAZY_ENERGY;
extern function<double(double)> power_source_func;

void ScheduleBuilder::BuildScheduleFromFile(const char *filename) {
	double vol, cur;
	int lastFinish(0), len(0);
	int realTaskID(0), idleTaskID(0); 
	dpTableEntry entry;

	ifstream infile(filename);
	while ((infile >> vol >> cur >> len).good()) {
		if (cur > 0.0) {
			entry.setAllFields(CRAZY_ENERGY, vol, cur, -1, realTaskID, len, lastFinish);
			++realTaskID;
		} else {
			entry.setAllFields(CRAZY_ENERGY, vol, cur, -1, idleTaskID, len, lastFinish);
			++idleTaskID;
		}
		lastFinish += len;
		m_schedule.push_back(entry);
	}
	infile.close();
}

void ScheduleBuilder::PredictEnergyForSchedule(double initEnergy) {

	dcconvertIN dcload;
	double inputPower(0.0);
	double currentEnergy(initEnergy);
	nnetmultitask nnetPredictor;
	std::tr1::function<double(double, double, double)> energyCalculator;
	energyCalculator = bind(&nnetmultitask::predictWithEnergyLength, nnetPredictor, placeholders::_1, placeholders::_2, placeholders::_3);

	for (tableEntryIter iter = m_schedule.begin(); iter != m_schedule.end(); ++iter) {
		inputPower = power_source_func(0.0) - dcload.GetPowerConsumptionWithLoad(iter->voltage, iter->current);
		currentEnergy = energyCalculator(inputPower, currentEnergy, iter->len);
		iter->totalEnergy = currentEnergy;
	}

	return;
}

void ScheduleBuilder::DumpSchedule(ostream &os) const {

	tableEntryConstIter iter;
	os << endl;
	os << "#########################################################" << endl;
	os << "############### Begin to dump schedule! #################" << endl;
	os << "#########################################################" << endl;
	for (iter = m_schedule.begin(); iter != m_schedule.end(); ++iter) {
		os << *iter << endl;
	}
	os << "#########################################################" << endl;
	os << "###############  End to dump schedule!  #################" << endl;
	os << "#########################################################" << endl;
	os << endl;

}
