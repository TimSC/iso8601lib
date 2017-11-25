#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "iso8601.h"
using namespace std;

//This string pattern matcher is stricter than what scanf provides
bool MatchPattern2(const char *str, const char *pattern)
{
	size_t pc=0, pl=strlen(pattern), sc=0, sl=strlen(str);
	while (pc<pl && sc<sl)
	{
		char code = pattern[pc];
		char ch = str[sc];
		if(code == 'd') //Digit
		{
			if(ch < '0' || ch > '9')
			{
				return false;
			}
			pc ++;
			sc ++;
			continue;
		}
		if(code == 'f') //2 digit padded float (decimal point is required)
		{
			if(pc>=pl || sc>=sl || ch < '0' || ch > '9')
				return false;
			sc ++;
			ch = str[sc];
			if(pc>=pl || sc>=sl || ch < '0' || ch > '9')
				return false;
			sc ++;
			ch = str[sc];
			if(pc>=pl || sc>=sl || ch != '.')
				return false;
			sc ++;
			ch = str[sc];
			while(ch >= '0' && ch <= '9')
			{
				sc++;
				if(sc >= sl)
					break;
				ch = str[sc];				
			}

			pc ++;
			continue;
		}
		if(code == ':' || code == '-' || code == '+' || code == 'W') //Literal character
		{
			if(ch != code)
				return false;
			sc ++;
			pc ++;
			continue;
		}
		return false;
	}
	if(sc != sl || pc != pl) //All characters must match pattern
		return false; 

	return true;
}

bool MatchPattern(const char *str, const char *pattern)
{
	//cout << "a " << str << endl;
	bool ret = MatchPattern2(str, pattern);
	//cout << "result " << ret << endl;
	return ret;
}

void ApplyTimezoneNormalize(struct tm &tmout, bool normalize, int tzh, int tzm)
{
	//Apply timezone to get UTC
	tmout.tm_hour -= tzh;
	tmout.tm_min -= tzm;

	if(normalize)
	{
		//Normalize format
		time_t ts = mktime (&tmout);

		#ifdef __unix
		struct tm tmp;
		gmtime_r(&ts, &tmp);
		memcpy(&tmout, &tmp, sizeof(struct tm));
		#else
		#if defined(_WIN32) || defined(_WIN64)
		struct tm tmp;
		gmtime_s(&ts, &tmp);
		memcpy(&tmout, &tmp, sizeof(struct tm));
		#else
		#warning "Code is not thread safe because no alternative to gmtime was found"
		struct tm *tmp = gmtime(&ts);
		memcpy(&tmout, tmp, sizeof(struct tm));
		#endif
		#endif
	}
}

void CalcStartOfFirstWeek(int y, struct tm *sow)
{
	//Find start of week containing 4th Jan
	struct tm fourthjan;
	memset(&fourthjan,0x00,sizeof(struct tm));
	fourthjan.tm_year = y - 1900;
	fourthjan.tm_mday = 4;
	ApplyTimezoneNormalize(fourthjan, true, 0, 0);

	//Start of first week (the Monday before 4th Jan)
	*sow = fourthjan;
	if(fourthjan.tm_wday == 0)
		fourthjan.tm_wday = 7;
	sow->tm_mday = fourthjan.tm_mday - fourthjan.tm_wday + 1; //Jump to monday
	sow->tm_wday = 0;
	ApplyTimezoneNormalize(*sow, true, 0, 0);
}

bool ParseIso8601Date(const char *str, struct tm &tmout, bool normalize)
{
	int y=1900,M=1,w=1,d=1;
	int sl = strlen(str);

	//A better idea would be to put the shorter formats first, so they can be replaced by 
	//more complete representations.

	//Format 1 conventional date with dashes
	if(sl == 10 && MatchPattern(str, "dddd-dd-dd"))
	{
		int ret = sscanf(str, "%4d-%2d-%2d", &y, &M, &d);
		if(ret == 3 && y >= 0 && M >= 0 && d >= 0)
		{
			tmout.tm_year = y - 1900;
			tmout.tm_mon = M - 1;
			tmout.tm_mday = d;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 2 plus signed year
	if(sl == 5 && MatchPattern(str, "+dddd"))
	{
		y = 1900;
		int ret2 = sscanf(str, "%5d", &y);
		if(ret2 == 1)
		{
			tmout.tm_year = y - 1900;
			tmout.tm_mon = 0;
			tmout.tm_mday = 1;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 2b minus signed year
	if(sl == 5 && MatchPattern(str, "-dddd"))
	{
		y = 100;
		int ret2b = sscanf(str, "%5d", &y);
		if(ret2b == 1)
		{
			tmout.tm_year = - y - 1900;
			tmout.tm_mon = 0;
			tmout.tm_mday = 1;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 3 plain year
	if(sl == 4 && MatchPattern(str, "dddd"))
	{
		y = 1900;
		int ret3 = sscanf(str, "%4d", &y);
		if(ret3 == 1 && y >= 0)
		{
			tmout.tm_year = y - 1900;
			tmout.tm_mon = 0;
			tmout.tm_mday = 1;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 4 plain year and month
	if(sl == 7 && MatchPattern(str, "dddd-dd"))
	{
		y = 1900; M = 1;
		int ret5 = sscanf(str, "%4d-%2d", &y, &M);
		if(ret5 == 2 && y >= 0 && M >= 0)
		{
			tmout.tm_year = y - 1900;
			tmout.tm_mon = M - 1;
			tmout.tm_mday = 1;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 5 full date with no dashes
	if(sl==8 && MatchPattern(str, "dddddddd"))
	{
		y = 1900; M = 1; d = 1;
		int ret4 = sscanf(str, "%4d%2d%2d", &y, &M, &d);
		if(ret4 == 3 && y >= 0 && M >= 0 && d >= 0)
		{
			tmout.tm_year = y - 1900;
			tmout.tm_mon = M - 1;
			tmout.tm_mday = d;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 6a ordinal date with dashes
	if(sl==8 && MatchPattern(str, "dddd-ddd"))
	{
		y = 1900; d = 1;
		int ret4 = sscanf(str, "%4d-%3d", &y, &d);
		if(ret4 == 2 && y >= 0 && d >= 0)
		{
			tmout.tm_year = y - 1900;
			tmout.tm_mday = d;
			if(d <= 0 || d > 366)
				return false;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 6b ordinal date
	if(sl==7 && MatchPattern(str, "ddddddd"))
	{
		y = 1900; d = 1;
		int ret4 = sscanf(str, "%4d%3d", &y, &d);
		if(ret4 == 2 && y >= 0 && d >= 0)
		{
			tmout.tm_year = y - 1900;
			tmout.tm_mday = d;
			if(d <= 0 || d > 366)
				return false;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 7a ISO week day
	if(sl==10 && MatchPattern(str, "dddd-Wdd-d"))
	{
		y = 1900; w = 1; d = 1;
		int ret4 = sscanf(str, "%4d-W%2d-%1d", &y, &w, &d);
		if(ret4 == 3)
		{
			struct tm sow;
			CalcStartOfFirstWeek(y, &sow);

			tmout = sow;
			tmout.tm_mday += (d-1) + (w-1)*7;
			if(d <= 0 || d > 7)
				return false;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 7a ISO week day, no dashes
	if(sl==8 && MatchPattern(str, "ddddWddd"))
	{
		y = 1900; w = 1; d = 1;
		int ret4 = sscanf(str, "%4dW%2d%1d", &y, &w, &d);
		if(ret4 == 3)
		{
			struct tm sow;
			CalcStartOfFirstWeek(y, &sow);

			tmout = sow;
			tmout.tm_mday += (d-1) + (w-1)*7;
			if(d <= 0 || d > 7)
				return false;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 8a ISO week
	if(sl==8 && MatchPattern(str, "dddd-Wdd"))
	{
		y = 1900; w = 1;
		int ret4 = sscanf(str, "%4d-W%2d", &y, &w);
		if(ret4 == 2)
		{
			struct tm sow;
			CalcStartOfFirstWeek(y, &sow);

			tmout = sow;
			tmout.tm_mday += (w-1)*7;
			if(d <= 0 || d > 7)
				return false;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
	}

	//Format 8b ISO week, no dash
	if(sl==7 && MatchPattern(str, "ddddWdd"))
	{
		y = 1900; w = 1;
		int ret4 = sscanf(str, "%4dW%2d", &y, &w);
		if(ret4 == 2)
		{
			struct tm sow;
			CalcStartOfFirstWeek(y, &sow);

			tmout = sow;
			tmout.tm_mday += (w-1)*7;
			if(d <= 0 || d > 7)
				return false;
			ApplyTimezoneNormalize(tmout, normalize, 0, 0);
			return true;
		}
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
	int h=0,m=0;
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
	char baseTime[100];
	if(firstTzChar != NULL)
	{
		int baseDateLen = firstTzChar - str;
		if(baseDateLen >= 100)
			return false; //Input too long
		strncpy(baseTime, str, baseDateLen);
		baseTime[baseDateLen] = '\0';
	}
	else
		strncpy(baseTime, str, 99);
	int btl = strlen(baseTime);

	//Assume UTC if no timezone is specified
	int tzh = 0, tzm = 0;
	if(firstTzChar != NULL)
	{
		char tzStr[100];
		strncpy(tzStr, firstTzChar, 99);
		bool ok = ParseIso8601Timezone(tzStr, tzh, tzm);
		if(!ok) return false;
		//cout << "tz " << tzh << "," << tzm << endl;
	}
	
	//Format 1 full time
	if(btl >= 8 && (MatchPattern(baseTime, "dd:dd:f") || MatchPattern(baseTime, "dd:dd:dd")))
	{
		int ret = sscanf(baseTime, "%2d:%2d:%f%100s", &h, &m, &s, excess);
		if(ret == 3)
		{
			tmout.tm_hour = h;
			tmout.tm_min = m;
			tmout.tm_sec = round(s);
			ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
			return true;
		}
	}

	//Format 2 hours and minutes
	if(btl >= 5 && (MatchPattern(baseTime, "dd:f") || MatchPattern(baseTime, "dd:dd")))
	{
		h = 0; mf = 0.0f;
		int ret2 = sscanf(baseTime, "%2d:%f%100s", &h, &mf, excess);
		if(ret2 == 2)
		{
			tmout.tm_hour = h;
			tmout.tm_min = mf;
			tmout.tm_sec = round((mf - int(mf)) * 60.0);
			if(h == 24 && mf != 0)
				return false; //Invalid time
			ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
			return true;
		}
	}

	//Format 3 full time with no dashes
	if(btl >= 6 && (MatchPattern(baseTime, "ddddf") || MatchPattern(baseTime, "dddddd")))
	{
		h = 0; m = 0; s = 0.0f;
		int ret4 = sscanf(baseTime, "%2d%2d%f%s", &h, &m, &s, excess);
		if(ret4 == 3)
		{
			tmout.tm_hour = h;
			tmout.tm_min = m;
			tmout.tm_sec = round(s);
			ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
			return true;
		}
	}

	//Format 4 hours and minutes, with no dashes
	if(btl >= 4 && (MatchPattern(baseTime, "ddf") || MatchPattern(baseTime, "dddd")))
	{
		h = 0; mf = 0.0f;
		int ret5 = sscanf(baseTime, "%2d%f%s", &h, &mf, excess);
		if(ret5 == 2)
		{
			tmout.tm_hour = h;
			tmout.tm_min = int(mf);
			tmout.tm_sec = round((mf - tmout.tm_min)*60.0);
			ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
			return true;
		}
	}

	//Format 5 hours
	if(btl >= 2 && (MatchPattern(baseTime, "f") || MatchPattern(baseTime, "dd")))
	{
		hf = 0.0f;
		int ret3 = sscanf(baseTime, "%f%100s", &hf, excess);
		if(ret3 == 1)
		{
			tmout.tm_hour = int(hf);
			float mins = (hf - (float)tmout.tm_hour) * 60.0f;
			tmout.tm_min = (int)mins;
			tmout.tm_sec = round((mins - (float)tmout.tm_min) * 60.0f);
			ApplyTimezoneNormalize(tmout, normalize, tzh, tzm);
			return true;
		}
	}

	return false;
}

bool ParseIso8601Datetime(const char *str, struct tm &tmout, bool normalize)
{
	//cout << str << endl;
	memset(&tmout,0x00,sizeof(tmout));
	tmout.tm_isdst = 0;

	const char *tChar = strchr (str, 'T');	
	bool ok = true;
	if(tChar != NULL)
	{
		int dateLen = tChar - str;
		if(dateLen >= 100)
			return false; //Input too long
		char dateStr[100];
		strncpy(dateStr, str, dateLen);
		dateStr[dateLen] = '\0';

		int timeLen = strlen(str) - dateLen - 1;
		if(timeLen >= 100)
			return false; //Input too long
		char timeStr[100];
		strncpy(timeStr, tChar+1, timeLen);
		timeStr[timeLen] = '\0';

		//printf("split %s, %s\n", dateStr, timeStr);

		ok = ParseIso8601Date(dateStr, tmout, false);
		if(!ok) return false;
		ok = ParseIso8601Time(timeStr, tmout, false);
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

