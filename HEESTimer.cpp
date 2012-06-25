#include <fstream>
#include <iostream>

#include "HEESTimer.hpp"

using namespace std;

HEESTimer::HEESTimer(double start_time_sec, int record_step) :
	CurrentTimeInSecond(start_time_sec),
	CurrentTimeIndex(0),
	RecordStep(record_step),
	LastRecordedEnergy(0.0),
	RecordFileName("RecordFile.txt")
{
	ofstream output(RecordFileName.c_str());
	output << "time_index\t\tenergy\t\tdelta_energy" << endl;
	output.close();
}

void HEESTimer::HEESTimerSetCurrentSecond(int curr_time_sec) {
	CurrentTimeInSecond = curr_time_sec;
	return;
}

void HEESTimer::HEESTimerSetRecordStep(int record_step) {
	RecordStep = record_step;
	return;
}

void HEESTimer::HEESTimerAdvancdTimerIndex(int TimerIndex, ees_bank *bank) {
	CurrentTimeIndex += TimerIndex;
	CurrentTimeInSecond += TimerIndex * min_time_interval;
	if (CurrentTimeIndex % RecordStep == 0) {
		RecordNewEnergy(bank);
	}
	return;
}

void HEESTimer::RecordNewEnergy(ees_bank *bank) {
	double curr_energy = bank->EESBankGetEnergy();
	double delta_energy = curr_energy - LastRecordedEnergy;
	ofstream output(RecordFileName.c_str(), ios_base::app);
	output << CurrentTimeIndex << "\t\t" << curr_energy << "\t\t" << delta_energy << "\n";
	output.close();
	LastRecordedEnergy = curr_energy;
	return;
}

int HEESTimer::HEESTimerGetCurrentTimeIndex(void) const {
	return CurrentTimeIndex;
}

double HEESTimer::HEESTimerGetCurrentTimeInSecond(void) const {
	return CurrentTimeInSecond;
}
