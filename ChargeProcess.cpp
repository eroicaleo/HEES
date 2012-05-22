#include <fstream>
#include <iostream>
#include <math.h>

#include "ChargeProcess.hpp"
#include "DCCon_in.hpp"
#include "DCCon_out.hpp"
#include "SuperCap.hpp"
#include "selVCTI.hpp"
#include "main.hpp"

using namespace std;

ChargeProcess::ChargeProcess() :
	vcti(0.0), icti(0.0),
	super_cap_iin(0.0), super_cap_qacc(0.0), super_cap_voc(0.0), super_cap_vcc(0.0), dc_load_power(0.0),
	dc_super_cap_vout(super_cap_vcc), dc_super_cap_iout(super_cap_iin), dc_super_cap_vin(vcti), dc_super_cap_iin(0.0), dc_super_cap_power(0.0),
	dc_load_vin(vcti), dc_load_iin(icti), dc_load_vout(1.0), dc_load_iout(1.0) {}

void ChargeProcess::ChargeProcessOurPolicy(double power_input, supcapacitor *sp, lionbat *lb, loadApplication *load) {
	
	dcconvertIN dc_load;
	dcconvertOUT dc_super_cap;

	// DC-DC converter for the load
	// FIXME: should initilized from load
	dc_load_vout = 1.0; 
	dc_load_iout = 1.0;

	// Task info and timer stuff
	double task_duration = 2000000.0; // FIXME: should be replaced with load->TimeDuration(Vdd)
	double time_elapsed = 0.0;
	int time_index = 0;
	double current_task_remaining_time = task_duration;

	// Output file
	ofstream output("Charge.txt");
	output << "VCTI\tVcap_oc\tVcap_cc\tQacc\tIsup\tPdcsup\tPdcload\tPrbank\tEsup" << endl;

	bool supcap_reconfig_return = true;

	// Set the VCTI to the load vdd;
	vcti = dc_load_vout;
	// Compute the current Icti on CTI from bank to load
	dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

	dc_super_cap_iin = power_input/vcti - dc_load_iin;

	while (current_task_remaining_time > 0) {
		
		dc_super_cap.ConverterModel_SupCap(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, sp);
		// Reconfig if necessary
		while ((dc_super_cap_vout > dc_super_cap_vin) && supcap_reconfig_return) {
			supcap_reconfig_return = sp->SupCapOperating(dc_super_cap_iin, dc_super_cap_vout, -100.0);
			dc_super_cap.ConverterModel_SupCap(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, sp);
		}

		// Recorde the curren status of the super capacitor
		print_super_cap_info(output, sp);

		sp->SupCapCharge(super_cap_iin, min_time_interval, super_cap_vcc, super_cap_qacc);

		current_task_remaining_time -= min_time_interval;
		time_elapsed += min_time_interval;
		++time_index;

		if (time_index > MAX_TIME_INDEX)
			break;
	}

	output.close();
	return;
}

void ChargeProcess::ChargeProcessOptimalVcti(double power_input, supcapacitor *sp, lionbat *lb, loadApplication *load) {
	
	dcconvertIN dc_load;
	dcconvertOUT dc_super_cap;

	// DC-DC converter for the load
	// FIXME: should initilized from load
	dc_load_vout = 1.0; 
	dc_load_iout = 1.0;

	// Task info and timer stuff
	double task_duration = 2000000.0; // FIXME: should be replaced with load->TimeDuration(Vdd)
	double time_elapsed = 0.0;
	int time_index = 0;
	double current_task_remaining_time = task_duration;

	// Output file
	ofstream output("Charge.txt");
	output << "VCTI\tVcap_oc\tVcap_cc\tQacc\tIsup\tPdcsup\tPdcload\tPrbank\tEsup\tCacc" << endl;

	// Set the VCTI to the load vdd;
	vcti = dc_load_vout;

	selVcti sel_vcti;

	while (current_task_remaining_time > 0) {

		if (time_index % recompute_vcti_time_index == 0) {
			vcti = sel_vcti.bestVCTI(power_input, dc_load_iout, dc_load_vout, "out_SupCap", lb, sp);
		}

		// Compute the current Icti on CTI from bank to load
		dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

		dc_super_cap_iin = power_input/vcti - dc_load_iin;
		
		dc_super_cap.ConverterModel_SupCap(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, sp);

		// Recorde the curren status of the super capacitor
		print_super_cap_info(output, sp);

		sp->SupCapCharge(super_cap_iin, min_time_interval, super_cap_vcc, super_cap_qacc);

		current_task_remaining_time -= min_time_interval;
		time_elapsed += min_time_interval;
		++time_index;

		if (time_index > MAX_TIME_INDEX)
			break;
	}

	output.close();
	return;
}
void ChargeProcess::print_super_cap_info(ofstream &output, supcapacitor *sp) {
	// Output the status to a file
	super_cap_voc = sp->SupCapGetVoc();
	super_cap_qacc = sp->SupCapGetQacc();
	output << vcti << "\t"
			<< super_cap_voc << "\t"
			<< super_cap_vcc << "\t"
			<< super_cap_qacc << "\t"
			<< super_cap_iin << "\t"
			<< dc_super_cap_power << "\t"
			<< dc_load_power << "\t"
			<< super_cap_iin*(sp->SupCapGetRacc()*sp->SupCapGetRacc()) << "\t"
			<< sp->SupCapGetEnergy() << "\t"
			<< sp->SupCapGetCacc() << endl;

	return;
}
