#ifndef _SCHEDULER_HPP_
#define _SCHEDULER_HPP_

#include <vector>

class TaskSet {
	private:
		int m_numOfTask;
		int m_numOfVolt; 
		double m_deadline;
		std::vector<double> m_inputDuration;
		std::vector<double> m_inputEnergy;
	public:
};

int generateSchedule();
void randomTaskSetGenerator(int &m_numOfTask, int &m_numOfVolt, double &m_deadline, std::vector<double> &m_inputDuration, std::vector<double> &m_inputEnergy);
void highPowerLongTask(std::vector<double> &m_taskDuration, std::vector<double> &m_taskEnergy, std::vector<double> &m_taskPower);

#endif
