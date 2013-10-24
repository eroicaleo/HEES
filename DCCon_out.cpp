#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "DCCon_out.hpp"
#include <vector>
//#include "LionBat.hpp"
#include "SuperCap.hpp"
#include "main.hpp"

using namespace std;

dcconvertOUT::dcconvertOUT() :
	m_Rsw(4, 25),
	m_Qsw(4, 60),
	m_RL(39),
	m_RC(100),
	m_Ictrl(4),
	m_fs(500),
	m_Lf(6) { }

/*void dcconvertOUT::ConverterModel_LionBat(double Vin, double Iin, double &Vout, double &Iout, double &Pdcdc, lionbat *lion_battery) {

	// Write the Vin, Iin, SoC of the battery to the dcdc_input.txt for Matlab
	ofstream dcdc_input_file("./matlab/dcdc_battery_input.txt");
	dcdc_input_file << Vin << endl
					<< Iin << endl
					<< lion_battery->VsocCheck(0.0, 0.0) << endl;
	dcdc_input_file.close();

	// Call the matlab to solve the Vout and Iout
	system("matlab -nojvm -nodisplay -nosplash < ./matlab/dcdc_battery_test.m");
	// Call the sundials to solve the Vout and Iout
	// bsolver->SolveItGivenDCInput(Vin, Iin, Vout, Iout, Pdcdc, lion_battery);

	// Retrieve the Vout, Iout and Pdcdc infomation
	ifstream dcdc_output_file("dcdc_battery_output.txt");
	dcdc_output_file >> Vout >> Iout >> Pdcdc;
	dcdc_output_file.close();

	// Error check
	if ((Vout < 0) && (Iout < 0) && (Pdcdc < 0)) {
		cerr << "ERROR: matlab results are wrong!" << endl;
	}

	return;
}
*/

void dcconvertOUT::MatlabSolverGivenDCInput(double Vin, double Iin, double &Vout, double &Iout, double &Pdcdc, ees_bank *bank) {

    // Write the Vin, Iin, SoC of the battery to the dcdc_input.txt for Matlab
	ofstream dcdc_input_file("./matlab/dcdc_supcap_input.txt");
	dcdc_input_file << Vin << endl
					<< Iin << endl
					<< bank->EESBankGetRacc()<< endl 
					<< bank->EESBankGetCacc()<< endl
					<< bank->EESBankGetQacc()<< endl;
	dcdc_input_file.close();

	// Call the matlab to solve the Vout and Iout
	system("matlab -nojvm -nodisplay -nosplash < ./matlab/dcdc_supcap_test.m");

	// Retrieve the Vout, Iout and Pdcdc infomation
	ifstream dcdc_output_file("dcdc_supcap_output.txt");
	dcdc_output_file >> Vout >> Iout >> Pdcdc;
	dcdc_output_file.close();

	return;
}

void dcconvertOUT::ConverterModel_EESBank(double Vin, double Iin, double &Vout, double &Iout, double &Pdcdc, ees_bank *bank){

	// Using the sundial solver
	int ret = dc_solver.SolveItGivenDCInput(Vin, Iin, Vout, Iout, Pdcdc, bank);

	double delta = fabs(Vout*Iout+Pdcdc-Vin*Iin);

	if ((ret == -2) || (delta > 1.e-3)) {
#ifdef DEBUG_YANGGE
		cerr << "WARNING: sundial is not able to find valid solution! Try Matlab!" << endl;
#endif

#ifdef USING_MATLAB
		MatlabSolverGivenDCInput(Vin, Iin, Vout, Iout, Pdcdc, bank);
		delta = fabs(Vout*Iout+Pdcdc-Vin*Iin);
		// Error check
		if ((Vout < 0) || (Iout < 0) || (Pdcdc < 0) || (delta > 1.e-3)) {
			cerr << "WARNING: matlab can't get valid results either!" << endl;
		}
#endif
	}

    return;
}
