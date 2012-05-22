#ifndef _LOAD_APP_H_
#define _LOAD_APP_H_

class loadApplication {
	public:
		//Default Constructor
		loadApplication();
		//Load Model
		double LoadModel(double Vdd, double &Iload, double &Tdur);

	private:
		double m_Ta, m_Tb;
		double m_Pa, m_Pb, m_Pc;

		double PowConsumption(double Vdd);
		double TimeDuration(double Vdd);
};
#endif
