#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>

#include "ChargeProcess.hpp"
#include "DCCon_in.hpp"
#include "DCCon_out.hpp"
#include "HEESTimer.hpp"
#include "LoadApp.hpp"
#include "SuperCap.hpp"
#include "main.hpp"

using namespace std;
using namespace std::tr1;

extern HEESTimer HTimer;

double dc_super_cap_energy = 0.0;
double bank_res_energy = 0.0;

ChargeProcess::ChargeProcess() :
	vcti(0.0), icti(0.0),
	super_cap_iin(0.0), super_cap_qacc(0.0), super_cap_voc(0.0), super_cap_vcc(0.0), dc_load_power(0.0),
	dc_super_cap_vout(super_cap_vcc), dc_super_cap_iout(super_cap_iin), dc_super_cap_vin(vcti), dc_super_cap_iin(0.0), dc_super_cap_power(0.0),
	dc_load_vin(vcti), dc_load_iin(icti), dc_load_vout(1.0), dc_load_iout(1.0), 
	power_input(0.0),
	time_elapsed(0.0), time_index(0),
	output_file("OverallProcess.txt") { 
	
	ofstream output(output_file.c_str());
	output << setw(8) << "Timeix"    << "    ";
	output << setw(8) << "Pinput"    << "    ";
	output << setw(8) << "VCTI"      << "    ";
	output << setw(8) << "Vcap_oc"   << "    ";
	output << setw(8) << "Vcap_cc"   << "    ";
	output << setw(8) << "Qacc"      << "    ";
	output << setw(8) << "Isup"      << "    ";
	output << setw(8) << "Pdcsup"    << "    ";
	output << setw(8) << "PdcsupIi"  << "    ";
	output << setw(8) << "Pdcload"   << "    ";
	output << setw(8) << "Prbank"    << "    ";
	output << setw(8) << "Esup"      << "    ";
	output << setw(8) << "Cacc"      << "    ";
	output << setw(8) << endl;
	output.close();
}

void ChargeProcess::compute_dc_bank_iin() {
	// Compute the current Icti on CTI from bank to load
	dc_load.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);

	dc_super_cap_iin = power_input/vcti - dc_load_iin;

	// Test if the supply power is large enough
	if (dc_super_cap_iin < 0) {
		power_status = POWER_NOT_ENOUGH_FOR_LOAD;
	} else {
		power_status = POWER_NORMAL;
	}
}

void ChargeProcess::charge_policy_our_policy(ees_bank *bank) {

	// Set the VCTI to the load vdd;
	if (power_status == POWER_INIT) {
		if (fixed_vcti > 0.0)
			vcti = fixed_vcti;
		else
			vcti = dc_load_vout;
		compute_dc_bank_iin();
	}

	if (power_status == POWER_NORMAL) {
		dc_super_cap.ConverterModel_EESBank(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, bank);
		// Reconfig if necessary
		// while ((dc_super_cap_vout > dc_super_cap_vin) && supcap_reconfig_return && bank_reconfig_enable) {
			// supcap_reconfig_return = bank->EESBankOperating(dc_super_cap_iin, dc_super_cap_vout, -100.0);
			// supcap_reconfig_return = bank->EESBankReconfiguration(dc_super_cap_iin, dc_super_cap_vout, &dc_super_cap);
			// dc_super_cap.ConverterModel_EESBank(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, bank);
		// }
		if ((dc_super_cap_vout > dc_super_cap_vin) && supcap_reconfig_return && bank_reconfig_enable) {
			supcap_reconfig_return = bank->EESBankReconfiguration(dc_super_cap_iin, dc_super_cap_vout, &dc_super_cap);
			dc_super_cap.ConverterModel_EESBank(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, bank);
		}
	}

	return;
}

void ChargeProcess::charge_policy_optimal_vcti(ees_bank *bank) {
	if (time_index % recompute_vcti_time_index == 0) {
		vcti = sel_vcti.bestVCTI(power_input, dc_load_iout, dc_load_vout, "out_SupCap", bank);
	}

	compute_dc_bank_iin();
	if (power_status == POWER_NORMAL) {
		dc_super_cap.ConverterModel_EESBank(dc_super_cap_vin, dc_super_cap_iin, dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power, bank);
	}

	return;
}

int ChargeProcess::ChargeProcessOptimalVcti(ees_bank *bank, lionbat *lb, loadApplication *load) {

	this->power_input = power_source_func(HTimer.HEESTimerGetCurrentTimeInSecond());
	power_status = POWER_INIT;

	// Bind to optimal vcti policy
	charge_policy = bind(&ChargeProcess::charge_policy_optimal_vcti, this, placeholders::_1);

	return ChargeProcessApplyPolicy(bank, lb, load);
}

int ChargeProcess::ChargeProcessOurPolicy(ees_bank *bank, lionbat *lb, loadApplication *load) {

	this->power_input = power_source_func(HTimer.HEESTimerGetCurrentTimeInSecond());
	power_status = POWER_INIT;

	supcap_reconfig_return = true;

	// Bind to our policy
	charge_policy = bind(&ChargeProcess::charge_policy_our_policy, this, placeholders::_1);

	return ChargeProcessApplyPolicy(bank, lb, load);
}

int ChargeProcess::ChargeProcessApplyPolicy(ees_bank *bank, lionbat *lb, loadApplication *load) {
	
	// DC-DC converter for the load
	dc_load_vout = load->get_vdd();
	dc_load_iout = load->get_idd();

	// Task info and timer stuff
	time_elapsed = 0.0;
	time_index = 0;

	// Output file
	ofstream output(output_file.c_str(), ios_base::app);
	if (!output.good()) {
		cerr << "Can not open files!" << endl;
	}

	if (HTimer.HEESTimerGetCurrentTimeIndex() == 0)
		print_super_cap_info(output, bank, power_input, HTimer);

	while (fabs(load->CurrentTaskRemainingTime()) > 1e-3) {

		// Every second, update the power input
		if (time_index % 10 == 0) {
			this->power_input = power_source_func(HTimer.HEESTimerGetCurrentTimeInSecond());
			power_status = POWER_INIT;
		}
		charge_policy(bank);

		// Check if we need to break because the power_input is not enough
		if (power_status == POWER_NOT_ENOUGH_FOR_LOAD) {
			time_index = -1;
			// Still report initial bank status
			print_super_cap_info(output, bank, power_input, HTimer);
			break;
		}
		
		time_elapsed += min_time_interval;
		++time_index;
		HTimer.HEESTimerAdvancdTimerIndex(1, bank);
		load->AdvanceLoadProgress(min_time_interval);

		// Charge the bank for the current min_time_interval
		bank->EESBankCharge(super_cap_iin, dc_super_cap_vout, min_time_interval, super_cap_vcc, super_cap_qacc);

		// Record the current status of the ees bank at the end of each second
		// Because we normall set delta_energy_steps to 10
		if (HTimer.HEESTimerGetCurrentTimeIndex() % delta_energy_steps == 0)
			print_super_cap_info(output, bank, power_input, HTimer);

		// Recorde the wasted energy on dcdc and bank resistance
		dc_super_cap_energy += min_time_interval * dc_super_cap_power;
		bank_res_energy += min_time_interval * super_cap_iin * super_cap_iin *bank->EESBankGetRacc();

		if (HTimer.HEESTimerGetCurrentTimeIndex() > MAX_TIME_INDEX)
			break;
	}

	output.close();
	// After charging, reset the power status to POWER_INIT
	power_status = POWER_INIT;

	return time_index;
}

void ChargeProcess::print_super_cap_info(ofstream &output, ees_bank *bank, double power_input, const HEESTimer &htimer) {
	// Output the status to a file
	super_cap_voc = bank->EESBankGetVoc();
	super_cap_qacc = bank->EESBankGetQacc();
	output.precision(3);
	output.setf(ios::fixed,ios::floatfield);
    output << setw(8) << htimer.HEESTimerGetCurrentTimeIndex() << "    "
		   << setw(8) << power_input << "    "
           << setw(8) << vcti << "    "
           << setw(8) << super_cap_voc << "    "
           << setw(8) << super_cap_vcc << "    "
           << setw(8) << super_cap_qacc << "    "
           << setw(8) << super_cap_iin << "    "
           << setw(8) << dc_super_cap_power << "    "
           << setw(8) << dc_super_cap_iin << "    "
           << setw(8) << dc_load_power << "    "
           << setw(8) << super_cap_iin*super_cap_iin*bank->EESBankGetRacc() << "    "
           << setw(8) << bank->EESBankGetEnergy() << "    "
           << setw(8) << bank->EESBankGetCacc() << endl;

	return;
}
