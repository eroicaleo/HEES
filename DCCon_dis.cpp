#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include "DCCon_dis.hpp"
#include "LionBat.hpp"
#include "SuperCap.hpp"
using namespace std;

dcconvertDIS::dcconvertDIS() :
	m_Rsw(4, 25),
	m_Qsw(4, 60),
	m_RL(39),
	m_RC(100),
	m_Ictrl(4),
	m_fs(500),
	m_Lf(6) { }

void dcconvertDIS::ConverterModel_battery(double Vout, double Iout, double &Vin, double &Iin, double &Pdcdc, lionbat *lion_battery) {

	// Write the Vin, Iin, SoC of the battery to the dcdc_input.txt for Matlab
	ofstream dcdc_input_file("./matlab/dcdc_battery_input.txt");
	dcdc_input_file << Vout << endl
					<< Iout << endl
					<< lion_battery->VsocCheck(0.0, 0.0) << endl;
	dcdc_input_file.close();

	// Call the matlab to solve the Vout and Iout
	system("matlab -nojvm -nodisplay -nosplash < ./matlab/battery_dcdc_test.m");
	// Call the sundials to solve the Vout and Iout
	// bsolver->SolveItGivenDCInput(Vin, Iin, Vout, Iout, Pdcdc, lion_battery);

	// Retrieve the Vout, Iout and Pdcdc infomation
	ifstream dcdc_output_file("dcdc_battery_output.txt");
	dcdc_output_file >> Vin >> Iin >> Pdcdc;
	dcdc_output_file.close();

	// Error check
	if ((Vin < 0) && (Iin < 0) && (Pdcdc < 0)) {
		cerr << "ERROR: matlab results are wrong!" << endl;
	}

	return;
}

void dcconvertDIS::ConverterModel_supcap(double Vout, double Iout, double &Vin, double &Iin, double &Pdcdc, supcapacitor *sp) {

	// We first check if the bank is empty of not
	if (sp->SupCapGetQacc() <= 0.0) {
		Vin = 0.0;
		Iin = 0.0;
		Pdcdc = 0.0;
		return;
	}

	// Using the sundial solver
	int ret = dc_solver.SolveItGivenDCOutput(Vout, Iout, Vin, Iin, Pdcdc, sp);

	double delta = fabs(Vout*Iout+Pdcdc-Vin*Iin);

	if ((ret == -2) || (delta > 1.e-3)) {
		cerr << "WARNING: sundial is not able to find valid solution! Try Matlab!" << endl;
		MatlabSolverGivenDCOutput(Vout, Iout, Vin, Iin, Pdcdc, sp);
	}

    // Error check
    if ((Vin < 0) && (Iin < 0) && (Pdcdc < 0)) {
        cerr << "ERROR: matlab results are wrong! Negative!" << endl;
    }

	delta = fabs(Vout*Iout+Pdcdc-Vin*Iin);

	if (delta > 1.e-3) {
		cerr << "WARNING: Can not find valid solutions even in matlab!" << endl; 
		Vin = 0.0;
		Iin = 0.0;
		Pdcdc = 0.0;
		/* 
		 * Since we are in discharge mode, if it is most serial configuration
		 * and we still can not solve it, we are not gonna further discharge it
		 * just set the Qacc to 0 for now
		 */
		if (sp->SupCapIsFullySerial()) {
			sp->SupCapSetQacc(0.0);
		}
		return;
	}

    if ((Iin > 5) || (Pdcdc > 10)) {
        cerr << "ERROR: matlab results are wrong! Too big!" << endl;
    }

    return;
}

void dcconvertDIS::MatlabSolverGivenDCOutput(double Vout, double Iout, double &Vin, double &Iin, double &Pdcdc, supcapacitor *sp) {

    // Write the Vin, Iin, SoC of the battery to the dcdc_input.txt for Matlab
    ofstream dcdc_input_file("./matlab/dcdc_supcap_input.txt");
    dcdc_input_file << Vout << endl
                    << Iout << endl
					<< sp -> SupCapGetRacc() << endl
					<< sp -> SupCapGetCacc() << endl
                    << sp -> SupCapGetQacc() << endl;
    dcdc_input_file.close();

    // Call the matlab to solve the Vout and Iout
    system("matlab -nojvm -nodisplay -nosplash < ./matlab/supcap_dcdc_test.m");
    // Call the sundials to solve the Vout and Iout
    // bsolver->SolveItGivenDCInput(Vin, Iin, Vout, Iout, Pdcdc, lion_battery);

    // Retrieve the Vout, Iout and Pdcdc infomation
    ifstream dcdc_output_file("dcdc_supcap_output.txt");
    dcdc_output_file >> Vin >> Iin >> Pdcdc;
    dcdc_output_file.close();
}
