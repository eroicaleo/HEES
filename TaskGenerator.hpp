#ifndef _TASK_GENERATOR_HPP_
#define _TASK_GENERATOR_HPP_

#include <vector>

#include "VoltageTable.hpp"

using std::vector;

double randomTaskSetGenerator(vector<double> &InDuration, vector<double> &InEnergy, vector<TaskVoltageTable> &vec_tvt, const VoltageTable &vt);

#endif
