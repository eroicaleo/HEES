#include <fstream>
#include <iostream>
#include <math.h>

#include "SuperCap.hpp"
#include "DCCon_out.hpp"
#include "LoadApp.hpp"
#include "LionBat.hpp"
#include "DischargeProcess.hpp"
#include "ChargeProcess.hpp"
#include "HEESTimer.hpp"

#include "main.hpp"
#include "powersource.hpp"

using namespace std;

HEESTimer HTimer(0, 1000);

int main(){
	supcapacitor sp;
	lionbat lb; 
	loadApplication load;

	DischargeProcess dp;
	ChargeProcess cp;

	sp.SupCapReset();
	// sp.SupCapSetQacc(22.91672);
	sp.SupCapReconfig(4, 1);
	// sp.SupCapSetQacc(0.0);
	sp.SupCapSetQacc(0.0);

	// Set current task info
	double vdd = 1.0, idd = 1.0;
	double deadline = 10.0, exec_time = 10.0;
	load.SetTaskParameters(vdd, idd, deadline, exec_time);

	// timer staff
	int time_index = -1;
	int hh = 11, mm = 0, ss = 0;
	HTimer.HEESTimerSetCurrentSecond(3600*hh + 60*mm + ss);
	
	// powersource
	double power_input = 0.0;

	// The main loop
	while (HTimer.HEESTimerGetCurrentTimeIndex() < MAX_TIME_INDEX) {

		power_input = powersource_sec(HTimer.HEESTimerGetCurrentTimeInSecond());
		// power_input = 1.75;

		// if ((total_time_index/100) % 2 == 1) {
		// if (total_time_index == 3000) {
		// 	load.SetTaskParameters(vdd, 1.3, deadline, exec_time);
		// }

		// if ((total_time_index/100) % 2 == 0) {
		// if (total_time_index == 0) {
			load.SetTaskParameters(vdd, 1.0, deadline, exec_time);
		// }

		// ChargeProcess
		time_index = cp.ChargeProcessOurPolicy(power_input, &sp, &lb, &load);
		// time_index = cp.ChargeProcessOptimalVcti(power_input, &sp, &lb, &load);

		// DischargeProcess
		if (time_index < 0) {
			// time_index = dp.DischargeProcessOurPolicy(power_input, &sp, &lb, &load);
			// time_index = dp.DischargeProcessOptimalVcti(power_input, &sp, &lb, &load);
		}

		if (time_index < 0) {
			cerr << "time_index can not be less than 0" << endl;
			break;
		}

		if (sp.SupCapGetEnergy() <= 0)
			break;
	}

#ifdef _COMPLEX_MAIN_
	dcconvertOUT dcout;

	//powersource
	double VCTI = 1.0, powerInput = 1.25, delVCTI = 0;
	//DCDC converter
	double dccon1_Vin = 0.0, dccon1_Vout = 0.0, dccon1_Iout = 0.0, dccon1_Iin = 0.0, dccon1_Pdcdc = 0.0;
	//SuperCap
	double sup_Iin = 0.0, sup_Tdur = 0.0, sup_Qacc = 0.0, sup_Vs = 0.0;

	double dccon1_energy = 0.0;

	bool supcap_reconfig_return = true;
	// int charging_phase = 1;
	double Prbank = 0.0;

	int task = 1;
	double task_duration = 2000.0;

	string filename1("Result.txt");
	string filename2("Process.txt");
	FILE *fp1 = fopen(filename1.c_str(),"w");
	FILE *fp2 = fopen(filename2.c_str(),"w");
	
	fprintf(fp1,"Isup\t Vsup\t Psup\n");
	fprintf(fp2,"VCTI\t Vs_sup\t Vs_dc\t del_VCTI\t Qacc\n");
	
	while(task < 0){
		dccon1_Iin = powerInput/VCTI - 1.153;
		dccon1_Vin = VCTI;

		double current_task_remaining_time = task_duration;
		while (current_task_remaining_time > 0) {
			
			/*
			if ((charging_phase == 1) && (sp.SupCapGetEnergy() > 20)) {
				charging_phase = 2;
				sp.SupCapReconfig(2, 2);
			}
			if ((charging_phase == 2) && (sp.SupCapGetEnergy() > 90)) {
				charging_phase = 3;
				sp.SupCapReconfig(1, 4);
			}
			*/

			dcout.ConverterModel_SupCap(dccon1_Vin, dccon1_Iin, dccon1_Vout, dccon1_Iout, dccon1_Pdcdc, &sp);
			sup_Iin = dccon1_Iout;
			sup_Tdur = min_time_interval;
			while ((dccon1_Vout > dccon1_Vin) && supcap_reconfig_return) {
				supcap_reconfig_return = sp.SupCapOperating(sup_Iin, VCTI, delVCTI);
				dcout.ConverterModel_SupCap(dccon1_Vin, dccon1_Iin, dccon1_Vout, dccon1_Iout, dccon1_Pdcdc, &sp);
			}
			// If the supercapacitor can not reconfigure anymore, abort
			/*
			if (!supcap_reconfig_return) {
				break;
			}
			*/

			Prbank = dccon1_Iout * dccon1_Iout * sp.SupCapGetRacc();
			fprintf(fp1,"%f\t%f\t%f\t%f\n", dccon1_Iout, dccon1_Vout, dccon1_Pdcdc, Prbank);
			sp.SupCapCharge(sup_Iin, sup_Tdur, sup_Vs, sup_Qacc);
			delVCTI = fabs(VCTI - sup_Vs);
			fprintf(fp2, "%f\t%f\t%f\t%f\t%f\n", VCTI, sup_Vs, dccon1_Vout, delVCTI, sp.SupCapGetQacc());
	//		sp.SupCapOperating(sup_Iin, VCTI, delVCTI);
			current_task_remaining_time -= min_time_interval;
			time_elapsed += min_time_interval;
			++time_index;

			if (time_index > MAX_TIME_INDEX)
				break;

			dccon1_energy += dccon1_Pdcdc * min_time_interval;
		}

		/*
		if (!supcap_reconfig_return) {
			break;
		}
		*/
		task--;
	}
	fclose(fp1);
	fclose(fp2);

	printf("energy consumed is %f, time elapsed is %f\n", dccon1_energy, time_elapsed);
#endif

	return 0;
}
