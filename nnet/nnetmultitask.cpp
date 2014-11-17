#include <iostream>

#include "nnetmultitask.hpp"

using namespace std;
using namespace std::tr1;

static double cutoff_power = 1.305;

static char filenames[6][50] = {
"nnetmodel1to5",
"nnetmodel10",
"nnetmodel20",
"nnetmodel40",
"nnetmodel100",
"nnetmodel10bot"
};

/* FIXME: when we get more models */
nnetmultitask::nnetmultitask() :
	interalVec(1, 10),
	nnet1to5(),
	nnet10(),
	nnet20(),
	nnet40(),
	nnet100(),
	nnet10bot()
{
	interalVec[0] = 10;
	nnet10.readnnetmodel(filenames[1]);
	nnet20.readnnetmodel(filenames[2]);
	nnet40.readnnetmodel(filenames[3]);
	nnet100.readnnetmodel(filenames[4]);
	nnet10bot.readnnetmodel(filenames[5]);
}

double nnetmultitask::predictWithEnergyLength(double inputPower, double startEnergy, double len) {

	/* Preprocessing the length, rounding it to the nearest one */
	int intlen = (int)len;
	nnetInput[0] = inputPower;
	nnetInput[1] = startEnergy;
	nnetInput[2] = intlen;

	for (size_t i = 0; i < interalVec.size(); ++i) {
		bindCalculator(interalVec[i], inputPower);
		while (intlen >= interalVec[i]) {
			startEnergy = computeEnergy(nnetInput);
			intlen -= interalVec[i];
			nnetInput[0] = inputPower;
			nnetInput[1] = startEnergy;
			nnetInput[2] = intlen;
#ifdef DEBUG_NNET
			double initEnergy = nnetInput[1];
			cout << "my prediction: " << inputPower << ", " << initEnergy << ", " << interalVec[i] << ", " << startEnergy << endl;
#endif
		}
	}

	// Here the remaining length must be less than interalVec.back()
	nnetInput[0] = inputPower;
	nnetInput[1] = startEnergy;
	nnetInput[2] = interalVec.back();
	// We need to deal with when len < interalVec.back()
	// So we precompute the energy increase as if we charge for the minimum prediction granularity
	// And then we do compensation later
	double energydiff = 0.0;
	int lendiff = 0;
	if (intlen > 0) {
		bindCalculator(interalVec.back(), inputPower);
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

void nnetmultitask::bindCalculator(int len, double inputPower) {
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
	}
}
