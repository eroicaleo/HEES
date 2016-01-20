#ifndef _TASK_HANDOFF_HPP_
#define _TASK_HANDOFF_HPP_

#include <iostream>
#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

class TaskHandoff {

	private:
		int length;
		double power;
		double energy;

	public:
		virtual ~TaskHandoff() {}

		TaskHandoff(int l, double p, double e) :
			length(l), power(p), energy(e) {
			}

		virtual void print(std::ostream &os) const;

		int getLength() const {
			return length;
		}

		friend std::ostream &operator<<(std::ostream &os, const TaskHandoff &t);
};

class TaskHandoffHEES : public TaskHandoff {

	private:
		double voltage;
		double current;

	public:
		TaskHandoffHEES(int l, double p, double e, double v, double c) :
			TaskHandoff(l, p, e), voltage(v), current(c) {
			}

		virtual void print(std::ostream &os) const;

		friend std::ostream &operator<<(std::ostream &os, const TaskHandoffHEES &t);
};

typedef boost::shared_ptr<TaskHandoff> TaskHandoffPtr;
typedef boost::shared_ptr<TaskHandoffHEES> TaskHandoffHEESPtr;

void genScheduleTaskHandoffSet(const std::vector<TaskHandoff> &taskHandoffSet, std::string fileName);
void genScheduleTaskHandoffSet(const std::vector<TaskHandoffHEES> &taskHandoffSet, std::string fileName);

#endif
