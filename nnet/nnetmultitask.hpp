#ifndef _NNT_MULTI_TASK_
#define _NNT_MULTI_TASK_

#include <tr1/functional>
#include <vector>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include "nnet.hpp"
#include "../selModel/testFilesystem.hpp"

class nnetmultitask {
	public:
		nnetmultitask();
		double predictWithEnergyLength(double inputPower, double enrg, double len);
	private:
		std::vector<int> interalVec;
		std::map<double, Level2Map> nnetMap;

		boost::numeric::ublas::vector<double> nnetInput;
		double currentEnergy;

		std::tr1::function<double(boost::numeric::ublas::vector<double>&)> computeEnergy;
		void bindCalculator(int, double, double);
};

#endif
