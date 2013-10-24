#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include "LionBat.hpp"

lionbat::lionbat() {
	double m_b_arr[21] = {-0.67, -16.21, -0.03, 1.28, -0.40, 7.55, 0.10, -4.32, 0.34, 0.15, -19.60, 0.19, -72.39, -40.83, 102.80, 2.07, -190.41, 0.20, -695.30, -110.63, 611.50};
	vector<double> m_b_vec(m_b_arr, m_b_arr + sizeof(m_b_arr)/sizeof(double));

	m_VSOC = 0.0;
	m_b = m_b_vec;
	m_Rsd = 0.0;
	m_Cb = 3600 * 0.35; //350 mah
	m_Cinit = 0.0;
}
// out[0]: Vcc; out[1]: Vsoc; out[2]
void lionbat::BatteryModel(double Ilin, double Tdur, double &Voc, double &Vsoc){
	double m_Rout = 0.0;
	double m_Voc = 0.0, m_Vsoc = 0.0;
	double m_Rs = 0.0, m_Rts = 0.0, m_Rtl = 0.0;
	double m_Cts = 0.0, m_Ctl = 0.0;
	
	m_Vsoc = VsocCheck(Ilin, Tdur);
	m_Voc = m_b[0] * exp(m_b[1] * m_Vsoc) + m_b[2] * (m_Vsoc * m_Vsoc * m_Vsoc) + m_b[3] * (m_Vsoc * m_Vsoc) + m_b[4] * m_Vsoc + m_b[5];
	
	m_Rs = m_b[6] * exp(m_b[7] * m_Vsoc) + m_b[8];
	m_Rts = m_b[9] * exp(m_b[10] * m_Vsoc) + m_b[11];
	m_Rtl = m_b[15] * exp(m_b[16] * m_Vsoc) + m_b[17];

	m_Cts = m_b[12] * exp(m_b[13] * m_Vsoc) + m_b[14];
	m_Ctl = m_b[18] * exp(m_b[19] * m_Vsoc) + m_b[20];
	m_Rout = m_Rs + m_Rts + m_Rtl;

    Voc = m_Voc;
	Vsoc = m_Vsoc;
}

double lionbat::VsocCheck(double Iin, double Tdur){
	
	return m_VSOC = m_VSOC + (Iin * Tdur)/ m_Cb;
}

void lionbat::BatterySetVsoc(double Vsoc) {
	m_VSOC = Vsoc;
}

double lionbat::BatteryGetVsoc(void) {
	return m_VSOC;
}

double lionbat::BatteryGetVoc(double Vsoc) {
	double ret = 0.0;
	return ret = m_b[0] * exp(m_b[1] * Vsoc) + m_b[2] * (Vsoc * Vsoc * Vsoc) + m_b[3] * (Vsoc * Vsoc) + m_b[4] * Vsoc + m_b[5];
}
