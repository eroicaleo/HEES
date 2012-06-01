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

class ChargeProcess {
public:
	ChargeProcess();
	int ChargeProcessOurPolicy(double power_input, ees_bank *bank, lionbat *lb, loadApplication *load);
	int ChargeProcessOptimalVcti(double power_input, ees_bank *bank, lionbat *lb, loadApplication *load); 
	void print_super_cap_info(std::ofstream &output_file, ees_bank *bank, double power_input);
// Private member function
private:
	std::tr1::function<void(ees_bank *)> charge_policy;
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

	// Output file
	std::string output_file;

};

#endif
