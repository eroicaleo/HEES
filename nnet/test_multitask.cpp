#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "nnet.hpp"
#include "nnetmultitask.hpp"

using namespace std;
using namespace boost::program_options;

int local_parse_command_line(int argc, char *argv[], double &init_energy, double &input_power, int &length);

int main(int argc, char *argv[]) {
	double init_energy(0.0);
	double input_power(0.0);
	int length(10);
	nnetmultitask nnet_model;

	local_parse_command_line(argc, argv, init_energy, input_power, length);

	double final_energy = nnet_model.predictWithEnergyLength(input_power, init_energy, length);
	cout << final_energy << endl;

	return 0;
}

int local_parse_command_line(int argc, char *argv[], double &init_energy, double &input_power, int &length) {

	int ret = 0;
	string config_file;

	try {
		options_description generic("Generic options");
		generic.add_options()
			("help", "produce help message")
			("config,c", value<string>(&config_file)->default_value("test_multitask.cfg"),
			                  "name of a file of a configuration.")
		;

		/* Bank configuration related parameters */
		options_description bank_config_options("test_multitask configuration options");
		bank_config_options.add_options()
			("init_energy", value<double>(&init_energy)->default_value(0.0), "The initial bank energy")
			("input_power", value<double>(&input_power)->default_value(0.0), "The harvesting power")
			("length", value<int>(&length)->default_value(10), "charging time in seconds")
		;

		options_description cmdline_options;
		cmdline_options.add(generic).add(bank_config_options);

		options_description config_file_options;
		config_file_options.add(bank_config_options);

		variables_map vm;
		store(parse_command_line(argc, argv, cmdline_options), vm);
		notify(vm);

		/* Now parse the config file */
		ifstream ifs(config_file.c_str());
		if (!ifs) {
            cout << "can not open config file: " << config_file << "\n";
            exit(0);
        } else {
			store(parse_config_file(ifs, config_file_options), vm);
			notify(vm);
		}

        if (vm.count("help")) {
            cout << "Usage: options_description [options]\n";
            cout << generic;
			exit(0);
        }

	} catch (exception &e) {
		cerr << "Error: " << e.what() << endl;
		return -1;
	} catch (...) {
		cerr << "Unknow exceptions!" << endl;
		return -1;
	}

	return ret;



}
