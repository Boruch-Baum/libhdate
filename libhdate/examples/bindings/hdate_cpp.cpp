// hdate_cpp.cpp
// example file for libhdate.
//

//
// compile: g++ -lm -lhdate -o hdate_cpp hdate_cpp.cpp
//

#include <iostream>
#include <hdatepp.h>

using namespace std;
using namespace hdate;

int
main (int argc, char* argv[])
{
	// create a new Hdate object
	Hdate h;

	// print gregorian date
	cout << "Today is :\n";
	cout << h.get_gday () << "," << h.get_gmonth () << "," << h.get_gyear () << "\n";
	
	// print hebrew date: 0 - long format
	cout << h.get_format_date (0);
	cout << "\n";

	return 0;
}
