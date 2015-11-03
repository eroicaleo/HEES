#include <algorithm>
#include <iostream>
#include <functional>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::transform;
using std::mem_fun_ref;

// http://stackoverflow.com/questions/1257721/can-i-use-a-mask-to-iterate-files-in-a-directory-with-boost

int main(int argc, char *argv[]) {

	// To avoid the run time error:
	// terminate called after throwing an instance of 'std::runtime_error'
	// what():  locale::facet::_S_create_c_locale name not valid
	// https://svn.boost.org/trac/boost/ticket/5928

	path model_dir(string("./data/PredictionModel/"));
	if (is_directory(model_dir)) {
		vector<directory_entry> v_entry;
		vector<path> v;

		// http://stackoverflow.com/questions/458525/iterating-over-vector-and-calling-functions
		// http://www.cplusplus.com/reference/functional/mem_fun_ref/
		copy(directory_iterator(model_dir), directory_iterator(), back_inserter(v_entry));
		transform(v_entry.begin(), v_entry.end(), back_inserter(v), mem_fun_ref(&directory_entry::path));

		for (vector<path>::const_iterator it(v.begin()); it != v.end(); ++it) {
			cout << *it << endl;
		}
	}

	return 0;
}
