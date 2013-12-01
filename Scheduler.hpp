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

#endif
