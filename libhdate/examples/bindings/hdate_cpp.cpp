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
	cout << "Today is : " << h.get_gday () << ",";
	cout << h.get_gmonth () << ",";
	cout << h.get_gyear () << "\n";

	// print hebrew date
	cout << h.get_format_date (0, 0);
	cout << "\n";

	return 0;
}
