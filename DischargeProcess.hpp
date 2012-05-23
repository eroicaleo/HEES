#ifndef _DISCHARGE_PROCESS_
#define _DISCHARGE_PROCESS_

#include <fstream>
#include <string>

class supcapacitor;
class lionbat;
class loadApplication;

class DischargeProcess {
public:
	DischargeProcess();
	int DischargeProcessOurPolicy(double power_input, supcapacitor *sp, lionbat *lb, loadApplication *load);
	int DischargeProcessOptimalVcti(double power_input, supcapacitor *sp, lionbat *lb, loadApplication *load); 
private:
	double ComputeBestVCTI(double load_vdd, double load_i);
	void print_super_cap_info(std::ofstream &output, supcapacitor *sp, double power_input); 

	double vcti, icti;
	double super_cap_iout, super_cap_qacc, super_cap_voc, super_cap_vcc, dc_load_power;
	double &dc_super_cap_vin, &dc_super_cap_iin, &dc_super_cap_vout, dc_super_cap_iout, dc_super_cap_power;
	double &dc_load_vin, &dc_load_iin, dc_load_vout, dc_load_iout; 
	std::string output_file; 
};

#endif
