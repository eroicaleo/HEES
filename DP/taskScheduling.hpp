#ifndef _DYNAMIC_PROGRAMMING_H_
#define _DYNAMIC_PROGRAMMING_H_

#include <fstream>
#include <stack>
#include <tr1/functional>
#include <vector>

#include "../DCCon_in.hpp"

using std::vector;

struct dpTableEntry;

class dynProg {
	private:
		int m_numOfTask; //Number of task
		int m_numOfVolt;
		int m_deadline;// The deadline of all the task
		vector<double>m_inputDuration; // The task duration on 1.0V
		vector<double>m_inputEnergy; // The task energy on 1.0V

		//double m_taskDuration[m_numOfTask][5];
		vector< vector<int> > m_taskDuration;// The task duration from 0.8V to 1.2V
		//double m_taskEnergy[m_numOfTask][5];
		vector< vector<double> > m_taskEnergy;// The task energy from 0.8V to 1.2V
		vector< vector<double> > m_taskCurrent;//Current I information recorded during scheduling
		//double m_scheduleEnergy[m_numOfTask][m_deadline * 10 + 1];
		vector< vector<double> > m_scheduleEnergy;//Energy information recorded during scheduling
		//double m_scheduleVolt[m_numOfTask][m_deadline * 10 + 1];
		vector< vector<double> > m_scheduleVolt;//Voltage information recorded during  scheduling
		//double m_lastStepDuration[m_numOfTask][m_deadline * 10 + 1];
		vector< vector<double> > m_lastStepDuration;//Duration information recorded during scheduling

		vector<double>m_durationSet;//scheduled task set
		vector<double>m_voltSet;//scheduled voltage set

		vector<double> volSel;

		// DC-DC converter related variable
		dcconvertIN m_dcLoad;

		// System model related parameters
		double m_solarPower;
		double m_initialEnergy;

		std::stack<dpTableEntry> optimalSchedule;
		std::vector<vector<dpTableEntry> > m_scheduleWithIdleTask;

	public:
		//default construction method :
		//Table the task energy and duration at each voltage
	    dynProg(int numOfTask, vector<double> voltageTable, double deadline, vector<double> taskDuration, vector<double> taskEnergy);
		//memoried the voltage selection of each task at each time step	  
		void taskTimeline();
		//taskScheduling method: recurrsive method for task schedule
	    void taskScheduling();
		void taskTimelineWithIdle();
		vector<double>getDurationSet();
		vector<double>getVoltSet();

		// The function object take two variables, current energy and task length
		std::tr1::function<double(double, double, double)> energyCalculator;

		void backTracing();
		int genScheduleForEES();

	private:
		double getExtraChargePower(int taskIdx, int volLevel);
};

struct dpTableEntry {

	// We use the compiler generated copy constructor and operator=
	// Should be OK, because they are all built-in type.
	// dpTableEntry(const dpTableEntry &);

	double totalEnergy;
	double voltage;
	double current;
	int volLevel;
	int taskID;
	int len;

	// Record when the last task finishes must be non-negative number.
	size_t lastTaskFinishTime;

	dpTableEntry();
};

void readInput(vector<double> &InDuration, vector<double> &InEnergy, double &deadline);

#endif
