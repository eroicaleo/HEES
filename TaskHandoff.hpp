#ifndef _TASK_HANDOFF_HPP_
#define _TASK_HANDOFF_HPP_

#include <iostream>
#include <vector>
#include <string>

class TaskHandoff {

	private:
		int length;
		double power;
		double energy;

	public:
		TaskHandoff(int l, double p, double e) :
			length(l), power(p), energy(e) {
			}

		friend std::ostream& operator<<(std::ostream &os, const TaskHandoff &t);
};

void genScheduleTaskHandoffSet(const std::vector<TaskHandoff> &taskHandoffSet, std::string fileName);

#endif
