#ifndef _SUPER_CAP_H_
#define _SUPER_CAP_H_

#include "ees_bank.hpp"

class supcapacitor : public ees_bank {
	public:
		//Default Constructor
		supcapacitor();
		//SuperCapacitor Model
		void SupCapCharge(double Iin, double Tdur, double &Vs, double &Qacc);
		bool SupCapOperating(double Iin, double VCTI, double delVCTI);
		bool SupCapMoreSeriesReconfig(); 
		double SupCapReconfig(double new_s, double new_p); 
		double SupCapGetRacc(void) const;
		double SupCapGetCacc(void) const;
		double SupCapGetQacc(void) const;
		double SupCapGetEnergy(void) const;
		double SupCapGetVoc(void) const;
		void SupCapReset();
		void SupCapSetQacc(double Qacc);
	/* Inherited public interface from base class ees_bank */
	public:
		virtual double EESBankGetCacc() const;
		virtual double EESBankGetVoc() const;
		virtual double EESBankGetQacc() const;
		virtual double EESBankGetRacc() const;
		virtual double EESBankGetEnergy() const;
		virtual bool EESBankOperating(double, double, double);
		virtual	void EESBankCharge(double Iin, double Tdur, double &Vs, double &Qacc);
		
	private:
		double m_p;
		double m_s;
		double m_Totl;
		double m_Racc, m_Racc1;
		double m_Rs, m_Rp;
		double m_Rbank;
		double m_Qacc;
		double m_Cacc, m_Cacc1;
		double m_Energy;
};
#endif
