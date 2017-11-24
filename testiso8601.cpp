#include "iso8601.h"
#include <iostream>
#include <time.h>
#include <cmath>
#include <assert.h>
using namespace std;

void Iso8601TestCases(std::vector<std::string> &testStrs, std::vector<double> &testTimestamps)
{
	testStrs.clear();
	testTimestamps.clear();

	testStrs.push_back("2017-09-11"); //datetime.datetime(2017, 9, 11, 0, 0)
	testTimestamps.push_back(1505088000);
	
	testStrs.push_back("2017-09-11T21:52:13+00:00"); // datetime.datetime(2017, 9, 11, 21, 52, 13)
	testTimestamps.push_back(1505166733);

	testStrs.push_back("2017-09-11T21:52:13Z"); // datetime.datetime(2017, 9, 11, 21, 52, 13, tzinfo=tzutc())
	testTimestamps.push_back(1505166733);

	testStrs.push_back("20170911T215213Z"); // datetime.datetime(2017, 9, 11, 21, 52, 13)
	testTimestamps.push_back(1505166733);

	testStrs.push_back("2009-12T12:34"); // datetime.datetime(2009, 12, 24, 12, 34)
	testTimestamps.push_back(1259670840);

	//Subset of test cases from https://www.myintervals.com/blog/2009/05/20/iso-8601-date-validation-that-doesnt-suck/
	testStrs.push_back("2009"); // datetime.datetime(2009, 11, 24, 0, 0)
	testTimestamps.push_back(1230768000);

	testStrs.push_back("2009-05-19"); //datetime.datetime(2009, 5, 19, 0, 0)
	testTimestamps.push_back(1242691200);

	testStrs.push_back("20090519"); // datetime.datetime(2009, 5, 19, 0, 0)
	testTimestamps.push_back(1242691200);

	testStrs.push_back("2009-05"); // datetime.datetime(2009, 5, 24, 0, 0) really?
	testTimestamps.push_back(1241136000);

	testStrs.push_back("2009-05-19T14:39Z"); // datetime.datetime(2009, 5, 19, 14, 39)
	testTimestamps.push_back(1242743940);

	testStrs.push_back("20090621T0545Z"); // datetime.datetime(2009, 6, 21, 5, 45)
	testTimestamps.push_back(1245563100);

	testStrs.push_back("2007-04-06T00:00"); // datetime.datetime(2007, 4, 6, 0, 0)
	testTimestamps.push_back(1175817600);

	testStrs.push_back("2007-04-05T24:00"); // ??
	testTimestamps.push_back(1175817600);

	testStrs.push_back("2010-02-18T16:23:48.5"); // datetime.datetime(2010, 2, 18, 16, 23, 48, 500000)
	testTimestamps.push_back(1266510228.5);

	testStrs.push_back("2010-02-18T16:23.4"); // datetime.datetime(2010, 2, 18, 16, 23, 23)
	testTimestamps.push_back(1266510204);

	//testStrs.push_back("2010-02-18T16:23.33+0600"); // datetime.datetime(2010, 2, 18, 16, 23, 19, tzinfo=tzoffset(None, 21600))
	//testTimestamps.push_back();

	//testStrs.push_back("2010-02-18T16:23.33-0530"); // datetime.datetime(2010, 2, 18, 16, 23, 19, tzinfo=tzoffset(None, -19800))
	//testTimestamps.push_back();

	testStrs.push_back("2010-02-18T16.23334444"); // ??
	testTimestamps.push_back(1266509640);

	testStrs.push_back("2010-02-18T16.2233344445555");
	testTimestamps.push_back(1266509604.0040004);

	testStrs.push_back("2010-02-18T16.22333444455555666666");
	testTimestamps.push_back(1266509604.0040004);
}

int main()
{
	std::vector<std::string> testStrs;
	std::vector<double> testTimestamps;
	Iso8601TestCases(testStrs, testTimestamps);
	assert(testStrs.size() == testTimestamps.size());
	
	for(size_t i=0; i<testStrs.size(); i++)
	{
		struct tm dt;
		bool ok = ParseIso8601Datetime(testStrs[i].c_str(), dt);
		if(!ok)
		{
			cout << "Parse FAIL" << endl;
			continue;
		}

		time_t ts = mktime (&dt);
		cout << "Test case: " << testStrs[i] << endl;
		//cout << 1900+dt.tm_year << "," << 1+dt.tm_mon << "," << dt.tm_mday << "," << dt.tm_hour << "," << dt.tm_min << "," << dt.tm_sec << endl;

		cout << ctime(&ts);// UTC date+time string
		cout << (int64_t)ts << "=" << (int64_t)round(testTimestamps[i]); //UTC unix time-stamp
		if(fabs((int64_t)ts - round(testTimestamps[i]))>1e-6) cout << " FAIL";
		cout << endl;
	}
}

