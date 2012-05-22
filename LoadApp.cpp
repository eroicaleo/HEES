#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include "LoadApp.hpp"

loadApplication::loadApplication() :
	m_Ta(-0.0111),
	m_Tb(0.0164),
	m_Pa(470.59),
	m_Pb(-83.59),
	m_Pc(-164.05){
}

double loadApplication::LoadModel(double Vdd, double &Iload, double &Tur){
	double m_Iload = 0.0;
	double m_Tdur = 0.0;
	double m_Pow = 0.0;

	m_Tdur = TimeDuration(Vdd);
	
	m_Pow = PowConsumption(Vdd);
	m_Iload = m_Pow / Vdd;

	Iload = m_Iload;
	Tur = m_Tdur;

	return m_Pow * m_Tdur;
}

double loadApplication::PowConsumption(double Vdd){
	double m_ret = 0.0;

	return m_ret = (m_Pa * Vdd * Vdd * Vdd + m_Pb * Vdd * Vdd + m_Pc * Vdd) / 100;
}

double loadApplication::TimeDuration(double Vdd){
	double m_ret = 0.0;

	return m_ret = (m_Ta * Vdd + m_Tb) * 1000;
}
