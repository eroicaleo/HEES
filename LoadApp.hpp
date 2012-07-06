#ifndef _LOAD_APP_H_
#define _LOAD_APP_H_

#include <deque>

#include "main.hpp"

class task_struct {
public:
	task_struct(double v, double i, double e) :
		vdd(v),
		idd(i),
		power(v*i),
		exec_time(e),
		remain_time(e) 
		{}
	double GetPower() const { return power; }
	double GetVDD() const { return vdd; }
	double GetIDD() const { return idd; }
	double AdvanceTaskProgress(double t) { 
		remain_time -= t;
		if (remain_time <= time_near_zero)
			remain_time = 0.0;
		return remain_time; 
	}
	double GetRemainingTime() const { return remain_time; }
private:
	double vdd;
	double idd;
	double power;
	double exec_time;
	double remain_time;
};

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
		// double get_deadline();
		// double get_exec_time();

	// Task related
	public:
		void InitilizeTaskQueue();
		void AdvanceLoadProgress(double t);
		double CurrentTaskRemainingTime() const;
		bool IsNoTaskToDo() const;
	private:
		void ContextSwitch();
		std::deque<task_struct> task_queue;
		task_struct *current_task;
		
	private:
		double m_Ta, m_Tb;
		double m_Pa, m_Pb, m_Pc;

		double vdd, idd;
		double deadline, exec_time;

		double PowConsumption(double Vdd);
		double TimeDuration(double Vdd);
};
#endif
