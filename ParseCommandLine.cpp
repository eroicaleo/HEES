#include "ParseCommandLine.hpp"

using namespace std;
using namespace std::tr1;
using namespace boost::program_options;

/* Bank related parameters */
int supcap_parallel_conf;
int supcap_serial_conf;
int battery_parallel_conf;
int battery_serial_conf;

double supcap_init_charge;
double battery_init_charge;

int bank_reconfig_enable;

double supcap_res;
double supcap_cap;

/* VCTI related parameters */
double fixed_vcti;

/* Time related parameters */
int start_time_hh;
int start_time_mm;
int start_time_ss;

int max_simu_steps;
int delta_energy_steps;

/* Power source related parameters */
string power_source_type;
double constant_power_value;
ConstantPowerSource cps;
VariablePowerSource vps;

function<double(double)> power_source_func;

/* Scheduling related parameters */
double ratio_runtime_and_deadline;
double scheduling_deadline;
double min_task_power;
double max_task_power;
int number_of_tasks;

/* Predictor related parameters */
double min_training_power;
double max_training_power;

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
			("bank_reconfig_enable", value<int>(&bank_reconfig_enable)->default_value(1), "If hees bank reconfiguration is enabled or not")
			("fixed_vcti", value<double>(&fixed_vcti)->default_value(0.0), "Set it to > 0, if you want to fix the Vcti")
			("supcap_res", value<double>(&supcap_res)->default_value(10e-3), "The internal resistance of each cell in the supercapacitor bank")
			("supcap_cap", value<double>(&supcap_cap)->default_value(10e-3), "The capacitance of each cell in the supercapacitor bank")
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

		/* Power options */
		options_description power_options("Power source options");
		power_options.add_options()
			("power_source", value<string>(&power_source_type)->default_value("constant_power"), "Use a constant power, must specify the constant_power_value option")
			("constant_power_value", value<double>(&constant_power_value)->default_value(0.0), "The value of the constant power source")
		;

		/* Schedule options */
		options_description schedule_options("Schedule source options");
		schedule_options.add_options()
			("ratio_runtime_and_deadline", value<double>(&ratio_runtime_and_deadline)->default_value(1.0), "The ratio between the runtime @ nominal speed and deadline")
			("scheduling_deadline", value<double>(&scheduling_deadline)->default_value(-1.0), "User defined deadline")
			("min_task_power", value<double>(&min_task_power)->default_value(0.6), "The minimum task power generated randomly")
			("max_task_power", value<double>(&max_task_power)->default_value(1.5), "The maximum task power generated randomly")
			("number_of_tasks", value<int>(&number_of_tasks)->default_value(3), "The number of tasks generated randomly")
		;

		/* Predictor options */
		options_description predictor_options("Neural network predictor options");
		predictor_options.add_options()
			("min_training_power", value<double>(&min_training_power)->default_value(0.0), "The minimum power used in training prediction model")
			("max_training_power", value<double>(&max_training_power)->default_value(5.0), "The maximum power used in training prediction model")
		;

		options_description cmdline_options;
		cmdline_options.add(generic).add(bank_config_options).add(time_config_options).add(power_options).add(schedule_options).add(predictor_options);

		options_description config_file_options;
		config_file_options.add(bank_config_options).add(time_config_options).add(power_options).add(schedule_options).add(predictor_options);

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

		if (power_source_type == "solar_power") {
			power_source_func = solar_power_source_sec;
		} else if ((power_source_type == "constant_power") && (vm.count("constant_power_value"))) {
			cps.SetPowerValue(constant_power_value);
			power_source_func = bind(&ConstantPowerSource::GetConstantPower, &cps, placeholders::_1);
		} else if (power_source_type == "variable_power") {
			vps.ReadVariablePowerSource("VariablePowerSource.txt");
			power_source_func = bind(&VariablePowerSource::AdvanceVariablePowerSource, &vps, placeholders::_1);
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
