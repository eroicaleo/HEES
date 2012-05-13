#ifndef _DC_CONVERTER_OUT_H_
#define _DC_CONVERTER_OUT_H_

#include <vector>

using std::vector;

class lionbat;
class supcapacitor;

class dcconvertOUT {
	public:
		//Default Constructor
		dcconvertOUT();
		//Converter model
//		void ConverterModel_LionBat(double Vin, double Iin, double &Vout, double &Iout, double &Pdcdc, lionbat *lion_battery);
		void ConverterModel_SupCap(double Vin, double Iin, double &Vout, double &Iout, double &Pdcdc, supcapacitor *sp);
	private:
		vector<double> m_Rsw;
		vector<double> m_Qsw;
		double m_RL, m_RC;
		double m_Ictrl;
		double m_fs;
		double m_Lf;

};
#endif
