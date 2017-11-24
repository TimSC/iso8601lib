#include "iso8601.h"
#include <iostream>
#include <time.h>
#include <assert.h>
using namespace std;

int main()
{

	std::vector<std::string> testStrs;
	std::vector<int64_t> testTimestamps;
	Iso8601TestCases(testStrs, testTimestamps);
	assert(testStrs.size() == testTimestamps.size());
	
	for(size_t i=0; i<testStrs.size(); i++)
	{
		struct tm dt;
		ParseIso8601Datetime(testStrs[i].c_str(), dt);

		time_t ts = mktime (&dt);
		cout << "Test case: " << testStrs[i] << endl; 
		cout << ctime(&ts);// UTC date+time string
		cout << (int)ts << "=" << testTimestamps[i]; //UTC unix time-stamp
		if((int)ts != testTimestamps[i]) cout << " FAIL";
		cout << endl;
	}
}

