// File: nnet.cpp

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "nnet.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;

using boost::numeric::ublas::vector;
using boost::numeric::ublas::matrix;

nnetmodel::nnetmodel(): 
		output_min(0), output_range(0),
		b2(0), energy_offset(0.0) {}

void 
nnetmodel::readnnetmodel(const char *name) {
	
    using namespace boost::numeric::ublas;

	ifstream infile(name);
	if (!infile) {
		cerr << "Can not open nnet model file!\n" << endl;
		exit(1);
	}
	string varname;
	// Read number of neurons
	infile >> varname;
	infile >> NUM_OF_NEURONS;

	// Read the input dimension and output dimension
	infile >> varname;
	infile >> INPUT_DIM;
	infile >> varname;
	infile >> OUTPUT_DIM;

	// Read the input min and input range for normalization
	input_min = vector<double> (INPUT_DIM);
	input_range = vector<double> (INPUT_DIM);
	infile >> varname;
	readvectorfromfile(infile, input_min, INPUT_DIM);
#ifdef DEBUG_NNET
	cout << "input_min is: " << endl;
	cout << input_min << endl;
#endif
	infile >> varname;
	readvectorfromfile(infile, input_range, INPUT_DIM);
#ifdef DEBUG_NNET
	cout << "input_range is: " << endl;
	cout << input_range << endl;
#endif
	
	// Read the output min and range for normalization
	infile >> varname;
	infile >> output_min;
#ifdef DEBUG_NNET
	cout << "output_min is: " << endl;
	cout << output_min << endl;
#endif
	infile >> varname;
	infile >> output_range;
#ifdef DEBUG_NNET
	cout << "output_range is: " << endl;
	cout << output_range << endl;
#endif

	// Read the IW
	IW = matrix<double> (NUM_OF_NEURONS, INPUT_DIM);
	infile >> varname;
	readmatrixfromfile(infile, IW, NUM_OF_NEURONS, INPUT_DIM);
#ifdef DEBUG_NNET
	cout << "IW is: " << endl;
	cout << IW << endl;
#endif

	// Read the b1
	b1 = vector<double> (NUM_OF_NEURONS);
	infile >> varname;
	readvectorfromfile(infile, b1, NUM_OF_NEURONS);
#ifdef DEBUG_NNET
	cout << "b1 is: " << endl;
	cout << b1 << endl;
#endif

	// Read the LW
	LW = vector<double> (NUM_OF_NEURONS);
	infile >> varname;
	readvectorfromfile(infile, LW, NUM_OF_NEURONS);
#ifdef DEBUG_NNET
	cout << "LW is: " << endl;
	cout << LW << endl;
#endif

	// Read the b2
	infile >> varname;
	infile >> b2;
#ifdef DEBUG_NNET
	cout << "b2 is: " << endl;
	cout << b2 << endl;
#endif

	infile.close();
	infile.clear();

	input = vector<double> (INPUT_DIM);
	inter_output = vector<double> (NUM_OF_NEURONS);

	return;
}

double 
nnetmodel::simnnet(vector<double> &input) {
	
	// Set the result to initialEnergy
	double result = 0.0;
	double inputPower  = input[0];
	if (energy_offset > 0.0)
		input(1) -= energy_offset;
	double initEnergy = input(1);

	// Preprocessing the input
	// Down scale
	preprocessing(input, input_min, input_range, INPUT_DIM);

	// We return earlier when the input power
	// is less than the min training power
	// input[0]: preprocessed power
	// input[1]: preprocessed initial energy
	if (input(0) < -1.0) {
#ifdef DEBUG_NNET
		printf("We found the input power is less than the training minimum:\n");
		cout << input << endl;
#endif
		// Added the energy_offset back
		return initEnergy+energy_offset;
	}

#ifdef DEBUG_NNET
	printf("Scaled input: toolbox/nnet/nnutils/+nnsim/y.m:29 Pc\n");
	cout << input << endl;
#endif

	// The first level multiplication
	// IW * input
	inter_output = prod(IW, input);

#ifdef DEBUG_NNET
	printf("IW * input:\n");
	cout << inter_output << endl;
#endif

	// First level bias
	inter_output += b1;

#ifdef DEBUG_NNET
	printf("Added first level bias: toolbox/nnet/nnutils/+nnsim/a.m:114 n\n");
	cout << inter_output << endl;
#endif

	// Transfer function: tansig
	tansig(inter_output, NUM_OF_NEURONS);

#ifdef DEBUG_NNET
	printf("After tansig function: toolbox/nnet/nnutils/+nnsim/a.m:120 Ac\n");
	cout << inter_output << endl;
#endif

	// The second level multiplication
	// LW * first level output
	// Vector to vector multiplication
	for (int i = 0; i < NUM_OF_NEURONS; ++i) {
		result += inter_output[i] * LW[i];
	}

	// Second level bias
	result += b2;

#ifdef DEBUG_NNET
	printf("After second layer: toolbox/nnet/nnutils/+nnsim/a.m:120 Ac\n");
	cout << result << endl;
#endif

	// Postprocessing the output
	// Up scale
	result = postprocessing(result, output_min, output_range);

#ifdef DEBUG_NNET
	printf("After post scale: toolbox/nnet/nnutils/+nnsim/y.m:49 Y and \n");
	cout << result << endl;
#endif

	// FIXME: if the prediction is out of wack
	// Then we have to assume something wrong
	// with the predictor
	if ((result > initEnergy + inputPower*10.0) || (result < initEnergy)){
		result = initEnergy;
	}

	// Added energy_offset back
	result += energy_offset;

	return result;
}

void readmatrixfromfile(ifstream &infile, matrix<double> &matrix, int row, int col) {
		if (!infile) {
				cerr << "The matrix file is not valid! EXIT!" << endl;
				exit(1);
		}

		int i = 0, j = 0;
		for (i = 0; i < row; ++i) {
				for (j = 0; j < col; ++j) {
						infile >> matrix(i, j);
				}
		}

		return;
}

void readvectorfromfile(ifstream &infile, vector<double> &vector, int col) {
		if (!infile) {
				cerr << "The matrix file is not valid! EXIT!" << endl;
				exit(1);
		}

		int i = 0;
		for (i = 0; i < col; ++i) {
				infile >> vector(i);
		}

		return;
}

void tansig(vector<double> &x1, int dimension) {
		int i = 0;
		for (i = 0; i < dimension; ++i) {
				x1(i) = 2.0 / (1 + exp(-2*x1(i))) - 1;
		}

		return;
}

void preprocessing(vector<double> &input, vector<double> &min, vector<double> &range, int dimension) {

		int i = 0;
		double yrange = 2.0, ymin = -1.0;
		for (i = 0; i < dimension; ++i) {
				input(i) = yrange * (input(i) - min(i)) / range(i) + ymin;
		}

		return;
}

double postprocessing(double d, double min, double range) {

		double out = 0.0;
		double yrange = 2.0, ymin = -1.0;
		out = range * (d - ymin) / yrange + min;

		return out;
}

#ifdef NNET_BINARY
/* Testbench */

int readInput(double (*data)[4], const char *filename) {

	ifstream inputFile;
	double power(0.0), initialEnergy(0.0), duration(0.0), endEnergy(0.0);
	int i = 0;

	inputFile.open(filename);
	if (!inputFile) {
		cerr << "Can not open nnet model file!\n" << endl;
		exit(1);
	}

	while ((inputFile >> power >> initialEnergy >> duration >> endEnergy).good()) {
		data[i][0] = power;
		data[i][1] = initialEnergy;
		data[i][2] = duration;
		data[i][3] = endEnergy;
		++i;
	}
	
	inputFile.close();
	return i;
}

int main() {

    using namespace boost::numeric::ublas;

	struct nnetmodel nnet;
	vector<double> input(2);
	input[0] = 0.438;
	input[1] = 239.229;

	nnet.readnnetmodel("data/PredictionModel/nnetmodel0100");
	nnet.energy_offset = 0.5 * pow(100, 2) / 40.0; 

	double res = nnet.simnnet(input);
	cout << "Predicted energy is: " << res << endl;

	return 0;
}
#endif

