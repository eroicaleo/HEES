
#include "nnetmultitask.hpp"

using namespace std;
using namespace std::tr1;

static char filenames[4][50] = {
"nnetmodel1to5",
"nnetmodel20",
"nnetmodel40",
"nnetmodel100"
};

/* FIXME: when we get more models */
nnetmultitask::nnetmultitask() :
	interalVec(1, 20),
	nnet1to5(),
	nnet20(),
	nnet40(),
	nnet100()
{
	nnet20.readnnetmodel(filenames[1]);
}

double nnetmultitask::predictWithEnergyLength(double inputPower, double startEnergy, double len) {

	nnetInput[0] = inputPower;
	nnetInput[1] = startEnergy;
	nnetInput[2] = len;

	for (size_t i = 0; i < interalVec.size(); ++i) {
		bindCalculator(interalVec[i]);
		while (len > interalVec[i]) {
			startEnergy = computeEnergy(nnetInput);
			len -= interalVec[i];
			nnetInput[0] = inputPower;
			nnetInput[1] = startEnergy;
			nnetInput[2] = len;
		}
	}

	return startEnergy;
}

void nnetmultitask::bindCalculator(int len) {
	switch (len) {
		case 100:
		case 40:
		case 20:
		case 5:
		default :
			computeEnergy = bind(&nnetmodel::simnnet, nnet20, placeholders::_1);
			break;
	}
}
