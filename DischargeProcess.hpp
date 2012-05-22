#ifndef _DISCHARGE_PROCESS_
#define _DISCHARGE_PROCESS_

class supcapacitor;
class lionbat;
class loadApplication;

class DischargeProcess {
public:
	DischargeProcess();
	void DischargeProcessOurPolicy(supcapacitor *sp, lionbat *lb, loadApplication *load);
	void DischargeProcessOptimalVcti(supcapacitor *sp, lionbat *lb, loadApplication *load); 
private:
	double ComputeBestVCTI(double load_vdd, double load_i);
};

#endif
