#include <iostream>
#include <fstream>
#include <vector>

#include "../DCCon_in.hpp"
#include "../ParseCommandLine.hpp"
#include "../nnet/nnetmultitask.hpp"

using namespace std;

struct Trace {

	Trace(double e) : initEnergy(e) {
	}

	double initEnergy;
	vector<double> voltageTrace;
	vector<double> currentTrace;
	vector<double> solarTrace;

	void readVCTraceFromFile(string filename);
	void readSolarTraceFromFile(string filename);
	double predictTasksEnergyInterval();
	void addDCDCPower(vector<double> &powerTrace);
	double predictPowerInterval(const vector<double> &chargeTrace, double startEnergy);

	nnetmultitask nnetPredictor;
};

double Trace::predictPowerInterval(const vector<double> &chargeTrace, double startEnergy) {

	double energy = startEnergy;
	vector<double>::const_iterator start = chargeTrace.begin();
	vector<double>::const_iterator end;
	while (start != chargeTrace.end()) {
		end = find_if(start, chargeTrace.end(), bind1st(not_equal_to<double>(), *start));
		energy = nnetPredictor.predictWithEnergyLength(*start, energy, end-start);
		start = end;
	}

	return energy;
}

double Trace::predictTasksEnergyInterval() {
	vector<double> powerTrace;

	transform(voltageTrace.begin(), voltageTrace.end(),
				currentTrace.begin(), back_inserter(powerTrace),
				multiplies<double>());

	addDCDCPower(powerTrace);

	vector<double> chargePowerInterval;
	solarTrace.resize(powerTrace.size());
	transform(solarTrace.begin(), solarTrace.end(), powerTrace.begin(), back_inserter(chargePowerInterval), minus<double>());

	return predictPowerInterval(chargePowerInterval, initEnergy);
}

void Trace::addDCDCPower(vector<double> &powerTrace) {
	vector<pair<double, double> > vcPairTrace;
	transform(voltageTrace.begin(), voltageTrace.end(),
				currentTrace.begin(), back_inserter(vcPairTrace),
				make_pair<double, double>);

	dcconvertIN dcLoad;
	double dc_load_vin(1.0), dc_load_vout(1.0), dc_load_iout(0.0);
	double dc_load_iin(0.0), dc_load_power(0.0);

	vector<pair<double, double> >::iterator start = vcPairTrace.begin();
	vector<pair<double, double> >::iterator end;

	vector<double>::iterator s;
	vector<double>::iterator e;

	while (start != vcPairTrace.end()) {
		dc_load_vout = start->first;
		dc_load_iout = start->second;
		dcLoad.ConverterModel(dc_load_vin, dc_load_vout, dc_load_iout, dc_load_iin, dc_load_power);
		end = find_if(start, vcPairTrace.end(), bind1st(not_equal_to<pair<double, double> >(), *start));
		s = powerTrace.begin() + (start - vcPairTrace.begin());
		e = powerTrace.begin() + (end - vcPairTrace.begin());
		transform(s, e, s, bind2nd(plus<double>(), dc_load_power));
		start = end;
	}

	return;
}

void Trace::readVCTraceFromFile(string filename) {
	ifstream infile(filename.c_str());
	if (!infile) {
		cerr << "Can not find " << filename << " for read!" << endl;
		exit(66);
	}

	voltageTrace.clear();
	currentTrace.clear();
	double v, c;
	int l;
	while ((infile >> v >> c >> l).good()) {
		voltageTrace.insert(voltageTrace.end(), l, v);
		currentTrace.insert(currentTrace.end(), l, c);
	}

	infile.close();
}

void Trace::readSolarTraceFromFile(string filename) {
	ifstream infile(filename.c_str());
	if (!infile) {
		cerr << "Can not find " << filename << " for read!" << endl;
		exit(66);
	}

	solarTrace.clear();
	double p;
	int l;
	while ((infile >> p >> l).good()) {
		solarTrace.insert(solarTrace.end(), l, p);
	}

	infile.close();
}

int main(int argc, char *argv[]) {

	hees_parse_command_line(argc, argv);

	double cap = supcap_cap * supcap_parallel_conf / supcap_serial_conf;
	double initEnergy = 0.5 * (supcap_init_charge * supcap_init_charge) / cap;
	
	Trace trace(initEnergy);
	trace.readVCTraceFromFile("TasksSCHEDForEES.txt");
	trace.readSolarTraceFromFile("VariablePowerSource.txt");
	double finalEnergy = trace.predictTasksEnergyInterval();

	cout << "Final Energy: " << finalEnergy << endl;

	return 0;
}
