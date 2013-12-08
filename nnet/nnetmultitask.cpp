#include <iostream>

#include "nnetmultitask.hpp"

using namespace std;
using namespace std::tr1;

static char filenames[5][50] = {
"nnetmodel1to5",
"nnetmodel10",
"nnetmodel20",
"nnetmodel40",
"nnetmodel100"
};

/* FIXME: when we get more models */
nnetmultitask::nnetmultitask() :
	interalVec(2, 10),
	nnet1to5(),
	nnet10(),
	nnet20(),
	nnet40(),
	nnet100()
{
	interalVec[0] = 20;
	nnet10.readnnetmodel(filenames[1]);
	nnet20.readnnetmodel(filenames[2]);
	nnet40.readnnetmodel(filenames[3]);
	nnet100.readnnetmodel(filenames[4]);
}

double nnetmultitask::predictWithEnergyLength(double inputPower, double startEnergy, double len) {

	/* Preprocessing the length, rounding it to the nearest one */
	int intlen = (int)len;
	int lendiff = intlen;
	if (intlen % 10 < 5) {
		len = (intlen / 10) * 10;
	} else {
		len = (intlen / 10 + 1) * 10;
	}
	lendiff -= (int)len;

	nnetInput[0] = inputPower;
	nnetInput[1] = startEnergy;
	nnetInput[2] = intlen;

	double energydiff = 0.0;
	for (size_t i = 0; i < interalVec.size(); ++i) {
		bindCalculator(interalVec[i]);
		while (len >= interalVec[i]) {
			double initEnergy = nnetInput[1];
			startEnergy = computeEnergy(nnetInput);
			len -= interalVec[i];
			nnetInput[0] = inputPower;
			nnetInput[1] = startEnergy;
			nnetInput[2] = len;
			energydiff = startEnergy - initEnergy;
			cout << "my prediction: " << inputPower << ", " << initEnergy << ", " << interalVec[i] << ", " << startEnergy << endl;
		}
	}
	// Here we compute the energy might need compensate
	energydiff = ((double)lendiff / (double)interalVec.back()) * energydiff;
	startEnergy += energydiff;
	cout << "We round the task length by: " << lendiff << " so we compensate energy by: " << energydiff << " and the final energy is: " << startEnergy << endl;

	return startEnergy;
}

void nnetmultitask::bindCalculator(int len) {
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
			computeEnergy = bind(&nnetmodel::simnnet, nnet10, placeholders::_1);
			break;
		case 5:
		default :
			computeEnergy = bind(&nnetmodel::simnnet, nnet20, placeholders::_1);
			break;
	}
}
