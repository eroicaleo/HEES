#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#define MAX_TIME_INDEX max_simu_steps

const double min_time_interval = 0.1;

const int recompute_vcti_time_index = 10;

int hees_parse_command_line(int argc, char *argv[]); 

// Declare the global variables
extern int supcap_parallel_conf;
extern int supcap_serial_conf;
extern int battery_parallel_conf;
extern int battery_serial_conf;

extern double supcap_init_charge;
extern double battery_init_charge;

extern int start_time_hh;
extern int start_time_mm;
extern int start_time_ss;

extern int max_simu_steps;
extern int delta_energy_steps;

#endif
