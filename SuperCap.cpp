#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include "SuperCap.hpp"
#include "main.hpp"
#include <vector>
 using namespace std;

supcapacitor::supcapacitor() :
	m_p(1),
	m_s(4),
	m_Totl(4),
	m_Racc(0.0), m_Racc1(34e-3),
	m_Rs(10e-3), m_Rp(10e-3),
	m_Rbank(0),
    m_Qacc(0.0),
	m_Cacc1(100.0), m_Cacc(m_p / m_s * m_Cacc1) {
}

void supcapacitor::SupCapCharge(double Iin, double Vin, double Tdur, double &Vs, double &Qacc) {
	
	// Recalculate the charge
	m_Qacc = m_Qacc + Iin * Tdur;

	// Recalculate the energy
	m_Energy = (0.5)*(m_Qacc*m_Qacc)/m_Cacc;

	Qacc = m_Qacc;
	Vs = m_Qacc / m_Cacc + Iin * m_Rbank;
}

bool supcapacitor::SupCapOperating(double Iin, double VCTI, double delVCTI){
	double m_Vs = 0.0;
	double m_R = 0.0;
	double m_C = 0.0;
	double m_del = 0;
	double m_Combp = m_p;
	double m_Combs = m_s;
	double m_vsdel = -100.0;

	double voc_orig = m_Qacc / this->m_Cacc;

	bool flag = false;
    
	vector<double>m_comb;
	for(int i = 1; i <= m_Totl; i++){
		if((int)m_Totl % i == 0){
			m_comb.push_back((double)i);
		}
	}

	while(m_comb.size() > 0){
		double p = 0, s =0;
		p = m_comb.back();
		s = m_Totl / p;
		m_R = (2.0/3.0 * p - 1.0 + 1.0/(3.0 * p)) * s * m_Rp + s / p  * m_Racc1 + (s - 1.0) * m_Rs;// m_Racc
        m_C = p / s * m_Cacc1;//m_Cacc
        m_Vs = m_Qacc * sqrt(m_C/m_Cacc) / m_C;
        m_del = m_Vs - VCTI;
		// Choose the V_cap such that it is smaller and close to V_cti
		// but less than the current V_oc
        if((m_del > m_vsdel) && (m_del < 0) && (m_Vs < voc_orig)){
        	m_Combp = p;
            m_Combs = s;
            m_vsdel = m_del;
			flag = true;
        }
		m_comb.pop_back();
	}

	if (flag) {
		cout<<"------p------"<<m_Combp<<"----s----"<<m_Combs<<endl;
		// Make the reconfiguration here!
		SupCapReconfig(m_Combs, m_Combp);
	}

	return flag;
}

bool supcapacitor::SupCapMoreSeriesReconfig() {
	int new_s = (int)m_s + 1;
	do {
		if ((int)m_Totl % new_s == 0) {
			SupCapReconfig(new_s, m_Totl/new_s);
			return true;
		}
	} while (new_s++ < m_Totl);
	return false;
}

double supcapacitor::SupCapReconfig(double new_s, double new_p) {
	double new_C = new_p / new_s * m_Cacc1;
	this->m_Totl = new_s * new_p;

	// E_sup = (1/2)*Q^2/C
	// After reconfiguration, the energy is same
	double new_Q = m_Qacc * sqrt(new_C / m_Cacc);

	m_Qacc = new_Q;
	m_Cacc = new_C;
	m_p = new_p;
	m_s = new_s;
	m_Rbank = (2.0/3.0 * m_p - 1.0 + 1.0/(3.0 * m_p)) * m_s * m_Rp + m_s / m_p  * m_Racc1 + (m_s - 1.0) * m_Rs;
	m_Racc = m_s / m_p * m_Racc1;

	return new_Q;
}

bool supcapacitor::SupCapReconfiguration(double Iin, double VCTI, dcconvertOUT *dc_super_cap) {
	/*
	 * We start from the most serial config
	 */
	int total = (int)m_Totl, s = total, p = 1;
	bool flag = false;
	/*
	 * If there is no energy stored in the bank, just set most serial config
	 */
	if (SupCapGetEnergy() < near_zero) {
		SupCapReconfig(s, p);
		return true;
	}
	for (; s > 0; --s) {
		if (total % s == 0) {
			p = total / s;
			SupCapReconfig(s, p);

			double Iout, Vout, Power;
			dc_super_cap->ConverterModel_EESBank(VCTI, Iin, Vout, Iout, Power, this);
			/* 
			 * If we found the Vout < Vin, 
			 * this means we found the current best config
			 * because we start from the most serial config
			 */
			if (Vout < VCTI) {
				flag = true;
				break;
			}
		}
	}
	return true;
}

double supcapacitor::SupCapGetRacc() const {
	return m_Rbank;
}
double supcapacitor::SupCapGetCacc() const {
	return m_Cacc;
}
double supcapacitor::SupCapGetQacc() const {
    return m_Qacc;
}
void supcapacitor::SupCapReset(){
	m_Qacc = 0;
	m_Cacc = m_p / m_s * m_Cacc1;
	m_Racc = m_s / m_p * m_Racc1;
	m_Rbank = (2.0 / 3.0 * m_p - 1.0 + 1.0/(3.0 * m_p)) * m_s * m_Rp + m_Racc + (m_s - 1.0) * m_Rs;

	m_Energy = (0.5)*(m_Qacc*m_Qacc)/m_Cacc;
}

void supcapacitor::SupCapSetQacc(double Qacc){
	m_Qacc = Qacc;
	m_Energy = (0.5)*(m_Qacc*m_Qacc)/m_Cacc;
}

double supcapacitor::SupCapGetEnergy(void) const {
	if (m_Qacc < 0)
		return -1.0;
	return m_Energy;
}

double supcapacitor::SupCapGetVoc(void) const {
	return (m_Qacc/m_Cacc);
}

bool supcapacitor::SupCapIsFullySerial(void) const {
	return (m_p == 1);
}

bool supcapacitor::SupCapIsFullyParallel(void) const {
	return (m_s == 1);
}

/* Implement the interface inherited from base class ees_bank */
double supcapacitor::EESBankGetCacc() const {
	return SupCapGetCacc();
}

double supcapacitor::EESBankGetVoc() const {
	return SupCapGetVoc();
}

double supcapacitor::EESBankGetQacc() const {
	return SupCapGetQacc();
}

double supcapacitor::EESBankGetRacc() const {
	return SupCapGetRacc();
}

double supcapacitor::EESBankGetEnergy() const {
	return SupCapGetEnergy();
}

bool supcapacitor::EESBankOperating(double Iin, double VCTI, double delVCTI) {
	return SupCapOperating(Iin, VCTI, delVCTI);
}

bool supcapacitor::EESBankReconfiguration(double Iin, double VCTI, dcconvertOUT *dc_super_cap) {
	return SupCapReconfiguration(Iin, VCTI, dc_super_cap);
}

void supcapacitor::EESBankCharge(double Iin, double Vin, double Tdur, double &Vs, double &Qacc) {
	return SupCapCharge(Iin, Vin, Tdur, Vs, Qacc);
}
