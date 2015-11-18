// File: nnet.hpp
#ifndef _NNET_HPP_
#define _NNET_HPP_

#include <fstream>

#include <math.h>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

using std::ifstream;
using namespace boost::numeric::ublas;

/* Read matrix from file */
void readmatrixfromfile(ifstream &infile, matrix<double> &matrix, int row, int col);

/* Read vector from file */
void readvectorfromfile(ifstream &infile, vector<double> &vector, int col);

/* matrix vector multiplication */
/* result = mat * vec */
void mbynmatvecmult(vector<double> &result, matrix<double> &mat, vector<double> &vec, int row, int col); 

/* tansig function */
void tansig(vector<double> &x1, int dimension);

/* Down scale the input */
void preprocessing(vector<double> &input, vector<double> &min, vector<double> &range, int dimension);

/* Up scale the output */
double postprocessing(double d, double min, double range);

struct nnetmodel {
		/* Default constructor */
	nnetmodel();  
 
	/* Public interface */
	
	/* Read in all the data from nnet model */
	void readnnetmodel(const char *name);

	/* Simulation the neural networks */
	double simnnet(vector<double> &input);

	/* Data member */
	int NUM_OF_NEURONS;
	int INPUT_DIM;
	int OUTPUT_DIM;

	vector<double> input_min;
	vector<double> input_range;
	double output_min;
	double output_range;

	vector<double> inter_output;

	matrix<double> IW;
	vector<double> b1;
	vector<double> LW;
	double b2;

	double energy_offset;
	// The 3 dimensions are average, high, low power
	// Other dimensions are neighbor high, lower temperature
	vector<double> input;

	/* variables for model sanity check */
	double bank_vol;
	double solar_min;
	double solar_max;
};

#endif
