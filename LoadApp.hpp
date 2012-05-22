#ifndef _LOAD_APP_H_
#define _LOAD_APP_H_

class loadApplication {
	public:
		//Default Constructor
		loadApplication();
		//Load Model
		double LoadModel(double Vdd, double &Iload, double &Tdur);
		// Set parameters
		void SetTaskParameters(double vdd, double idd, double deadline, double exec_time);
		// Get parameters
		double get_vdd();
		double get_idd();
		double get_deadline();
		double get_exec_time();

	private:
		double m_Ta, m_Tb;
		double m_Pa, m_Pb, m_Pc;

		double vdd, idd;
		double deadline, exec_time;

		double PowConsumption(double Vdd);
		double TimeDuration(double Vdd);
};
#endif
