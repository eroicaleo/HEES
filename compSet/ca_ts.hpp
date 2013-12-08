#ifndef _CA_TS_HPP_
#define _CA_TS_HPP_

#include <vector>

int minEnergySchedule(int m_numOfTask, int m_numOfVolt, double m_deadline, std::vector<double>m_inputDuration, std::vector<double>m_inputEnergy);

int minEnergyScheduleFixed(int m_numOfTask, int m_numOfVolt, double m_deadline, std::vector<double>m_inputDuration, std::vector<double>m_inputEnergy);

#endif
