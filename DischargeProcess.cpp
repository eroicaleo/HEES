#include <fstream>
#include <iostream>
#include <math.h>

#include "DischargeProcess.hpp"
#include "DCCon_dis.hpp"
#include "DCCon_in.hpp"
#include "HEESTimer.hpp"
#include "LoadApp.hpp"
#include "SuperCap.hpp"
#include "ees_bank.hpp"
#include "main.hpp"
#include "selVCTI.hpp"

using namespace std;

extern HEESTimer HTimer;

DischargeProcess::DischargeProcess() :
	vcti(0.0), icti(0.0),
	super_cap_iout(0.0), super_cap_qacc(0.0), super_cap_voc(0.0), super_cap_vcc(0.0), dc_load_power(0.0),
	dc_super_cap_vin(super_cap_vcc), dc_super_cap_iin(super_cap_iout), dc_super_cap_vout(vcti), dc_super_cap_iout(0.0), dc_super_cap_power(0.0),
	dc_load_vin(vcti), dc_load_iin(icti), dc_load_vout(1.0), dc_load_iout(1.0),
	output_file("OverallProcess.txt") {

	ofstream output(output_file.c_str());
	output << "power_intput\tVCTI\tVcap_oc\tVcap_cc\tQacc\tIsup\tPdcsup\tPdcload\tPrbank\tEsup\tCacc" << endl;
	output.close();
}

int DischargeProcess::DischargeProcessOurPolicy(double power_input, supcapacitor *sp, lionbat *lb, loadApplication *load) {

	dcconvertDIS dc_super_cap;
	dcconvertIN dc_load;

	// DC-DC converter for the load
	// FIXME: should initilized from load
	dc_load_vout = load->get_vdd();
	dc_load_iout = load->get_idd();

	// Task info and timer stuff
	double time_elapsed = 0.0;
	int time_index = 0;

	// Output file
	ofstream output(output_file.c_str(), ios_base::app);
	if (!output.good()) {
		cerr << "Can not open files!" << endl;
	}

	// Compute the initial Vcti first
	vcti = ComputeBestVCTI(dc_load_vout, dc_load_iout);
	if (vcti < dc_load_vout) {
		cerr << "Unable to operate at optimal Vcti point!" << endl;
		exit(1);
	}
	
	// Compute the current Icti on CTI from bank to load
	dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

	dc_super_cap_iout = dc_load_iin - power_input/vcti;

	// Test if the supply power is small enough
	if (dc_super_cap_iout < 0) {
		return -1;
	}

	bool could_reconfig_flag = true;
	while (load->CurrentTaskRemainingTime() > min_time_interval) {

		// Compute the current status of the Supercapacitor bank
		dc_super_cap.ConverterModel_supcap(dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_power, sp);
		super_cap_voc = sp->SupCapGetVoc();

		// Doing reconfiguration if necessary
		while ((dc_super_cap_vin < dc_super_cap_vout) && (could_reconfig_flag) && (bank_reconfig_enable)) {
			could_reconfig_flag = sp->SupCapMoreSeriesReconfig();
			dc_super_cap.ConverterModel_supcap(dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_power, sp);
		}

		// Recorde the curren status of the super capacitor
		print_super_cap_info(output, sp, power_input);

		// compute the energy consumption
		sp->SupCapCharge(-super_cap_iout, dc_super_cap_vin, min_time_interval, super_cap_vcc, super_cap_qacc);

		// time elapse
		time_elapsed += min_time_interval;
		++time_index;
		HTimer.HEESTimerAdvancdTimerIndex(1, sp);
		load->AdvanceLoadProgress(min_time_interval);

		// stop if there is no energy in the supercapacitor
		if ((sp->SupCapGetEnergy() <= 0) || (HTimer.HEESTimerGetCurrentTimeIndex() > MAX_TIME_INDEX))
			break;
	}

	output.close();

	return time_index;
}

int DischargeProcess::DischargeProcessOptimalVcti(double power_input, supcapacitor *sp, lionbat *lb, loadApplication *load) {

	dcconvertDIS dc_super_cap;
	dcconvertIN dc_load;
	selVcti sel_vcti;

	// DC-DC converter for the load
	// FIXME: should initilized from load
	dc_load_vout = load->get_vdd();
	dc_load_iout = load->get_idd();

	// Task info and timer stuff
	double time_elapsed = 0.0;
	int time_index = 0;

	// Output file
	ofstream output(output_file.c_str(), ios_base::app);
	if (!output.good()) {
		cerr << "Can not open files!" << endl;
	}

	vcti = dc_load_vout;

	while (load->CurrentTaskRemainingTime() > min_time_interval) {

		if (time_index % 10 == 0) {
			vcti = sel_vcti.bestVCTI(0.0, dc_load_iout, dc_load_vout, "dis_SupCap", sp);
			// Compute the current Icti on CTI from bank to load
			dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
		}

		// Compute the current Icti on CTI from bank to load
		dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

		dc_super_cap_iout = dc_load_iin - power_input/vcti;

		// Test if the supply power is small enough
		if (dc_super_cap_iout < 0) {
			return -1;
		}

		// Compute the current status of the Supercapacitor bank
		dc_super_cap.ConverterModel_supcap(dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_power, sp);

		print_super_cap_info(output, sp, power_input);
		
		// compute the energy consumption
		sp->SupCapCharge(-super_cap_iout, dc_super_cap_vin, min_time_interval, super_cap_vcc, super_cap_qacc);

		// time elapse
		time_elapsed += min_time_interval;
		++time_index;
		HTimer.HEESTimerAdvancdTimerIndex(1, sp);
		load->AdvanceLoadProgress(min_time_interval);

		// stop if there is no energy in the supercapacitor
		if ((sp->SupCapGetEnergy() <= 0) || (HTimer.HEESTimerGetCurrentTimeIndex() > MAX_TIME_INDEX))
			break;
	}

	output.close();

	return time_index;
}

double DischargeProcess::ComputeBestVCTI(double load_vdd, double load_i) {
	
	double c(0.0), d(0.0);
	double beta = 0.084;
	double gamma = beta*load_i*load_i + load_i * load_vdd;

	d = -2*gamma*gamma;
	c = -2*beta*gamma;

	double delta = (d*d/4) + (c*c*c/27);

	double v(0.0);
	if (delta < 0)
		v = -(2.0*c/3.0) * cos(acos(27.0*d/(-2.0*c*c*c))/3.0);
	else
		v = pow(-d/2+sqrt(delta), 1/3.0) + pow(-d/2-sqrt(delta), 1/3.0);

	delta = v*v*v + c*v + d;
	if (fabs(delta) > 0.1)
		cerr << "The solution is wrong!" << endl;

	return v;
}

void DischargeProcess::print_super_cap_info(ofstream &output, supcapacitor *sp, double power_input) {
	// Output the status to a file
	super_cap_voc = sp->SupCapGetVoc();
	super_cap_qacc = sp->SupCapGetQacc();
	output << power_input << "\t"
			<< vcti << "\t"
			<< super_cap_voc << "\t"
			<< super_cap_vcc << "\t"
			<< super_cap_qacc << "\t"
			<< super_cap_iout << "\t"
			<< dc_super_cap_power << "\t"
			<< dc_load_power << "\t"
			<< super_cap_iout*super_cap_iout*sp->SupCapGetRacc() << "\t"
			<< sp->SupCapGetEnergy() << "\t"
			<< sp->SupCapGetCacc() << endl;

	return;
}

/*
double delta_test = dc_super_cap_vin*dc_super_cap_iin-dc_super_cap_vout*dc_super_cap_iout-dc_super_cap_power;
		super_cap_voc = sp->SupCapGetVoc();
		delta_test = super_cap_voc - super_cap_vcc - super_cap_iout*sp->SupCapGetRacc();
		dc_load.ConverterModel(dc_super_cap_vin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_iin, dc_load_power);
		delta_test = dc_load_power - dc_super_cap_power;
*/
