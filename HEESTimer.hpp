#ifndef _HEES_TIMER_
#define _HEES_TIMER_

#include <string>

#include "ees_bank.hpp"
#include "main.hpp"

using std::string;

class HEESTimer {
public:
	HEESTimer(double start_time_sec, int record_step);
	void HEESTimerAdvancdTimerIndex(int TimerIndex, ees_bank *bank);
	void HEESTimerSetCurrentSecond(int curr_time_sec);
	int HEESTimerGetCurrentTimeIndex(void);
	double HEESTimerGetCurrentTimeInSecond(void); 
private:
	void RecordNewEnergy(ees_bank *bank);
private:
	double CurrentTimeInSecond;
	int CurrentTimeIndex;
	int RecordStep;
	double LastRecordedEnergy;
	string RecordFileName;
};

#endif
