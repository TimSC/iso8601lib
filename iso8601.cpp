#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <cmath>
#include "iso8601.h"
using namespace std;

void ApplyTimezoneNormalize(struct tm &tmout, bool normalize, int tzh, int tzm)
{
	//Apply timezone to get UTC
	tmout.tm_hour -= tzh;
	tmout.tm_min -= tzm;

	if(normalize)
	{
		//Normalize format
		time_t ts = mktime (&tmout);
		struct tm *tmp = gmtime(&ts);
		memcpy(&tmout, tmp, sizeof(struct tm));
	}
}

bool ParseIso8601Date(const char *str, struct tm &tmout, bool normalize)
{
	int y=1900,M=1,d=1;
	char excess[101];

	//A better idea would be to put the shorter formats first, so they can be replaced by 
	//more complete representations.

	//Format 1 conventional date with dashes
	int ret = sscanf(str, "%4d-%2d-%2d%100s", &y, &M, &d, excess);
	if(ret == 3 && y >= 0 && M >= 0 && d >= 0)
	{
		tmout.tm_year = y - 1900;
		tmout.tm_mon = M - 1;
		tmout.tm_mday = d;
		ApplyTimezoneNormalize(tmout, normalize, 0, 0);
		return true;
	}

	//Format 2 plus signed year
	y = 1900;
	int ret2 = sscanf(str, "+%4d%100s", &y, excess);
	if(ret2 == 1 && y >= 0)
	{
		tmout.tm_year = y - 1900;
		tmout.tm_mon = 0;
		tmout.tm_mday = 1;
		ApplyTimezoneNormalize(tmout, normalize, 0, 0);
		return true;
	}

	//Format 2b minus signed year
	y = 100;
	int ret2b = sscanf(str, "-%4d%100s", &y, excess);
	if(ret2b == 1 && y >= 0)
	{
		tmout.tm_year = - y - 1900;
		tmout.tm_mon = 0;
		tmout.tm_mday = 1;
		ApplyTimezoneNormalize(tmout, normalize, 0, 0);
		return true;
	}

	//Format 3 plain year
	y = 1900;
	int ret3 = sscanf(str, "%4d%100s", &y, excess);
	if(ret3 == 1 && y >= 0)
	{
		tmout.tm_year = y - 1900;
		tmout.tm_mon = 0;
		tmout.tm_mday = 1;
		ApplyTimezoneNormalize(tmout, normalize, 0, 0);
		return true;
	}

	//Format 4 full date with no dashes
	y = 1900; M = 1; d = 1;
	int ret4 = sscanf(str, "%4d%2d%2d%100s", &y, &M, &d, excess);
	if(ret4 == 3 && y >= 0 && M >= 0 && d >= 0)
	{
		tmout.tm_year = y - 1900;
		tmout.tm_mon = M - 1;
		tmout.tm_mday = d;
		ApplyTimezoneNormalize(tmout, normalize, 0, 0);
		return true;
	}

	//Format 5 plain year and month
	y = 1900; M = 1;
	int ret5 = sscanf(str, "%4d-%2d%100s", &y, &M, excess);
	if(ret5 == 2 && y >= 0 && M >= 0)
	{
		tmout.tm_year = y - 1900;
		tmout.tm_mon = M - 1;
		tmout.tm_mday = 1;
		ApplyTimezoneNormalize(tmout, normalize, 0, 0);
		return true;
	}

	return false;
}

bool ParseIso8601Timezone(const char *str, int &h, int &m)
{
	h = 0;
	m = 0;

	//Format 1 zulu time
	if(strcmp(str, "Z") == 0)
		return true;

	//Format 2
	char sign;
	int hv=0, mv=0;
	int ret = sscanf(str, "%c%2d%2d", &sign, &h, &m);
	if(ret < 1)
		h = 0;
	if(ret < 2)
		m = 0;

	//Format 3
	int ret2 = sscanf(str, "%c%2d:%2d", &sign, &hv, &mv);
	if(ret2 > ret)
	{
		h = hv;
		if(ret2 >= 2)
			m = mv;
	}

	if(ret == 0 && ret2 == 0)
		return false;

	if(sign == '-')
		h = -h;
	if(h < 0)
		m = -m;
	return true;
}

bool ParseIso8601Time(const char *str, struct tm &tmout, bool normalize)
{
	int h=0,h2=0,m=0,si=0;
	float s=0.0f, mf=0.0f, hf=0.0f;
	char excess[101];

	const char *zChar = strchr (str, 'Z');	
	const char *plusChar = strchr (str, '+');	
	const char *minusChar = strchr (str, '-');	

	const char *firstTzChar = zChar;
	if(plusChar != NULL && (firstTzChar == NULL || plusChar < firstTzChar))
		firstTzChar = plusChar;
	if(minusChar != NULL && (firstTzChar == NULL || minusChar < firstTzChar))
		firstTzChar = minusChar;

	//Get time with no timezone info
	string baseTime(str);
	if(firstTzChar != NULL)
	{
		int baseDateLen = firstTzChar - str;
		baseTime.assign(str, baseDateLen);
	}

	//Assume UTC if no timezone is specified
	int tzh = 0, tzm = 0;
	if(firstTzChar != NULL)
	{
		string tzStr(firstTzChar);
		bool ok = ParseIso8601Timezone(tzStr.c_str(), tzh, tzm);
		if(!ok) return false;
		//cout << "tz " << tzh << "," << tzm << endl;
	}
	
	//Format 1 full time
	int ret = sscanf(baseTime.c_str(), "%2d:%2d:%f%100s", &h, &m, &s, excess);
	if(ret == 3 && h >= 0 && m >= 0 && s >= 0.0f)
	{
		tmout.tm_hour = h;
		tmout.tm_min = m;
		tmout.tm_sec = round(s);
		ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
		return true;
	}

	//Format 2 hours and minutes
	h = 0; mf = 0.0f;
	int ret2 = sscanf(baseTime.c_str(), "%2d:%f%100s", &h, &mf, excess);
	if(ret2 == 2 && h >= 0 && mf >= 0)
	{
		tmout.tm_hour = h;
		tmout.tm_min = mf;
		tmout.tm_sec = round((mf - int(mf)) * 60.0);
		ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
		return true;
	}

	//Format 3 hours
	hf = 0.0f;
	int ret3 = sscanf(baseTime.c_str(), "%f%100s", &hf, excess);
	if(ret3 == 2 && h >= 0 && h2 >= 0)
	{
		tmout.tm_hour = int(hf);
		float mins = (hf - tmout.tm_hour) * 60.0f;
		tmout.tm_min = (int)mins;
		tmout.tm_sec = round((mins - (float)tmout.tm_min) * 60.0f);
		ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
		return true;
	}

	//Format 4 full time with no dashes
	h = 0; m = 0; s = 0.0f;
	int ret4 = sscanf(baseTime.c_str(), "%2d%2d%f%s", &h, &m, &s, excess);
	if(ret4 == 3 && h >= 0 && m >= 0 && s >= 0.0f)
	{
		tmout.tm_hour = h;
		tmout.tm_min = m;
		tmout.tm_sec = round(si);
		ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
		return true;
	}

	//Format 5 hours and minutes, with no dashes
	h = 0; mf = 0.0f;
	int ret5 = sscanf(baseTime.c_str(), "%2d%f%s", &h, &mf, excess);
	if(ret5 == 2 && h >= 0 && mf >= 0.0f)
	{
		tmout.tm_hour = h;
		tmout.tm_min = int(mf);
		tmout.tm_sec = round((mf - tmout.tm_min)*60.0);
		ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
		return true;
	}

	return false;
}

bool ParseIso8601Datetime(const char *str, struct tm &tmout, bool normalize)
{
	//cout << str << endl;
	memset(&tmout,0x00,sizeof(tmout));
	tmout.tm_isdst = -1;

	const char *tChar = strchr (str, 'T');	
	bool ok = true;
	if(tChar != NULL)
	{
		int dateLen = tChar - str;
		string dateStr(str, dateLen);

		int timeLen = strlen(str) - dateLen - 1;
		string timeStr(tChar+1, timeLen);
		
		//cout << dateStr << "," << timeStr << endl;

		ok = ParseIso8601Date(dateStr.c_str(), tmout, false);
		if(!ok) return false;
		ok = ParseIso8601Time(timeStr.c_str(), tmout, false);
		if(!ok) return false;
	}
	else
	{
		ok = ParseIso8601Date(str, tmout, false); //Only the date is specified
		if(!ok) return false;
	}

	if(normalize)
	{
		//Normalize format
		time_t ts = mktime (&tmout);
		struct tm *tmp = gmtime(&ts);
		memcpy(&tmout, tmp, sizeof(struct tm));
	}

	return ok;
}

void Iso8601TestCases(std::vector<std::string> &testStrs)
{
	testStrs.clear();

	testStrs.push_back("2017-09-11");
	testStrs.push_back("2017-09-11T21:52:13+00:00");
	testStrs.push_back("2017-09-11T21:52:13Z");
	testStrs.push_back("20170911T215213Z");
	testStrs.push_back("2009-12T12:34");

	//Subset of test cases from https://www.myintervals.com/blog/2009/05/20/iso-8601-date-validation-that-doesnt-suck/
	testStrs.push_back("2009");
	testStrs.push_back("2009-05-19");
	testStrs.push_back("20090519");
	testStrs.push_back("2009-05");
	testStrs.push_back("2009-05-19T14:39Z");
	testStrs.push_back("20090621T0545Z");
	testStrs.push_back("2007-04-06T00:00");
	testStrs.push_back("2007-04-05T24:00");
	testStrs.push_back("2010-02-18T16:23:48.5");
	testStrs.push_back("2010-02-18T16:23.4");
	testStrs.push_back("2010-02-18T16:23.33+0600");
	testStrs.push_back("2010-02-18T16:23.33-0530");
	testStrs.push_back("2010-02-18T16.23334444");

	//Long number after remainder? Not really supported (yet)
	//testStrs.push_back("2010-02-18T16.2233344445555");
	//testStrs.push_back("2010-02-18T16.22333444455555666666");
}

