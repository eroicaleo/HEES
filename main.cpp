#include <iostream>
#include <fstream>
#include "SuperCap.hpp"
#include "DCCon_out.hpp"
#include <math.h>
using namespace std;

int main(){
	dcconvertOUT dcout;
	supcapacitor sp;
	//powersource
	double VCTI = 1.0, powerInput = 1.75, delVCTI = 0;
	//DCDC converter
	double dccon1_Vin = 0.0, dccon1_Vout = 0.0, dccon1_Iout = 0.0, dccon1_Iin = 0.0, dccon1_Pdcdc = 0.0;
	//SuperCap
	double sup_Iin = 0.0, sup_Tdur = 0.0, sup_Qacc = 0.0, sup_Vs = 0.0;

	double dccon1_energy = 0.0;

	int task = 1;
	double task_duration = 2.0;
	double min_time_interval = 0.1;

	string filename1("Result.txt");
	string filename2("Process.txt");
	FILE *fp1 = fopen(filename1.c_str(),"w");
	FILE *fp2 = fopen(filename2.c_str(),"w");
	
	fprintf(fp1,"Isup\t Vsup\t Psup\n");
	fprintf(fp2,"VCTI\t Vs_sup\t Vs_dc\t del_VCTI\t Qacc\n");
	sp.SupCapReset();
	while(task > 0){
		dccon1_Iin = powerInput/VCTI - 1.153;
		dccon1_Vin = VCTI;

		double current_task_remaining_time = task_duration;
		while (current_task_remaining_time > 0) {
			dcout.ConverterModel_SupCap(dccon1_Vin, dccon1_Iin, dccon1_Vout, dccon1_Iout, dccon1_Pdcdc, &sp);
			fprintf(fp1,"%f\t%f\t%f\n", dccon1_Iout, dccon1_Vout, dccon1_Pdcdc);
			sup_Iin = dccon1_Iout;
			sup_Tdur = min_time_interval;
	//		sp.SupCapOperating(sup_Iin, VCTI, delVCTI);
			sp.SupCapCharge(sup_Iin, sup_Tdur, sup_Vs, sup_Qacc);
			delVCTI = fabs(VCTI - sup_Vs);
			fprintf(fp2, "%f\t%f\t%f\t%f\t%f\n", VCTI, sup_Vs, dccon1_Vout, delVCTI, sp.SupCapGetQacc());
	//		sp.SupCapOperating(sup_Iin, VCTI, delVCTI);
			current_task_remaining_time -= min_time_interval;

			dccon1_energy += dccon1_Pdcdc * min_time_interval;
		}
		task--;
	}
	fclose(fp1);
	fclose(fp2);

	printf("%f\n", dccon1_energy);

	return 0;
}
