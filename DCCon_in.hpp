#ifndef _DC_CONVERTER_IN_H_
#define _DC_CONVERTER_IN_H_

#include <vector>

using std::vector;

class dcconvertIN {
	public:
		//Default Constructor
		dcconvertIN();
		//Converter model
		void ConverterModel(double Vin, double Vout, double Iout, double &Iin, double &Pdcdc);
	private:
		vector<double> m_Rsw;
		vector<double> m_Qsw;
		double m_RL, m_RC;
		double m_Ictrl;
		double m_fs;
		double m_Lf;
		double m_Pdcdc;

		void BuckMode(double Vin, double Vout, double Iout, double &Iin, double &Pdcdc);
		void BoostMode(double Vin, double Vout, double Iout, double &Iin, double &Pdcdc);
};
#endif
