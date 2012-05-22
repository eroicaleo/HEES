#ifndef _DC_CONVERTER_DIS_H_
#define _DC_CONVERTER_DIS_H_

#include <vector>

using std::vector;

class lionbat;
class supcapacitor;

class dcconvertDIS {
	public:
		//Default Constructor
		dcconvertDIS();
		//Converter model
		void ConverterModel_battery(double Vout, double Iout, double &Vin, double &Iin, double &Pdcdc, lionbat *lion_battery);
		void ConverterModel_supcap(double Vout, double Iout, double &Vin, double &Iin, double &Pdcdc, supcapacitor *sp);
	private:
		vector<double> m_Rsw;
		vector<double> m_Qsw;
		double m_RL, m_RC;
		double m_Ictrl;
		double m_fs;
		double m_Lf;

};
#endif
