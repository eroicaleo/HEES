#ifndef _NNT_MULTI_TASK_
#define _NNT_MULTI_TASK_

#include <tr1/functional>
#include <vector>

#include "nnet.hpp"

class nnetmultitask {
	public:
		nnetmultitask();
		double predictWithEnergyLength(double inputPower, double enrg, double len);
	private:
		std::vector<int> interalVec;
		nnetmodel nnet1to5;
		nnetmodel nnet10;
		nnetmodel nnet20;
		nnetmodel nnet40;
		nnetmodel nnet100;
		nnetmodel nnet10bot;
		double nnetInput[3];
		double currentEnergy;

		std::tr1::function<double(double *)> computeEnergy;
		void bindCalculator(int, double, double);
};

#endif
