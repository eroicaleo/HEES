#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include "LoadApp.hpp"

using namespace std;

loadApplication::loadApplication() :
	m_Ta(-0.0111),
	m_Tb(0.0164),
	m_Pa(470.59),
	m_Pb(-83.59),
	m_Pc(-164.05)
	{
		InitilizeTaskQueue();
}

void loadApplication::SetTaskParameters(double vdd, double idd, double deadline, double exec_time) {
	this->vdd = vdd;
	this->idd = idd;
	this->deadline = deadline;
	this->exec_time = exec_time;

	return;
}

double loadApplication::get_vdd() {
	ContextSwitch();
	return current_task->GetVDD();
}

double loadApplication::get_idd() {
	ContextSwitch();
	return current_task->GetIDD();
}

double loadApplication::CurrentTaskRemainingTime() const {
	return current_task->GetRemainingTime();
}

bool loadApplication::IsNoTaskToDo() const {
	return (task_queue.empty());
}

/*
double loadApplication::get_deadline() {
	return deadline;
} */

/*
double loadApplication::get_exec_time() {
	return exec_time;
} */

double loadApplication::LoadModel(double Vdd, double &Iload, double &Tur){
	double m_Iload = 0.0;
	double m_Tdur = 0.0;
	double m_Pow = 0.0;

	m_Tdur = TimeDuration(Vdd);
	
	m_Pow = PowConsumption(Vdd);
	m_Iload = m_Pow / Vdd;

	Iload = m_Iload;
	Tur = m_Tdur;

	return m_Pow * m_Tdur;
}

double loadApplication::PowConsumption(double Vdd){
	double m_ret = 0.0;

	return m_ret = (m_Pa * Vdd * Vdd * Vdd + m_Pb * Vdd * Vdd + m_Pc * Vdd) / 100;
}

double loadApplication::TimeDuration(double Vdd){
	double m_ret = 0.0;

	return m_ret = (m_Ta * Vdd + m_Tb) * 1000;
}

static task_struct idle_task(0.0, 0.0, 0.0);

void loadApplication::InitilizeTaskQueue() {
	/* Read tasks from a file */
	double vdd(0.0), idd(0.0), exec_time(0.0);
	ifstream infile("Tasks.txt");
	if (!infile) {
		cerr << "Can not find task file! Abort!" << endl;
	}

	while ((infile >> vdd >> idd >> exec_time).good()) {
		task_struct t1(vdd, idd, exec_time);
		task_queue.push_back(t1);
	}

	infile.close();

	if (!task_queue.empty())
		current_task = &task_queue.front();
	else
		current_task = &idle_task;

	return;
}

void loadApplication::AdvanceLoadProgress(double t) {
	current_task->AdvanceTaskProgress(t);
	return;
}

void loadApplication::ContextSwitch() {

	if ((!task_queue.empty()) && (current_task->GetRemainingTime() <= time_near_zero)) {
		task_queue.pop_front();
		if (!task_queue.empty())
			current_task = &task_queue.front();
		else
			current_task = &idle_task;
	}
}
