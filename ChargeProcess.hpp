#ifndef _CHARGE_PROCESS_HPP_
#define _CHARGE_PROCESS_HPP_

class supcapacitor;
class lionbat;
class loadApplication;
class ees_bank;

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <tr1/functional>

#include "DCCon_in.hpp"
#include "DCCon_out.hpp"
#include "selVCTI.hpp"

enum POWER_STATUS {POWER_INIT, POWER_NORMAL, POWER_NOT_ENOUGH_FOR_LOAD};

class ChargeProcess {
public:
	ChargeProcess();
	int ChargeProcessOurPolicy(ees_bank *bank, lionbat *lb, loadApplication *load);
	int ChargeProcessOptimalVcti(ees_bank *bank, lionbat *lb, loadApplication *load); 
	void print_super_cap_info(std::ofstream &output_file, ees_bank *bank, double power_input);
// Private member function
private:
	std::tr1::function<void(ees_bank *)> charge_policy;
	void compute_dc_bank_iin();
	void charge_policy_our_policy(ees_bank *bank);
	void charge_policy_optimal_vcti(ees_bank *bank); 
	int ChargeProcessApplyPolicy(ees_bank *bank, lionbat *lb, loadApplication *load); 
// Private data member
private:
	// CTI
	double vcti, icti;

	// Super capacitor discharging paramenters
	double super_cap_iin, super_cap_qacc, super_cap_voc, super_cap_vcc, dc_load_power;

	// DC-DC converter for the super capacitor bank discharge
	double &dc_super_cap_vout, &dc_super_cap_iout, &dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_power;

	// DC-DC converter for the load
	double &dc_load_vin, &dc_load_iin, dc_load_vout, dc_load_iout;

	// Power related parameters
	double power_input; // The power input to the system
	POWER_STATUS power_status; // The power status of the system

	// Task info and timer stuff
	double time_elapsed;
	int time_index;

	// dc-dc converters
	dcconvertIN dc_load;
	dcconvertOUT dc_super_cap;
	selVcti sel_vcti;

	// Policy related
	bool supcap_reconfig_return;

	// Output file
	std::string output_file;

};

#endif
