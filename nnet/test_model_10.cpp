#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include "nnet.hpp"
#include "nnetmultitask.hpp"

using namespace std;

const double init_energy = 20.0;

const double input_power_arr[] = {3.936, 3.732, 3.522, 3.304, 3.079, 2.847, 2.608, 2.362, 2.108, 1.848, 1.580, 1.305, 1.023, 0.734, 0.438, 0.135};

const vector<double> input_power_vec(input_power_arr, input_power_arr + sizeof(input_power_arr)/sizeof(double));

int main() {

	vector<double> energy_vec(input_power_vec.size(), init_energy);
	const double interval_len(10);
	nnetmultitask nnet_model;
	const size_t start_ix = 0;
	const int num_of_pred = 400;

	ofstream ofile;
	ofile.open("test_model_10.txt");
	ofile.unsetf ( std::ios::floatfield );
	ofile.precision(3);
	ofile.setf( std::ios::fixed, std:: ios::floatfield );

	for (size_t i = start_ix; i < input_power_vec.size(); ++i) {
		ofile << setw(8) << input_power_vec[i] << "    ";
	}
	ofile << endl;
	for (size_t i = start_ix; i < energy_vec.size(); ++i) {
		ofile << setw(8) << energy_vec[i] << "    ";
	}
	ofile << endl;

	for (int i = 0; i < num_of_pred; ++i) {
		for (size_t j = start_ix; j < energy_vec.size(); ++j) {
			energy_vec[j] = nnet_model.predictWithEnergyLength(input_power_vec[j], energy_vec[j], interval_len);
			ofile << setw(8) << energy_vec[j] << "    ";
		}
		ofile << endl;
	}
	ofile.close();
}
