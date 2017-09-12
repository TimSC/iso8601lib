#include "iso8601.h"
#include <iostream>
#include <time.h>
using namespace std;

int main()
{

	std::vector<std::string> testStrs;
	Iso8601TestCases(testStrs);
	
	for(size_t i=0; i<testStrs.size(); i++)
	{
		struct tm dt;
		ParseIso8601Datetime(testStrs[i].c_str(), dt);

		time_t ts = mktime (&dt);
		cout << "Test case: " << testStrs[i] << endl;
		cout << ctime(&ts) << (int)ts << endl; //UTC date+time string and unix time-stamp
	}
}

