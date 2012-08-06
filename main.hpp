#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include <tr1/functional>

#define MAX_TIME_INDEX max_simu_steps

const double min_time_interval = 0.1;

const int recompute_vcti_time_index = 10;

const double time_near_zero = 1e-4;

const double near_zero = 1e-4;

int hees_parse_command_line(int argc, char *argv[]); 

// Declare the global variables
extern int supcap_parallel_conf;
extern int supcap_serial_conf;
extern int battery_parallel_conf;
extern int battery_serial_conf;

extern double supcap_init_charge;
extern double battery_init_charge;

extern int bank_reconfig_enable;

extern double fixed_vcti;

extern int start_time_hh;
extern int start_time_mm;
extern int start_time_ss;

extern int max_simu_steps;
extern int delta_energy_steps;

extern std::tr1::function<double(double)> power_source_func;

extern double dc_super_cap_energy;
extern double bank_res_energy;

#endif
