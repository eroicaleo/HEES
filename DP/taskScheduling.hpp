#ifndef _DYNAMIC_PROGRAMMING_H_
#define _DYNAMIC_PROGRAMMING_H_

#include <fstream>
#include <stack>
#include <tr1/functional>
#include <vector>
#include <iostream>
#include <iomanip>

#include "../DCCon_in.hpp"
#include "../VoltageTable.hpp"

using std::vector;
using std::ostream;

struct dpTableEntry;
typedef vector<dpTableEntry>::iterator tableEntryIter;
typedef vector<dpTableEntry>::const_iterator tableEntryConstIter;
typedef vector<vector<dpTableEntry> >::iterator tableRowIter;
typedef vector<vector<dpTableEntry> >::reverse_iterator tableRowRIter;

class dynProg {
	private:
		int m_numOfTask; //Number of task
		int m_numOfVolt;
		int m_deadline;// The deadline of all the task

		//double m_taskDuration[m_numOfTask][5];
		vector< vector<int> > m_taskDuration;// The task duration from 0.8V to 1.2V
		vector< vector<double> > m_taskCurrent;//Current I information recorded during scheduling
		//double m_scheduleEnergy[m_numOfTask][m_deadline * 10 + 1];
		vector< vector<double> > m_scheduleEnergy;//Energy information recorded during scheduling

		// The processor available voltage range
		vector<double> volSel;

		// The real task voltage table
		vector<TaskVoltageTable> realTaskVoltageTable;

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
	    dynProg(int numOfTask, int deadline, const vector<TaskVoltageTable> &vec_tvt);
		//memoried the voltage selection of each task at each time step	  
		void taskTimeline();
		//taskScheduling method: recurrsive method for task schedule
	    void taskScheduling();
		void taskTimelineWithIdle();
		vector<double>getDurationSet();
		vector<double>getVoltSet();
		int getDeadline() const { return m_deadline; }
		double getOptimalScheduleEnergy() const;
		int getOptimalScheduleFinishTime() const;

		// The function object take two variables, current energy and task length
		std::tr1::function<double(double, double, double)> energyCalculator;
		double energyCalculatorWrapper(double inputPower, double initialEnergy, double len);

		void backTracing();
		int genScheduleForEES();
		void backTracingWithIdleTasks();
		void dynamicProgrammingWithIdleTasks();

	private:
		double getExtraChargePower(int taskID, int volLevel, double solarPower);
		double getExtraChargePower(const TaskVoltageTable &tvt, size_t volLevel, double solarPower);
		void populateFirstIdleTask(vector<dpTableEntry> &);
		void populateRealTask(const vector<dpTableEntry> &lastIdleRow, vector<dpTableEntry> &thisRealRow);
		void populateIdleTask(const vector<dpTableEntry> &lastRealRow, vector<dpTableEntry> &thisIdleRow);
		void findMaxEnergyTableEntry(tableRowRIter &row, tableEntryIter &col);
		void dumpOptimalSchedule();
		void dumpDPTable();
		void initDPSolarPower();
		double energyCalVarSolarPowerWrapper(tableEntryIter begin, tableEntryIter end, const TaskVoltageTable &tvt, size_t volLevel, double initEnergy);
};

struct dpTableEntry {

	// We use the compiler generated copy constructor and operator=
	// Should be OK, because they are all built-in type.
	dpTableEntry(const dpTableEntry &);

	double totalEnergy;
	double voltage;
	double current;
	int volLevel;
	int taskID;
	int len;

	// Record when the last task finishes must be non-negative number.
	size_t lastTaskFinishTime;

	// solar power at this entry
	double solarPower;

	dpTableEntry();
	void setAllFields(double e, double v, double c, int vl, int id, int l, size_t f) {
		totalEnergy = e;
		voltage = v;
		current = c;
		volLevel = vl;
		taskID = id;
		len = l;
		lastTaskFinishTime = f;
	}

	bool operator<(const dpTableEntry &d) {
		return this->totalEnergy < d.totalEnergy;
	}

};

class dpSolarComparator {
	public:
		dpSolarComparator(double sp) : p(sp) {}
		bool operator()(const dpTableEntry &e) const { return (e.solarPower != p); }
	private:
		double p;
};

ostream& operator<<(ostream& os, const dpTableEntry &e);

bool dpTableEntryComp(const dpTableEntry &a, const dpTableEntry &b);

double energyCalculatorApproximation(double inputPower, double initialEnergy, double len);

void readInput(vector<double> &InDuration, vector<double> &InEnergy, double &deadline);

#endif
