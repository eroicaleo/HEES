// File: nnet.cpp

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "util.h"
#include "nnet.hpp"

using namespace std;

nnetmodel::nnetmodel(): 
		input_min(NULL), input_range(NULL),
		output_min(0), output_range(0),
		inter_output(NULL),
		IW(NULL), b1(NULL),
		LW(NULL), b2(0), input(NULL) {}

void 
nnetmodel::readnnetmodel(const char *name) {
	
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
	input_min = dvector(INPUT_DIM);
	input_range = dvector(INPUT_DIM);
	infile >> varname;
	readvectorfromfile(infile, input_min, INPUT_DIM);
	infile >> varname;
	readvectorfromfile(infile, input_range, INPUT_DIM);

	// Read the output min and range for normalization
	infile >> varname;
	infile >> output_min;
	infile >> varname;
	infile >> output_range;

	// Read the IW
	IW = dmatrix(NUM_OF_NEURONS, INPUT_DIM);
	infile >> varname;
	readmatrixfromfile(infile, IW, NUM_OF_NEURONS, INPUT_DIM);

	// Read the b1
	b1 = dvector(NUM_OF_NEURONS);
	infile >> varname;
	readvectorfromfile(infile, b1, NUM_OF_NEURONS);

	// Read the LW
	LW = dvector(NUM_OF_NEURONS);
	infile >> varname;
	readvectorfromfile(infile, LW, NUM_OF_NEURONS);

	// Read the b2
	infile >> varname;
	infile >> b2;

	infile.close();
	infile.clear();

	input = dvector(INPUT_DIM);
	inter_output = dvector(NUM_OF_NEURONS);

	return;
}

double 
nnetmodel::simnnet(double *input) {
	
	double result = 0.0;

	// Preprocessing the input
	// Down scale
	preprocessing(input, input_min, input_range, INPUT_DIM);

	// The first level multiplication
	// IW * input
	mbynmatvecmult(inter_output, IW, input, NUM_OF_NEURONS, INPUT_DIM);

	// First level bias
	vecadd(inter_output, b1, NUM_OF_NEURONS);

	// Transfer function: tansig
	tansig(inter_output, NUM_OF_NEURONS);

	// The second level multiplication
	// LW * first level output
	// Vector to vector multiplication
	for (int i = 0; i < NUM_OF_NEURONS; ++i) {
		result += inter_output[i] * LW[i];
	}

	// Second level bias
	result += b2;

	// Postprocessing the output
	// Up scale
	result = postprocessing(result, output_min, output_range);

	return result;
}

void readmatrixfromfile(ifstream &infile, double **matrix, int row, int col) {
		if (!infile) {
				cerr << "The matrix file is not valid! EXIT!" << endl;
				exit(1);
		}

		int i = 0, j = 0;
		for (i = 0; i < row; ++i) {
				for (j = 0; j < col; ++j) {
						infile >> matrix[i][j];
				}
		}

		return;
}

void readvectorfromfile(ifstream &infile, double *vector, int col) {
		if (!infile) {
				cerr << "The matrix file is not valid! EXIT!" << endl;
				exit(1);
		}

		int i = 0;
		for (i = 0; i < col; ++i) {
				infile >> vector[i];
		}

		return;
}

void vecadd(double *x1, double *y1, int dimension) {

		int i = 0;
		for (i = 0; i < dimension; ++i) {
				x1[i] = x1[i] + y1[i];
		}

		return; 
}

void mbynmatvecmult(double *result, double **mat, double *vec, int row, int col) {
		if ((result == NULL) || (mat == NULL) || (vec == NULL)) {
				cerr << "The matrix multiplication failed!" << endl;
		}

		int i = 0, j = 0;
		for (i = 0; i < row; ++i) {
				result[i] = 0.0;
				for (j = 0; j < col; ++j) {
						result[i] += mat[i][j] * vec[j];
				}
		}

		return;
}

void tansig(double *x1, int dimension) {
		int i = 0;
		for (i = 0; i < dimension; ++i) {
				x1[i] = 2.0 / (1 + exp(-2*x1[i])) - 1;
		}

		return;
}

void preprocessing(double *input, double *min, double *range, int dimension) {

		int i = 0;
		double yrange = 2.0, ymin = -1.0;
		for (i = 0; i < dimension; ++i) {
				input[i] = yrange * (input[i] - min[i]) / range[i] + ymin;
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

	struct nnetmodel nnet;
	double inputData[100][4];
	int numInput = 0;
	double result = 0.0;

	nnet.readnnetmodel("nnetmodel20");
	numInput = readInput(inputData, "data20131115_20.txt");
	for (int i = 0; i < numInput; ++i) {
		result = nnet.simnnet(inputData[i]);
		cout << result << endl;
	}
	return 0;
}
#endif

