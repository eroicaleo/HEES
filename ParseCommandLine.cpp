#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

using namespace std;
using namespace boost::program_options;

int supcap_parallel_conf;
int supcap_serial_conf;
int battery_parallel_conf;
int battery_serial_conf;

double supcap_init_charge;
double battery_init_charge;

int start_time_hh;
int start_time_mm;
int start_time_ss;

int max_simu_steps;
int delta_energy_steps;

int hees_parse_command_line(int argc, char *argv[]) {
	
	int ret = 0;
	string config_file;

	try {
		options_description generic("Generic options");
		generic.add_options()
			("help", "produce help message")
			("config,c", value<string>(&config_file)->default_value("hees.cfg"),
			                  "name of a file of a configuration.")
		;

		/* Bank configuration related parameters */
		options_description bank_config_options("HEES bank configuration options");
		bank_config_options.add_options()
			("supcap_parallel_conf", value<int>(&supcap_parallel_conf)->default_value(1), "no. of parallel connected supercapacitor cell")
			("supcap_serial_conf", value<int>(&supcap_serial_conf)->default_value(4), "no. of serial connected supercapacitor cell")
			("battery_parallel_conf", value<int>(&battery_parallel_conf)->default_value(1), "no. of parallel connected battery cell")
			("battery_serial_conf", value<int>(&battery_serial_conf)->default_value(4), "no. of serial connected battery cell")
			("supcap_init_charge", value<double>(&supcap_init_charge)->default_value(0.0), "amount of initial charge in supercapacitor bank")
			("battery_init_charge", value<double>(&battery_init_charge)->default_value(0.0), "amount of initial charge in battery bank")
		;

		/* Time related parameters */
		options_description time_config_options("Time configuration options");
		time_config_options.add_options()
			("start_time_hh", value<int>(&start_time_hh)->default_value(11), "System starting time in hour")
			("start_time_mm", value<int>(&start_time_mm)->default_value(00), "System starting time in minute")
			("start_time_ss", value<int>(&start_time_ss)->default_value(00), "System starting time in second")
			("max_simu_steps", value<int>(&max_simu_steps)->default_value(6000), "Max simulation steps, one step = 0.1 sec ")
			("delta_energy_steps", value<int>(&delta_energy_steps)->default_value(1000), "We record the delta energy evergy delta_energy_steps, one step = 0.1 sec")
		;

		options_description cmdline_options;
		cmdline_options.add(generic).add(bank_config_options).add(time_config_options);

		options_description config_file_options;
		config_file_options.add(bank_config_options).add(time_config_options);

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
