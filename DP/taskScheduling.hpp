#ifndef _DYNAMIC_PROGRAMMING_H_
#define _DYNAMIC_PROGRAMMING_H_

#include <vector>

using std::vector;

class dynProg {
	private:
		int m_numOfTask; //Number of task
		int m_numOfVolt;
		double m_deadline;// The deadline of all the task
		vector<double>m_inputDuration; // The task duration on 1.0V
		vector<double>m_inputEnergy; // The task energy on 1.0V

		//double m_taskDuration[m_numOfTask][5];
		vector< vector<double> > m_taskDuration;// The task duration from 0.8V to 1.2V
		//double m_taskEnergy[m_numOfTask][5];
		vector< vector<double> > m_taskEnergy;// The task energy from 0.8V to 1.2V
		//double m_scheduleEnergy[m_numOfTask][m_deadline * 10 + 1];
		vector< vector<double> > m_scheduleEnergy;//Energy information recorded during scheduling
		//double m_scheduleVolt[m_numOfTask][m_deadline * 10 + 1];
		vector< vector<double> > m_scheduleVolt;//Voltage information recorded during  scheduling
		//double m_lastStepDuration[m_numOfTask][m_deadline * 10 + 1];
		vector< vector<double> > m_lastStepDuration;//Duration information recorded during scheduling

		vector<double>m_durationSet;//scheduled task set
		vector<double>m_voltSet;//scheduled voltage set

		double volSel[5];
	public:
			//default construction method :
			//Table the task energy and duration at each voltage
	    dynProg(int numOfTask, double deadline, vector<double> taskDuration, vector<double> taskEnergy);
			//memoried the voltage selection of each task at each time step	  
		void taskTimeline();
		  //taskScheduling method: recurrsive method for task schedule
	    void taskScheduling();
		vector<double>getDurationSet();
		vector<double>getVoltSet();
};
#endif
