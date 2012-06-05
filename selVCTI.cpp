#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include "DCCon_in.hpp"
#include "DCCon_out.hpp"
#include "DCCon_dis.hpp"
#include "SuperCap.hpp"
#include "LionBat.hpp"
#include "ees_bank.hpp"
#include "selVCTI.hpp"
#include <vector>
 using namespace std;

selVcti::selVcti() :
 	Vmax(11.0),
	Vmin(0.1),
	alpha(0.23),
	epsilon(0.1){}

double selVcti :: bestVCTI(double input_Power, double dccon1_Iout, double dccon1_Vout, string selMode, ees_bank *sp){
	dcconvertIN m_dcin;
	dcconvertOUT m_dcout;
	dcconvertDIS m_dcdis;

	// double m_Vsoc;
	double m_Qacc;
	double m_dccon1_Iout = 0.0, m_dccon1_Vout = 0.0, m_dccon1_Iin = 0.0, m_dccon1_Vin = 0.0, m_dccon1_Pdcdc = 0.0;
	double m_dccon2_Iout = 0.0, m_dccon2_Vout = 0.0, m_dccon2_Iin = 0.0, m_dccon2_Vin = 0.0, m_dccon2_Pdcdc = 0.0;
	double m_Vmax = 0.0, m_Vmin = 0.0, m_VCTI = 0.0;
	double m_Pup = 0, m_Pdn = 0, m_Pdcdc = 0;
	double m_ret = 0.0;
	int m_selMode = 0;

	if(selMode == "out_LionBat"){
		m_selMode = 0;
	}else if(selMode == "out_SupCap"){
		m_selMode = 1;
	}else if(selMode == "dis_LionBat"){
		m_selMode = 2;
	}else if(selMode == "dis_SupCap"){
		m_selMode = 3;
	}else{
		cerr<<"-----------No Such Mode Existed-------------"<<endl;
	}
    m_Vmax = Vmax;
    m_Vmin  = Vmin;
    while (m_Vmax - m_Vmin > epsilon){
		//VCTImax
    	m_VCTI = (1 - alpha) * m_Vmin + alpha * m_Vmax;
		// m_Vsoc = lb -> BatteryGetVsoc();
		m_Qacc = sp -> EESBankGetQacc();
		
		m_dccon1_Iout = dccon1_Iout;
        m_dccon1_Vout = dccon1_Vout;
        m_dccon1_Vin = m_VCTI;
       	m_dcin.ConverterModel(m_dccon1_Vin, m_dccon1_Vout, m_dccon1_Iout, m_dccon1_Iin, m_dccon1_Pdcdc);
		switch (m_selMode){
    		case 0:
				m_dccon2_Vin = m_VCTI;
				m_dccon2_Iin = input_Power/m_VCTI - m_dccon1_Iin;
				if (m_dccon2_Iin <= 0) {
					m_dccon2_Pdcdc = 100;
				} else {
					// m_dcout.ConverterModel_LionBat(m_dccon2_Vin, m_dccon2_Iin, m_dccon2_Vout, m_dccon2_Iout, m_dccon2_Pdcdc, lb); 
				}
			break;
    		case 1:
			    m_dccon2_Vin = m_VCTI;
                m_dccon2_Iin = input_Power/m_VCTI - m_dccon1_Iin;
				if (m_dccon2_Iin <= 0) {
					m_dccon2_Pdcdc = 100;
				} else {
                	m_dcout.ConverterModel_EESBank(m_dccon2_Vin, m_dccon2_Iin, m_dccon2_Vout, m_dccon2_Iout, m_dccon2_Pdcdc, sp);
				}
            break;   
    		case 2:
				m_dccon2_Vout = m_VCTI;
                m_dccon2_Iout = m_dccon1_Iin;
				// m_dcdis.ConverterModel_LionBat(m_dccon2_Vout, m_dccon2_Iout, m_dccon2_Vin, m_dccon2_Iin, m_dccon2_Pdcdc, lb); 
            break;
			case 3:
				m_dccon2_Vout = m_VCTI;
                m_dccon2_Iout = m_dccon1_Iin;
                // m_dcdis.ConverterModel_supcap(m_dccon2_Vout, m_dccon2_Iout, m_dccon2_Vin, m_dccon2_Iin, m_dccon2_Pdcdc, sp);
 			break; 
		}
		m_Pup= m_dccon1_Pdcdc + m_dccon2_Pdcdc;
	    //VCTImin
		// lb->BatterySetVsoc(m_Vsoc);

		m_VCTI = (1 - alpha) * m_Vmax + alpha * m_Vmin;
		m_dccon1_Iout = dccon1_Iout;
        m_dccon1_Vout = dccon1_Vout;
        m_dccon1_Vin = m_VCTI;
		m_dcin.ConverterModel(m_dccon1_Vin, m_dccon1_Vout, m_dccon1_Iout, m_dccon1_Iin, m_dccon1_Pdcdc);
        switch (m_selMode){
            case 0:
                m_dccon2_Vin = m_VCTI;
                m_dccon2_Iin = input_Power/m_VCTI - m_dccon1_Iin;
				if (m_dccon2_Iin <= 0) {
					m_dccon2_Pdcdc = 100;
				} else {
                	// m_dcout.ConverterModel_LionBat(m_dccon2_Vin, m_dccon2_Iin, m_dccon2_Vout, m_dccon2_Iout, m_dccon2_Pdcdc, lb);
				}
            break;
            case 1:
                m_dccon2_Vin = m_VCTI;
                m_dccon2_Iin = input_Power/m_VCTI - m_dccon1_Iin;
				if (m_dccon2_Iin <= 0) {
					m_dccon2_Pdcdc = 100;
				} else {
                	m_dcout.ConverterModel_EESBank(m_dccon2_Vin, m_dccon2_Iin, m_dccon2_Vout, m_dccon2_Iout, m_dccon2_Pdcdc, sp);
				}
            break;
            case 2:
                m_dccon2_Vout = m_VCTI;
                m_dccon2_Iout = m_dccon1_Iin;
//                m_dis.ConverterModel_SupCap(m_dccon2_Vout, m_dccon2_Iout, m_dccon2_Vin, m_dccon2_Iin, m_dccon2_Pdcdc, lb);
            break;
            case 3:
                m_dccon2_Vout = m_VCTI;
                m_dccon2_Iout = m_dccon1_Iin;
                // m_dcdis.ConverterModel_supcap(m_dccon2_Vout, m_dccon2_Iout, m_dccon2_Vin, m_dccon2_Iin, m_dccon2_Pdcdc, sp);
            break;
        }
        m_Pdn= m_dccon1_Pdcdc + m_dccon2_Pdcdc;		
		//Compare
		if(m_Pup > m_Pdn){
                    m_Vmin = (1 - alpha) * m_Vmin + alpha * m_Vmax;
                    m_VCTI = m_Vmin;
                    m_Pdcdc = m_Pup;
                }else{
                    m_Vmax = (1 - alpha) * m_Vmax + alpha * m_Vmin;
                    m_VCTI = m_Vmax;
                    m_Pdcdc = m_Pdn;
                }
            }
		return m_ret = m_VCTI;
}
