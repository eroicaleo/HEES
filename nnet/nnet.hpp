// File: nnet.hpp
#ifndef _NNET_HPP_
#define _NNET_HPP_

#include <fstream>

#include <math.h>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

/* Read matrix from file */
void readmatrixfromfile(std::ifstream &infile, boost::numeric::ublas::matrix<double> &matrix, int row, int col);

/* Read vector from file */
void readvectorfromfile(std::ifstream &infile, boost::numeric::ublas::vector<double> &vector, int col);

/* matrix vector multiplication */
/* result = mat * vec */
void mbynmatvecmult(boost::numeric::ublas::vector<double> &result, boost::numeric::ublas::matrix<double> &mat, boost::numeric::ublas::vector<double> &vec, int row, int col); 

/* tansig function */
void tansig(boost::numeric::ublas::vector<double> &x1, int dimension);

/* Down scale the input */
void preprocessing(boost::numeric::ublas::vector<double> &input, boost::numeric::ublas::vector<double> &min, boost::numeric::ublas::vector<double> &range, int dimension);

/* Up scale the output */
double postprocessing(double d, double min, double range);

struct nnetmodel {
		/* Default constructor */
	nnetmodel();  
 
	/* Public interface */
	
	/* Read in all the data from nnet model */
	void readnnetmodel(const char *name);

	/* Simulation the neural networks */
	double simnnet(boost::numeric::ublas::vector<double> &input);

	/* Data member */
	int NUM_OF_NEURONS;
	int INPUT_DIM;
	int OUTPUT_DIM;

	boost::numeric::ublas::vector<double> input_min;
	boost::numeric::ublas::vector<double> input_range;
	double output_min;
	double output_range;

	boost::numeric::ublas::vector<double> inter_output;

	boost::numeric::ublas::matrix<double> IW;
	boost::numeric::ublas::vector<double> b1;
	boost::numeric::ublas::vector<double> LW;
	double b2;

	double energy_offset;
	// The 3 dimensions are average, high, low power
	// Other dimensions are neighbor high, lower temperature
	boost::numeric::ublas::vector<double> input;

	/* variables for model sanity check */
	double bank_vol;
	double solar_min;
	double solar_max;
};

#endif
