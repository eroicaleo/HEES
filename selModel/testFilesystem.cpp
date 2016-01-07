#include <algorithm>
#include <iostream>
#include <functional>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/xpressive/xpressive.hpp>

#include "testFilesystem.hpp"

using namespace boost::filesystem;
using namespace boost::xpressive;
using std::cout;
using std::endl;
using std::make_pair;
using std::map;
using std::mem_fun_ref;
using std::pair;
using std::string;
using std::transform;
using std::vector;

/*
 * General reference:
 *   1. http://www.boost.org/doc/libs/1_59_0/libs/filesystem/doc/reference.html
 *   2. http://www.boost.org/doc/libs/1_59_0/libs/filesystem/doc/tutorial.html
 *   3. http://www.boost.org/doc/libs/1_49_0/doc/html/xpressive/user_s_guide.html
 *   4. http://stackoverflow.com/questions/1257721/can-i-use-a-mask-to-iterate-files-in-a-directory-with-boost
 */

const nnetmodel &recallNnetMap(const map<double, Level2Map> &nnetMap, double bankCharge, double solarPower) {

	// Find the 1st levle key
	if (nnetMap.upper_bound(bankCharge) == nnetMap.begin()) {
		std::cerr << "bank charge: " << bankCharge << " is out of training range!"
			<< " The min training range is: " << nnetMap.begin()->first << endl;
		exit(-1);
	}
	const Level2Map &l2map = (--nnetMap.upper_bound(bankCharge))->second;

	// Find the 2nd level key
	if (l2map.upper_bound(solarPower) == l2map.begin()) {
		std::cerr << "sun charge: " << solarPower << " is out of training range!"
			<< " The min training rage is: " << l2map.begin()->first << endl;
		exit(-1);
	}
	const nnetmodel &nmodel = (--l2map.upper_bound(solarPower))->second;

	return nmodel;
}

void buildNnetMap(map<double, Level2Map> &nnetMap) {

	// To avoid the run time error:
	// terminate called after throwing an instance of 'std::runtime_error'
	// what():  locale::facet::_S_create_c_locale name not valid
	// https://svn.boost.org/trac/boost/ticket/5928

	double bank_vol = 0.0;
	double solar_min = 0.0;
	double solar_max = 10000.0;

	path model_dir(string("./data/PredictionModel/"));
	if (is_directory(model_dir)) {
		std::vector<directory_entry> v_entry;
		std::vector<path> v;

		// http://stackoverflow.com/questions/458525/iterating-over-vector-and-calling-functions
		// http://www.cplusplus.com/reference/functional/mem_fun_ref/
		copy(directory_iterator(model_dir), directory_iterator(), back_inserter(v_entry));
		transform(v_entry.begin(), v_entry.end(), back_inserter(v), mem_fun_ref(&directory_entry::path));

		sregex rex1 = sregex::compile("nnetmodel(\\d+)");
		sregex rex2 = sregex::compile("nnetmodel(\\d+)_([\\d.]+)_([\\d.]+|inf)");
		smatch what;
		for (std::vector<path>::const_iterator it(v.begin()); it != v.end(); ++it) {
			cout << it->filename() << endl;

			solar_min = 0.0;
			solar_max = 10000.0;

			// http://stackoverflow.com/questions/4179322/how-to-convert-boost-path-type-to-string
			if (regex_match(it->filename().string(), what, rex1)) {

			} else if (regex_match(it->filename().string(), what, rex2)) {
				solar_min = atof(what.str(2).c_str());
				if (what.str(3) != "inf") {
					solar_max = atof(what.str(3).c_str());
				}
			} else {
				std::cerr << "Skip wrong filename: " << it->filename() << '\n';
				continue;
			}
			bank_vol = atof(what.str(1).c_str());
			std::cout << "bank voltage: " << bank_vol << std::endl;
			std::cout << "solar min: " << solar_min << std::endl;
			std::cout << "solar max: " << solar_max << std::endl;

			/* Read in the neural network model */
			struct nnetmodel nnet;
			nnet.readnnetmodel(it->string().c_str());
			nnet.energy_offset = 0.5 * pow(bank_vol, 2) / 40.0; 
			nnet.bank_vol = bank_vol;
			nnet.solar_min = solar_min;
			nnet.solar_max = solar_max;

			/* Insesrt the new model */
			if (nnetMap.find(bank_vol) == nnetMap.end()) {
				Level2Map l2map;
				nnetMap.insert(std::pair<double, Level2Map >(bank_vol, l2map));
			}
			nnetMap.find(bank_vol)->second.insert(std::pair<double, nnetmodel>(solar_min, nnet));
		}
	}

}

#ifdef TEST_FILESYSTEM_MAIN
int main(int argc, char *argv[]) {

	map<double, Level2Map> nnetMap;
	buildNnetMap(nnetMap);

	cout << "###############################" << endl;
	cout << "## Start recall process" << endl;
	cout << "###############################" << endl;

	std::vector<pair<double, double> > testcase;
	testcase.push_back(make_pair(1400.0, 0.5));
	testcase.push_back(make_pair(1400.0, 1.5));
	testcase.push_back(make_pair(1400.0, 2.5));
	testcase.push_back(make_pair(1450.0, 0.5));
	testcase.push_back(make_pair(1435.0, 1.5));
	testcase.push_back(make_pair(1476.0, 2.5));

	testcase.push_back(make_pair(1300.0, 0.5));
	testcase.push_back(make_pair(1300.0, 1.5));
	testcase.push_back(make_pair(1300.0, 2.5));
	testcase.push_back(make_pair(1375.0, 0.5));
	testcase.push_back(make_pair(1303.0, 1.5));
    testcase.push_back(make_pair(1350.0, 2.5));

    testcase.push_back(make_pair(1200.0, 0.5));
    testcase.push_back(make_pair(1200.0, 1.5));
    testcase.push_back(make_pair(1200.0, 2.5));
    testcase.push_back(make_pair(1295.0, 0.5));
    testcase.push_back(make_pair(1253.0, 1.5));
    testcase.push_back(make_pair(1290.0, 2.5));

    testcase.push_back(make_pair(1100.0, 0.5));
    testcase.push_back(make_pair(1100.0, 1.5));
    testcase.push_back(make_pair(1100.0, 2.5));
    testcase.push_back(make_pair(1175.0, 0.5));
    testcase.push_back(make_pair(1103.0, 1.5));
    testcase.push_back(make_pair(1150.0, 2.5));

    testcase.push_back(make_pair(1000.0, 0.5));
    testcase.push_back(make_pair(1000.0, 1.5));
    testcase.push_back(make_pair(1000.0, 2.5));
    testcase.push_back(make_pair(1075.0, 0.5));
    testcase.push_back(make_pair(1003.0, 1.5));
    testcase.push_back(make_pair(1050.0, 2.5));

    testcase.push_back(make_pair(900.0, 0.5));
    testcase.push_back(make_pair(900.0, 1.5));
    testcase.push_back(make_pair(900.0, 2.5));
    testcase.push_back(make_pair(975.0, 0.5));
    testcase.push_back(make_pair(903.0, 1.5));
    testcase.push_back(make_pair(950.0, 2.5));

    //testcase.push_back(make_pair(800.0, 0.5));
    //testcase.push_back(make_pair(800.0, 1.5));
    testcase.push_back(make_pair(800.0, 2.5));
   // testcase.push_back(make_pair(875.0, 0.5));
   // testcase.push_back(make_pair(803.0, 1.5));
    testcase.push_back(make_pair(850.0, 2.5));

    testcase.push_back(make_pair(700.0, 0.5));
    testcase.push_back(make_pair(700.0, 1.5));
    testcase.push_back(make_pair(700.0, 2.5));
    testcase.push_back(make_pair(775.0, 0.5));
    testcase.push_back(make_pair(703.0, 1.5));
    testcase.push_back(make_pair(750.0, 2.5));

    testcase.push_back(make_pair(600.0, 0.5));
    testcase.push_back(make_pair(600.0, 1.5));
    testcase.push_back(make_pair(600.0, 2.5));
    testcase.push_back(make_pair(675.0, 0.5));
    testcase.push_back(make_pair(603.0, 1.5));
    testcase.push_back(make_pair(650.0, 2.5));

    testcase.push_back(make_pair(500.0, 0.5));
    testcase.push_back(make_pair(500.0, 1.5));
    testcase.push_back(make_pair(500.0, 2.5));
    testcase.push_back(make_pair(575.0, 0.5));
    testcase.push_back(make_pair(503.0, 1.5));
    testcase.push_back(make_pair(550.0, 2.5));

    //testcase.push_back(make_pair(400.0, 0.5));
    //testcase.push_back(make_pair(400.0, 1.5));
    testcase.push_back(make_pair(400.0, 2.5));
    //testcase.push_back(make_pair(475.0, 0.5));
    //testcase.push_back(make_pair(403.0, 1.5));
    testcase.push_back(make_pair(450.0, 2.5));

    testcase.push_back(make_pair(300.0, 0.5));
    testcase.push_back(make_pair(300.0, 1.5));
    testcase.push_back(make_pair(300.0, 2.5));
    testcase.push_back(make_pair(375.0, 0.5));
    testcase.push_back(make_pair(303.0, 1.5));
    testcase.push_back(make_pair(350.0, 2.5));

    testcase.push_back(make_pair(200.0, 0.5));
    testcase.push_back(make_pair(200.0, 1.5));
    testcase.push_back(make_pair(200.0, 2.5));
    testcase.push_back(make_pair(275.0, 0.5));
    testcase.push_back(make_pair(203.0, 1.5));
    testcase.push_back(make_pair(250.0, 2.5));

    testcase.push_back(make_pair(100.0, 0.5));
    testcase.push_back(make_pair(100.0, 1.5));
    testcase.push_back(make_pair(100.0, 2.5));
    testcase.push_back(make_pair(175.0, 0.5));
    testcase.push_back(make_pair(103.0, 1.5));
    testcase.push_back(make_pair(150.0, 2.5));

	for (std::vector<pair<double, double> >::iterator iter(testcase.begin()); iter != testcase.end(); ++iter) {
		const nnetmodel &nmodel = recallNnetMap(nnetMap, iter->first, iter->second);
		cout << "Bank charge: " << nmodel.bank_vol << endl;
		cout << "Solar min: " << nmodel.solar_min << endl;
		cout << "Solar max: " << nmodel.solar_max << endl;
		cout << "Energy offset: " << nmodel.energy_offset << endl;
		cout << "model IW: " << nmodel.IW << endl;
		cout << "model LW: " << nmodel.LW << endl;
	}
	return 0;
}
#endif
