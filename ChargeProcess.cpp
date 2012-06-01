#include <fstream>
#include <iostream>
#include <math.h>

#include "ChargeProcess.hpp"
#include "DCCon_in.hpp"
#include "DCCon_out.hpp"
#include "LoadApp.hpp"
#include "SuperCap.hpp"
#include "selVCTI.hpp"
#include "main.hpp"

using namespace std;

ChargeProcess::ChargeProcess() :
	vcti(0.0), icti(0.0),
	super_cap_iin(0.0), super_cap_qacc(0.0), super_cap_voc(0.0), super_cap_vcc(0.0), dc_load_power(0.0),
	dc_super_cap_vout(super_cap_vcc), dc_super_cap_iout(super_cap_iin), dc_super_cap_vin(vcti), dc_super_cap_iin(0.0), dc_super_cap_power(0.0),
	dc_load_vin(vcti), dc_load_iin(icti), dc_load_vout(1.0), dc_load_iout(1.0), 
	output_file("OverallProcess.txt") {
	
	ofstream output(output_file.c_str());
	output << "power_intput\tVCTI\tVcap_oc\tVcap_cc\tQacc\tIsup\tPdcsup\tPdcload\tPrbank\tEsup\tCacc" << endl;
	output.close();
}

int ChargeProcess::ChargeProcessOurPolicy(double power_input, ees_bank *bank, lionbat *lb, loadApplication *load) {
	
	dcconvertIN dc_load;
	dcconvertOUT dc_super_cap;

	// DC-DC converter for the load
	dc_load_vout = load->get_vdd();
	dc_load_iout = load->get_idd();
	double current_task_remaining_time = load->get_exec_time();

	// Task info and timer stuff
	double time_elapsed = 0.0;
	int time_index = 0;

	// Output file
	ofstream output(output_file.c_str(), ios_base::app);
	if (!output.good()) {
		cerr << "Can not open files!" << endl;
	}

	bool supcap_reconfig_return = true;

	// Set the VCTI to the load vdd;
	vcti = dc_load_vout;
	// Compute the current Icti on CTI from bank to load
	dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

	dc_super_cap_iin = power_input/vcti - dc_load_iin;

	// Test if the supply power is large enough
	if (dc_super_cap_iin < 0) {
		return -1;
	}

	while (current_task_remaining_time > min_time_interval) {
		
		dc_super_cap.ConverterModel_EESBank(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, bank);
		// Reconfig if necessary
		while ((dc_super_cap_vout > dc_super_cap_vin) && supcap_reconfig_return) {
			supcap_reconfig_return = bank->EESBankOperating(dc_super_cap_iin, dc_super_cap_vout, -100.0);
			dc_super_cap.ConverterModel_EESBank(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, bank);
		}

		// Recorde the curren status of the super capacitor
		print_super_cap_info(output, bank, power_input);

		bank->EESBankCharge(super_cap_iin, min_time_interval, super_cap_vcc, super_cap_qacc);

		current_task_remaining_time -= min_time_interval;
		time_elapsed += min_time_interval;
		++time_index;

		if (time_index > MAX_TIME_INDEX)
			break;
	}

	output.close();
	return time_index;
}

int ChargeProcess::ChargeProcessOptimalVcti(double power_input, ees_bank *bank, lionbat *lb, loadApplication *load) {
	
	dcconvertIN dc_load;
	dcconvertOUT dc_super_cap;

	// DC-DC converter for the load
	dc_load_vout = load->get_vdd();
	dc_load_iout = load->get_idd();
	double current_task_remaining_time = load->get_exec_time();

	// Task info and timer stuff
	double time_elapsed = 0.0;
	int time_index = 0;

	// Output file
	ofstream output(output_file.c_str(), ios_base::app);

	// Set the VCTI to the load vdd;
	vcti = dc_load_vout;

	selVcti sel_vcti;

	while (current_task_remaining_time > min_time_interval) {

		if (time_index % recompute_vcti_time_index == 0) {
			vcti = sel_vcti.bestVCTI(power_input, dc_load_iout, dc_load_vout, "out_SupCap", lb, bank);
		}

		// Compute the current Icti on CTI from bank to load
		dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

		dc_super_cap_iin = power_input/vcti - dc_load_iin;

		// Test if the supply power is large enough
		if (dc_super_cap_iin < 0) {
			return -1;
		}
		
		dc_super_cap.ConverterModel_EESBank(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, bank);

		// Recorde the curren status of the super capacitor
		print_super_cap_info(output, bank, power_input);

		bank->EESBankCharge(super_cap_iin, min_time_interval, super_cap_vcc, super_cap_qacc);

		current_task_remaining_time -= min_time_interval;
		time_elapsed += min_time_interval;
		++time_index;

		if (time_index > MAX_TIME_INDEX)
			break;
	}

	output.close();
	return time_index;
}

void ChargeProcess::print_super_cap_info(ofstream &output, ees_bank *bank, double power_input) {
	// Output the status to a file
	super_cap_voc = bank->EESBankGetVoc();
	super_cap_qacc = bank->EESBankGetQacc();
	output << power_input << "\t"
			<< vcti << "\t"
			<< super_cap_voc << "\t"
			<< super_cap_vcc << "\t"
			<< super_cap_qacc << "\t"
			<< super_cap_iin << "\t"
			<< dc_super_cap_power << "\t"
			<< dc_load_power << "\t"
			<< super_cap_iin*(bank->EESBankGetRacc()*bank->EESBankGetRacc()) << "\t"
			<< bank->EESBankGetEnergy() << "\t"
			<< bank->EESBankGetCacc() << endl;

	return;
}
