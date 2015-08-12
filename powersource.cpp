#include <stdio.h>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "main.hpp"
#include "powersource.hpp"

using namespace std;
#define PI 3.1415926

const static double m_pwMax = 679.6755;
const static double m_start = 29520, m_end = 75690;
const static double scaled_power_max = 2.00;
const static double scale_ratio = scaled_power_max / m_pwMax;

double powersource(double stHour, double stMin, double stSec) {
	double m_power;
	double m_Tdur = 0.0;

	m_Tdur = stHour * 60 * 60 + stMin * 60 + stSec;
	if (m_Tdur < 29520) {
		return 0;
	} else if (m_Tdur > 75690) {
		return 0;
	} else {
		return	m_power = scale_ratio * m_pwMax * sin((m_Tdur - 29520.0)/ (75690.0 -29520.0)* PI);
	}
}

double solar_power_source_sec(double m_Tdur) {
	double m_power;

	if (m_Tdur < 29520) {
		return 0;
	} else if (m_Tdur > 75690) {
		return 0;
	} else {
		return	m_power = scale_ratio * m_pwMax * sin((m_Tdur - 29520.0)/ (75690.0 -29520.0)* PI);
	}
}

void VariablePowerSource::ReadVariablePowerSource(string filename) {
	ifstream infile(filename.c_str());
	if (!infile) {
		cerr << "Can not find variable power source file! Abort!" << endl;
		throw 23;
	}

	double power(0.0), len(0.0);
	while ((infile >> power >> len).good()) {
		PowerPeriod p1(power, len);
		PowerPeriodQueue.push_back(p1);
	}

	infile.close();
}

/*
 * arguments:
 * t : the current time in second, not the length
 *     the length is computed as the current time - last access time
 */
double VariablePowerSource::AdvanceVariablePowerSource(double t) {
	double len(1.0);
	if (LastTimeAccessInSeconds < 0) {
		len = 0.0;
	} else {
		len = t - LastTimeAccessInSeconds;
	}
	LastTimeAccessInSeconds = t;
	return AdvanceVariablePowerSourceCore(len);
}

double VariablePowerSource::AdvanceVariablePowerSourceCore(double len) {
	PowerPeriod &CurrentPowerPeriod = PowerPeriodQueue.front();
	// Take care of the power period progress
	// and switching
	if (CurrentPowerPeriod.RemainingTime > near_zero) {
		CurrentPowerPeriod.RemainingTime -= len;
	} else {
		if (PowerPeriodQueue.size() > 1) {
			PowerPeriodQueue.pop_front();
			PowerPeriodQueue.front().RemainingTime -= len;
			return PowerPeriodQueue.front().PowerValue;
		}
	}
	return CurrentPowerPeriod.PowerValue;
}

#ifdef _POWER_SOURCE_MAIN_
int main(){
	string filename("powersource.txt");
	FILE *fp = fopen(filename.c_str(),"w");

	for(double h = 0; h < 24; h++){
		for(double m = 0; m < 60; m++){
			for(double s = 0; s < 60; s++){
			fprintf(fp,"%f\n",powersource(h, m, s));
			}
		}
	}
}
#endif
