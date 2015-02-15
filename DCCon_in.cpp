#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include "DCCon_in.hpp"
#include <vector>

#include "ParseCommandLine.hpp"

using namespace std;

dcconvertIN::dcconvertIN() :
	m_Rsw(4, 25e-3),
	m_Qsw(4, 60e-9),
	m_RL(39e-3),
	m_RC(100e-3),
	m_Ictrl(4e-3),
	m_fs(500e3),
	m_Lf(6e-6) { }

void dcconvertIN::ConverterModel(double Vin, double Vout, double Iout, double &Iin, double &Pdcdc) const {
	double m_Iin = 0.0;
	double m_Pdcdc = 0.0;

	if (!dc_load_is_ideal) {
		if (Vin > Vout) { //buck mode
			BuckMode(Vin, Vout, Iout, m_Iin, m_Pdcdc);
			Iin = m_Iin;
			Pdcdc = m_Pdcdc;
		} else { //boost mode
			BoostMode(Vin, Vout, Iout, m_Iin, m_Pdcdc);
			Iin = m_Iin;
			Pdcdc = m_Pdcdc;
		}
	} else {
		Iin = Iout;
		Pdcdc = m_Pdcdc;
	}

	this->m_Pdcdc = m_Pdcdc;
}

double dcconvertIN::GetPowerConsumptionWithLoad(double loadvol, double loadcur) const {

    double dc_load_vin(1.0), dc_load_vout(loadvol), dc_load_iout(loadcur);
	double dc_load_iin(0.0), dc_load_power(0.0);
	ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

	return dc_load_vin*dc_load_iin;
}

void dcconvertIN::BuckMode(double Vin, double Vout, double Iout, double &Iin, double &Pdcdc) const {
	double m_Iin = 0.0;
	double m_Pdcdc = 0.0;
	double m_D = 0.0;
	double m_delI = 0.0;

	m_D = Vout / Vin;
	m_delI = (Vout * (1 - m_D)) / (m_Lf * m_fs);

	m_Pdcdc = Iout * Iout * (m_RL + m_D * m_Rsw[0] + (1 - m_D) * m_Rsw[1] + m_Rsw[3]) + ((m_delI * m_delI) / 12) * (m_RL + m_D * m_Rsw[0] + (1 - m_D) * m_Rsw[1] + m_Rsw[3] + m_RC) + Vin * m_fs * (m_Qsw[0] + m_Qsw[1]) + Vin * m_Ictrl;

	m_Iin = (Vout * Iout + m_Pdcdc) / Vin;

	Iin = m_Iin;
	Pdcdc = m_Pdcdc;
}

void dcconvertIN::BoostMode(double Vin, double Vout, double Iout, double &Iin, double &Pdcdc) const {
    double m_Iin = 0.0;
    double m_Pdcdc = 0.0;
    double m_D = 0.0;
    double m_delI = 0.0;

    m_D = 1 - (Vin / Vout);
    m_delI = (Vin * m_D) / (m_Lf * m_fs);

    m_Pdcdc = ((Iout * Iout) / ((1 - m_D) * (1 - m_D))) * (m_RL + m_D * m_Rsw[2] + (1 - m_D) * m_Rsw[3] + m_Rsw[0] + m_D * (1 - m_D) * m_RC) + ((m_delI * m_delI) / 12) * (m_RL + m_D * m_Rsw[2] + (1 - m_D) * m_Rsw[3] + m_Rsw[0] + (1 - m_D) * m_RC) + Vout * m_fs * (m_Qsw[2] + m_Qsw[3]) + Vin * m_Ictrl;

    m_Iin = (Vout * Iout + m_Pdcdc) / Vin;

    Iin = m_Iin;
    Pdcdc = m_Pdcdc;
}	
