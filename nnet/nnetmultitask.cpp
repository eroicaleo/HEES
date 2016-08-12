#include <cmath>
#include <iostream>

#include "nnetmultitask.hpp"

using namespace std;
using namespace std::tr1;

static double cutoff_power = 1.305;

/* FIXME: when we get more models */
nnetmultitask::nnetmultitask() :
	interalVec(1, 10),
	nnetInput(2, 0.0)
{
	interalVec[0] = 10;
	buildNnetMap(nnetMap);
}

double nnetmultitask::predictWithEnergyLength(double inputPower, double startEnergy, double len) {

	/* Preprocessing the length, rounding it to the nearest one */
	int intlen = (int)len;
	nnetInput[0] = inputPower;
	nnetInput[1] = startEnergy;

	for (size_t i = 0; i < interalVec.size(); ++i) {
		try {
			bindCalculator(interalVec[i], inputPower, startEnergy);
		} catch (int e) {
			// cout << "An exception occurred. Exception Nr. " << e << '\n';
			return startEnergy;
		}
		while (intlen >= interalVec[i]) {
			startEnergy = computeEnergy(nnetInput);
			intlen -= interalVec[i];
#ifdef DEBUG_NNET
			double initEnergy = nnetInput[1];
#endif
			nnetInput[0] = inputPower;
			nnetInput[1] = startEnergy;
#ifdef DEBUG_NNET
			cout << "my prediction: " << inputPower << ", " << initEnergy << ", " << interalVec[i] << ", " << startEnergy << endl;
#endif
		}
	}

	// Here the remaining length must be less than interalVec.back()
	nnetInput[0] = inputPower;
	nnetInput[1] = startEnergy;
	// We need to deal with when len < interalVec.back()
	// So we precompute the energy increase as if we charge for the minimum prediction granularity
	// And then we do compensation later
	double energydiff = 0.0;
	int lendiff = 0;
	if (intlen > 0) {
		try {
			bindCalculator(interalVec.back(), inputPower, startEnergy);
		} catch (int e) {
			// cout << "An exception occurred. Exception Nr. " << e << '\n';
			return startEnergy;
		}
		energydiff = computeEnergy(nnetInput) - startEnergy;
		startEnergy += energydiff;

		lendiff = (intlen == 0) ? 0 : intlen - interalVec.back();

		// Here we compute the energy might need compensate
		energydiff = ((double)lendiff / (double)interalVec.back()) * energydiff;
		startEnergy += energydiff;
	}
#ifdef DEBUG_NNET
	cout << "We round the task length by: " << lendiff << " so we compensate energy by: " << energydiff << " and the final energy is: " << startEnergy << endl;
#endif

	return startEnergy;
}

void nnetmultitask::bindCalculator(int len, double inputPower, double energy) {
	double capacitance = 40.0;
	double charge = sqrt(2.0 * energy * capacitance);
	nnetmodel m(recallNnetMap(nnetMap, charge, inputPower));
	computeEnergy = bind(&nnetmodel::simnnet, m, placeholders::_1);
	return;

	/*
	if (energy >= 24500.0) {
		computeEnergy = bind(&nnetmodel::simnnet, nnet1400, placeholders::_1);
		return;
	} else if (energy >= 21125.0) {
		computeEnergy = bind(&nnetmodel::simnnet, nnet1300, placeholders::_1);
		return;
	} else if (energy >= 4500.0) {
		computeEnergy = bind(&nnetmodel::simnnet, nnet0600, placeholders::_1);
		return;
	}
	switch (len) {
		case 100:
			computeEnergy = bind(&nnetmodel::simnnet, nnet100, placeholders::_1);
			break;
		case 40:
			computeEnergy = bind(&nnetmodel::simnnet, nnet40, placeholders::_1);
			break;
		case 20:
			computeEnergy = bind(&nnetmodel::simnnet, nnet20, placeholders::_1);
			break;
		case 10:
			if (inputPower < cutoff_power) {
				computeEnergy = bind(&nnetmodel::simnnet, nnet10bot, placeholders::_1);
			} else {
				computeEnergy = bind(&nnetmodel::simnnet, nnet10, placeholders::_1);
			}
			break;
		case 5:
		default :
			computeEnergy = bind(&nnetmodel::simnnet, nnet20, placeholders::_1);
			break;
	} */
}
