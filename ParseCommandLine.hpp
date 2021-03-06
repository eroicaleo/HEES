#ifndef __PARSE_COMMAND_LINE__
#define __PARSE_COMMAND_LINE__

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <tr1/functional>

#include <boost/program_options.hpp>

#include "powersource.hpp"
#include "VoltageTable.hpp"

using std::string;
using std::tr1::function;

/* Bank related parameters */
extern int supcap_parallel_conf;
extern int supcap_serial_conf;
extern int battery_parallel_conf;
extern int battery_serial_conf;

extern double supcap_init_charge;
extern double battery_init_charge;

extern int bank_reconfig_enable;

extern double supcap_res;
extern double supcap_cap;

/* VCTI related parameters */
extern double fixed_vcti;

/* Time related parameters */
extern int start_time_hh;
extern int start_time_mm;
extern int start_time_ss;

extern int max_simu_steps;
extern int delta_energy_steps;

/* Power source related parameters */
extern string power_source_type;
extern double constant_power_value;
extern ConstantPowerSource cps;

extern function<double(double)> power_source_func;

/* DC-DC converter related parameters */
extern unsigned int dc_load_is_ideal;

/* Scheduling related parameters */
extern double ratio_runtime_and_deadline;
extern double scheduling_deadline;
extern double min_task_power;
extern double max_task_power;
extern int min_task_len;
extern int max_task_len;
extern int number_of_tasks;

/* Predictor related parameters */
extern double min_training_power;
extern double max_training_power;

/* Processor DVFS options */
extern int cpu_dvfs_type;
extern const VoltageTable *cpu_voltage_table_ptr;

/* Functions */
int hees_parse_command_line(int argc, char *argv[]);

inline bool above_min_valid_input_power(double p) {
	return (p >= min_training_power);
}

inline bool below_max_valid_input_power(double p) {
	return (p <= max_training_power);
}

inline bool is_valid_input_power(double p) {
	return (above_min_valid_input_power(p) && below_max_valid_input_power(p));
}

#endif
