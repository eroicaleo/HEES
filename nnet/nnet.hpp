// File: nnet.hpp
#ifndef _NNET_HPP_
#define _NNET_HPP_

#include <fstream>

#include <math.h>

using std::ifstream;

/* Read matrix from file */
void readmatrixfromfile(ifstream &infile, double **matrix, int row, int col);

/* Read vector from file */
void readvectorfromfile(ifstream &infile, double *vector, int col);

/* matrix vector multiplication */
/* result = mat * vec */
void mbynmatvecmult(double *result, double **mat, double *vec, int row, int col); 

/* vector addition */
/* x1 = x1 + y1 */
void vecadd(double *x1, double *y1, int dimension); 

/* tansig function */
void tansig(double *x1, int dimension);

/* Down scale the input */
void preprocessing(double *input, double *min, double *range, int dimension);

/* Up scale the output */
double postprocessing(double d, double min, double range);

struct nnetmodel {
	
	/* Default constructor */
	nnetmodel();  

	/* Public interface */
	
	/* Read in all the data from nnet model */
	void readnnetmodel(const char *name);

	/* Simulation the neural networks */
	double simnnet(double *input);

	/* Data member */
	int NUM_OF_NEURONS;
	int INPUT_DIM;
	int OUTPUT_DIM;

	double *input_min;
	double *input_range;
	double output_min;
	double output_range;

	double *inter_output;

	double **IW;
	double *b1;
	double *LW;
	double b2;

	double energy_offset;
	// The 3 dimensions are average, high, low power
	// Other dimensions are neighbor high, lower temperature
	double *input;
};

#endif
