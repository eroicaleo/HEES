#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include "SuperCap.hpp"
#include <vector>
 using namespace std;

supcapacitor::supcapacitor() :
	m_p(4),
	m_s(1),
	m_Totl(4),
	m_Racc(0.0), m_Racc1(34e-3),
	m_Rs(10e-3), m_Rp(10e-3),
	m_Rbank(0),
    m_Qacc(0.0),
	m_Cacc(0.0), m_Cacc1(100){
}

void supcapacitor::SupCapCharge(double Iin, double Tdur, double &Vs, double &Qacc){
	double m_Vs =0;
	//Qacc = It, Vs = Qacc/Cacc + Iin*Racc
	//Rbank =(2/3 * n - 1 + 1/(3 * n)) * n * Rp + n/m * Racc + (m - 1) * Rs
	//Cacc  = m/n * Cacc1
	m_Cacc = m_p / m_s * m_Cacc1;
	m_Racc = m_s / m_p * m_Racc1;
	m_Rbank = (2.0 / 3.0 * m_p - 1.0 + 1.0 /(3.0 * m_p)) * m_s * m_Rp + m_Racc + (m_s - 1.0) * m_Rs;
	m_Qacc = m_Qacc + Iin * Tdur;
	m_Vs = m_Qacc / m_Cacc + Iin * m_Rbank;

	Vs = m_Vs;
	Qacc = m_Qacc;
}

void supcapacitor::SupCapOperating(double Iin, double VCTI, double delVCTI){
	double m_Vs = 0.0;
	double m_R = 0.0;
	double m_C = 0.0;
	double m_del = 0;
	double m_Combp = m_p;
	double m_Combs = m_s;
	double m_vsdel = delVCTI;
    
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
        m_Vs = m_Qacc / m_C + Iin * m_R;
        m_del = m_Vs - VCTI;
        // m_del = fabs(m_del);
        if(m_del < m_vsdel){
        	m_Combp = p;
            m_Combs = s;
            m_vsdel = m_del;
        }
		m_comb.pop_back();
	}

	cout<<"------p------"<<m_Combp<<"----s----"<<m_Combs<<endl;
	m_p = m_Combp;
	m_s = m_Combs;
}
double supcapacitor::SupCapGetRacc(){
	return m_Rbank;
}
double supcapacitor::SupCapGetCacc(){
	return m_Cacc;
}
double supcapacitor::SupCapGetQacc(){
    return m_Qacc;
}
void supcapacitor::SupCapReset(){
	m_Qacc = 0;
	m_Cacc = m_p / m_s * m_Cacc1;
	m_Racc = m_s / m_p * m_Racc1;
	m_Rbank = (2.0 / 3.0 * m_p - 1.0 + 1.0/(3.0 * m_p)) * m_s * m_Rp + m_Racc + (m_s - 1.0) * m_Rs;

}

void supcapacitor::SupCapSetQacc(double Qacc){
	m_Qacc = Qacc;
}
