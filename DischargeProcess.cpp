#include <fstream>
#include <iostream>
#include <math.h>

#include "DischargeProcess.hpp"
#include "DCCon_dis.hpp"
#include "DCCon_in.hpp"
#include "SuperCap.hpp"
#include "selVCTI.hpp"
#include "main.hpp"

using namespace std;

DischargeProcess::DischargeProcess() {}

void DischargeProcess::DischargeProcessOurPolicy(supcapacitor *sp, lionbat *lb, loadApplication *load) {

	dcconvertDIS dc_super_cap;
	dcconvertIN dc_load;

	// CTI
	double vcti(0.0), icti(0.0);

	// Super capacitor discharging paramenters
	double super_cap_iout(0.0), super_cap_qacc(0.0), super_cap_voc(0.0), super_cap_vcc(0.0), dc_load_power(0.0);

	// DC-DC converter for the super capacitor bank discharge
	double &dc_super_cap_vin(super_cap_vcc), &dc_super_cap_iin(super_cap_iout), &dc_super_cap_vout(vcti), &dc_super_cap_iout(icti), dc_super_cap_power(0.0);

	// DC-DC converter for the load
	// FIXME: should initilized from load
	double &dc_load_vin(vcti), &dc_load_iin(icti), dc_load_vout(1.0), dc_load_iout(1.0);

	// Task info and timer stuff
	double task_duration = 2000000.0; // FIXME: should be replaced with load->TimeDuration(Vdd)
	double time_elapsed = 0.0;
	int time_index = 0;
	double current_task_remaining_time = task_duration;

	// Output file
	ofstream output("Discharge.txt");
	output << "VCTI\tVcap_oc\tVcap_cc\tQacc\tIsup\tPdcsup\tPdcload\tPrbank\tEsup" << endl;

	// Compute the initial Vcti first
	vcti = ComputeBestVCTI(dc_load_vout, dc_load_iout);
	if (vcti < dc_load_vout) {
		cerr << "Unable to operate at optimal Vcti point!" << endl;
		exit(1);
	}
	
	// Compute the current Icti on CTI from bank to load
	dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

	bool could_reconfig_flag = true;
	while (current_task_remaining_time > 0) {

		// Compute the current status of the Supercapacitor bank
		dc_super_cap.ConverterModel_supcap(dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_power, sp);
				super_cap_voc = sp->SupCapGetVoc();

		// Doing reconfiguration if necessary
		while ((dc_super_cap_vin < dc_super_cap_vout) && (could_reconfig_flag)) {
			could_reconfig_flag = sp->SupCapMoreSeriesReconfig();
			dc_super_cap.ConverterModel_supcap(dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_power, sp);
		}
		// if (!could_reconfig_flag)
		//	break;

		// Output the status to a file
		super_cap_voc = sp->SupCapGetVoc();
		super_cap_qacc = sp->SupCapGetQacc();
		output << vcti << "\t"
				<< super_cap_voc << "\t"
				<< super_cap_vcc << "\t"
				<< super_cap_qacc << "\t"
				<< super_cap_iout << "\t"
				<< dc_super_cap_power << "\t"
				<< dc_load_power << "\t"
				<< super_cap_iout*(sp->SupCapGetRacc()*sp->SupCapGetRacc()) << "\t"
				<< sp->SupCapGetEnergy() << "\t"
				<< sp->SupCapGetCacc() << endl;

		// compute the energy consumption
		sp->SupCapCharge(-super_cap_iout, min_time_interval, super_cap_vcc, super_cap_qacc);

		// stop if there is no energy in the supercapacitor
		if (sp->SupCapGetEnergy() < 0)
			break;

		// time elapse
		current_task_remaining_time -= min_time_interval;
		time_elapsed += min_time_interval;
		++time_index;
	}

	output.close();
}

void DischargeProcess::DischargeProcessOptimalVcti(supcapacitor *sp, lionbat *lb, loadApplication *load) {

	dcconvertDIS dc_super_cap;
	dcconvertIN dc_load;
	selVcti sel_vcti;

	// CTI
	double vcti(1.0), icti(0.0);

	// Super capacitor discharging paramenters
	double super_cap_iout(0.0), super_cap_qacc(0.0), super_cap_voc(0.0), super_cap_vcc(0.0), dc_load_power(0.0);

	// DC-DC converter for the super capacitor bank discharge
	double &dc_super_cap_vin(super_cap_vcc), &dc_super_cap_iin(super_cap_iout), &dc_super_cap_vout(vcti), &dc_super_cap_iout(icti), dc_super_cap_power(0.0);

	// DC-DC converter for the load
	// FIXME: should initilized from load
	double &dc_load_vin(vcti), &dc_load_iin(icti), dc_load_vout(1.0), dc_load_iout(1.0);

	// Task info and timer stuff
	double task_duration = 2000.0; // FIXME: should be replaced with load->TimeDuration(Vdd)
	double time_elapsed = 0.0;
	int time_index = 0;
	double current_task_remaining_time = task_duration;

	// Output file
	ofstream output("Discharge.txt");
	output << "VCTI\tVcap_oc\tVcap_cc\tQacc\tIsup\tPdcsup\tPdcload\tPrbank\tEsup" << endl;

	// Compute the current Icti on CTI from bank to load
	dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

	while (current_task_remaining_time > 0) {

		if (time_index % 10 == 0) {
			vcti = sel_vcti.bestVCTI(0.0, dc_load_iout, dc_load_vout, "dis_SupCap", lb, sp);
			// Compute the current Icti on CTI from bank to load
			dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
		}

		// Compute the current status of the Supercapacitor bank
		dc_super_cap.ConverterModel_supcap(dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_power, sp);
				super_cap_voc = sp->SupCapGetVoc();

		// Output the status to a file
		super_cap_voc = sp->SupCapGetVoc();
		super_cap_qacc = sp->SupCapGetQacc();
		output << vcti << "\t"
				<< super_cap_voc << "\t"
				<< super_cap_vcc << "\t"
				<< super_cap_qacc << "\t"
				<< super_cap_iout << "\t"
				<< dc_super_cap_power << "\t"
				<< dc_load_power << "\t"
				<< super_cap_iout*(sp->SupCapGetRacc()*sp->SupCapGetRacc()) << "\t"
				<< sp->SupCapGetEnergy() << endl;

		// compute the energy consumption
		sp->SupCapCharge(-super_cap_iout, min_time_interval, super_cap_vcc, super_cap_qacc);

		// stop if there is no energy in the supercapacitor
		if (sp->SupCapGetEnergy() < 0)
			break;

		// time elapse
		current_task_remaining_time -= min_time_interval;
		time_elapsed += min_time_interval;
		++time_index;
	}

	output.close();
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

/*
double delta_test = dc_super_cap_vin*dc_super_cap_iin-dc_super_cap_vout*dc_super_cap_iout-dc_super_cap_power;
		super_cap_voc = sp->SupCapGetVoc();
		delta_test = super_cap_voc - super_cap_vcc - super_cap_iout*sp->SupCapGetRacc();
		dc_load.ConverterModel(dc_super_cap_vin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_iin, dc_load_power);
		delta_test = dc_load_power - dc_super_cap_power;
*/
