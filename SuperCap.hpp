#ifndef _SUPER_CAP_H_
#define _SUPER_CAP_H_

class supcapacitor {
	public:
		//Default Constructor
		supcapacitor();
		//SuperCapacitor Model
		void SupCapCharge(double Iin, double Tdur, double &Vs, double &Qacc);
		bool SupCapOperating(double Iin, double VCTI, double delVCTI);
		bool SupCapMoreSeriesReconfig(); 
		double SupCapReconfig(double new_s, double new_p); 
		double SupCapGetRacc(void);
		double SupCapGetCacc(void);
		double SupCapGetQacc(void);
		double SupCapGetEnergy(void);
		double SupCapGetVoc(void);
		void SupCapReset();
		void SupCapSetQacc(double Qacc);
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
