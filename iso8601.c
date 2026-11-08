#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "iso8601.h"

#define TMP_STRING_LEN 100

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

void PlatformGmtime(time_t ts, struct tm *tmout)
{
	#ifdef __unix__
		gmtime_r(&ts, tmout);
	#else
		#if defined(_WIN32) || defined(_WIN64)
		gmtime_s(tmout, &ts);
		#else
		#warning "Code is not thread safe because no safe alternative to gmtime was found"
		struct tm *tmp = gmtime(&ts);
		memcpy(tmout, tmp, sizeof(struct tm));
		#endif
	#endif
}

#if defined(_WIN32) || defined(_WIN64)
#define timegm _mkgmtime
#endif

void Normalize(struct tm *tmout)
{
	//Normalize format
	time_t ts = timegm (tmout);
	PlatformGmtime(ts, tmout);
}

void GetUtcTimeNow(struct tm *tmnow)
{
	time_t ts;
	time (&ts);
	PlatformGmtime(ts, tmnow);
}

void TmToUtc(struct tm *tmout, int timezoneMin)
{
	//Apply timezone to get UTC
	int h = timezoneMin / 60;
	tmout->tm_hour -= h;
	tmout->tm_min -= (timezoneMin - h*60);
	Normalize(tmout);
}

void CalcStartOfFirstWeek(int y, struct tm *sow)
{
	//Find start of week containing 4th Jan
	struct tm fourthjan;
	memset(&fourthjan,0x00,sizeof(struct tm));
	fourthjan.tm_year = y - 1900;
	fourthjan.tm_mday = 4;
	Normalize(&fourthjan);

	//Start of first week (the Monday before 4th Jan)
	*sow = fourthjan;
	if(fourthjan.tm_wday == 0)
		fourthjan.tm_wday = 7;
	sow->tm_mday = fourthjan.tm_mday - fourthjan.tm_wday + 1; //Jump to monday
	sow->tm_wday = 0;
	Normalize(sow);
}

static bool IsLeapYear(int y)
{
	return (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
}

static int DaysInMonth(int y, int m)
{
	static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if(m < 1 || m > 12)
		return 0;
	return days[m-1] + (m == 2 && IsLeapYear(y));
}

static int WeeksInIsoYear(int y)
{
	struct tm start, next;
	CalcStartOfFirstWeek(y, &start);
	CalcStartOfFirstWeek(y + 1, &next);
	return (int)((timegm(&next) - timegm(&start)) / (7 * 24 * 60 * 60));
}

static bool ValidTime(double h, double m, double s)
{
	if(!isfinite(h) || !isfinite(m) || !isfinite(s) ||
	   h < 0 || h > 24 || m < 0 || m >= 60 || s < 0 || s >= 60)
		return false;
	return h < 24 || (h <= 24 && !(m > 0 || s > 0));
}

//A leap second is second 60 of the last minute of a UTC day. Which days
//actually had one is not checked.
static bool ValidLeapSecond(int h, int m, double s, int tzh, int tzm)
{
	int utcMin;
	if(h >= 24 || m >= 60 || s < 60 || s >= 61)
		return false;
	utcMin = ((h*60 + m - (tzh*60 + tzm)) % 1440 + 1440) % 1440;
	return utcMin == 23*60 + 59;
}

static bool ValidTimeWithSeconds(int h, int m, double s, int tzh, int tzm)
{
	return ValidTime(h, m, s) || ValidLeapSecond(h, m, s, tzh, tzm);
}

static bool ParseDate(const char *str, struct tm *tmout, bool strict)
{
	int y=1900,M=1,w=1,d=1;
	int sl = strlen(str);

	//Format 1a Complete date
	if(sl==8 && MatchPattern(str, "dddddddd"))
	{
		y = 1900; M = 1; d = 1;
		int ret4 = sscanf(str, "%4d%2d%2d", &y, &M, &d);
		if(ret4 == 3)
		{
			if(strict && (M < 1 || M > 12 || d < 1 || d > DaysInMonth(y, M)))
				return false;
			tmout->tm_year = y - 1900;
			tmout->tm_mon = M - 1;
			tmout->tm_mday = d;
			Normalize(tmout);
			return true;
		}
	}

	//Format 1b Complete date, extended format
	if(sl == 10 && MatchPattern(str, "dddd-dd-dd"))
	{
		int ret = sscanf(str, "%4d-%2d-%2d", &y, &M, &d);
		if(ret == 3)
		{
			if(strict && (M < 1 || M > 12 || d < 1 || d > DaysInMonth(y, M)))
				return false;
			tmout->tm_year = y - 1900;
			tmout->tm_mon = M - 1;
			tmout->tm_mday = d;
			Normalize(tmout);
			return true;
		}
	}

	//Format 2 month and year
	if(sl == 7 && MatchPattern(str, "dddd-dd"))
	{
		y = 1900; M = 1;
		int ret5 = sscanf(str, "%4d-%2d", &y, &M);
		if(ret5 == 2)
		{
			if(strict && (M < 1 || M > 12))
				return false;
			tmout->tm_year = y - 1900;
			tmout->tm_mon = M - 1;
			tmout->tm_mday = 1;
			Normalize(tmout);
			return true;
		}
	}

	//Format 3a plus signed year
	if(sl == 5 && MatchPattern(str, "+dddd"))
	{
		y = 1900;
		int ret2 = sscanf(str, "%5d", &y);
		if(ret2 == 1)
		{
			tmout->tm_year = y - 1900;
			tmout->tm_mon = 0;
			tmout->tm_mday = 1;
			Normalize(tmout);
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
			tmout->tm_year = y - 1900;
			tmout->tm_mon = 0;
			tmout->tm_mday = 1;
			Normalize(tmout);
			return true;
		}
	}

	//Format 3c plain year
	if(sl == 4 && MatchPattern(str, "dddd"))
	{
		y = 1900;
		int ret3 = sscanf(str, "%4d", &y);
		if(ret3 == 1)
		{
			tmout->tm_year = y - 1900;
			tmout->tm_mon = 0;
			tmout->tm_mday = 1;
			Normalize(tmout);
			return true;
		}
	}

	//Format 4 century
	if(sl == 2 && MatchPattern(str, "dd"))
	{
		y = 1900;
		int ret3 = sscanf(str, "%2d", &y);
		if(ret3 == 1)
		{
			tmout->tm_year = y*100 - 1900;
			tmout->tm_mon = 0;
			tmout->tm_mday = 1;
			Normalize(tmout);
			return true;
		}
	}

#if 0
	//Format 5 date in current century, is this common enough to enable? it confuses some tests.
	if(sl == 6 && MatchPattern(str, "dddddd"))
	{
		y = 1900;
		int ret3 = sscanf(str, "%2d", &y);
		if(ret3 == 1)
		{
			struct tm tmnow;
			GetUtcTimeNow(tmnow);

			tmout->tm_year = (tmnow.tm_year/100)*100 + y - 1900;
			tmout->tm_mon = 0;
			tmout->tm_mday = 1;
			Normalize(tmout);
			return true;
		}
	}
#endif

	//Format 6a ordinal date with dashes
	if(sl==8 && MatchPattern(str, "dddd-ddd"))
	{
		y = 1900; d = 1;
		int ret4 = sscanf(str, "%4d-%3d", &y, &d);
		if(ret4 == 2)
		{
			if(strict && (d < 1 || d > (IsLeapYear(y) ? 366 : 365)))
				return false;
			tmout->tm_year = y - 1900;
			tmout->tm_mon = 0;
			tmout->tm_mday = d;
			Normalize(tmout);
			return true;
		}
	}

	//Format 6b ordinal date
	if(sl==7 && MatchPattern(str, "ddddddd"))
	{
		y = 1900; d = 1;
		int ret4 = sscanf(str, "%4d%3d", &y, &d);
		if(ret4 == 2)
		{
			if(strict && (d < 1 || d > (IsLeapYear(y) ? 366 : 365)))
				return false;
			tmout->tm_year = y - 1900;
			tmout->tm_mon = 0;
			tmout->tm_mday = d;
			Normalize(tmout);
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
			if(strict && (w < 1 || w > WeeksInIsoYear(y) || d < 1 || d > 7))
				return false;
			CalcStartOfFirstWeek(y, &sow);

			*tmout = sow;
			tmout->tm_mday += (d-1) + (w-1)*7;
			Normalize(tmout);
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
			if(strict && (w < 1 || w > WeeksInIsoYear(y) || d < 1 || d > 7))
				return false;
			CalcStartOfFirstWeek(y, &sow);

			*tmout = sow;
			tmout->tm_mday += (d-1) + (w-1)*7;
			Normalize(tmout);
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
			if(strict && (w < 1 || w > WeeksInIsoYear(y)))
				return false;
			CalcStartOfFirstWeek(y, &sow);

			*tmout = sow;
			tmout->tm_mday += (w-1)*7;
			Normalize(tmout);
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
			if(strict && (w < 1 || w > WeeksInIsoYear(y)))
				return false;
			CalcStartOfFirstWeek(y, &sow);

			*tmout = sow;
			tmout->tm_mday += (w-1)*7;
			Normalize(tmout);
			return true;
		}
	}

	return false;
}

static bool ParseTimezone(const char *str, int *h, int *m, bool strict)
{
	size_t len = strlen(str);
	char sign;
	int hh, mm = 0;
	if(strcmp(str, "Z") == 0)
	{
		*h = 0;
		*m = 0;
		return true;
	}
	if(len < 3 || (str[0] != '+' && str[0] != '-'))
		return false;
	sign = str[0];
	if(str[1] < '0' || str[1] > '9' || str[2] < '0' || str[2] > '9')
		return false;
	hh = (str[1] - '0') * 10 + str[2] - '0';
	if(len == 6 && str[3] == ':')
	{
		if(str[4] < '0' || str[4] > '9' || str[5] < '0' || str[5] > '9')
			return false;
		mm = (str[4] - '0') * 10 + str[5] - '0';
	}
	else if(len == 5)
	{
		if(str[3] < '0' || str[3] > '9' || str[4] < '0' || str[4] > '9')
			return false;
		mm = (str[3] - '0') * 10 + str[4] - '0';
	}
	else if(len != 3)
		return false;
	if(strict && (hh > 14 || mm > 59))
		return false;
	*h = sign == '-' ? -hh : hh;
	*m = sign == '-' ? -mm : mm;
	return true;
}

static bool ParseTime(const char *str, struct tm *tmout, int *timezoneOffsetMin, bool strict)
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
	char baseTime[TMP_STRING_LEN];
	if(firstTzChar != NULL)
	{
		int baseDateLen = firstTzChar - str;
		if(baseDateLen >= (int)sizeof(baseTime))
			return false; //Input too long
		strncpy(baseTime, str, baseDateLen);
		baseTime[baseDateLen] = '\0';
	}
	else
	{
		if(strlen(str) >= sizeof(baseTime))
			return false; //Input too long
		strncpy(baseTime, str, sizeof(baseTime)-1);
		baseTime[sizeof(baseTime)-1] = '\0';
	}
	int btl = strlen(baseTime);

	//Assume UTC if no timezone is specified
	int tzh = 0, tzm = 0;
	if(firstTzChar != NULL)
	{
		char tzStr[TMP_STRING_LEN];
		if(strlen(firstTzChar) >= sizeof(tzStr))
			return false; //Input too long
		strncpy(tzStr, firstTzChar, sizeof(tzStr)-1);
		tzStr[sizeof(tzStr)-1] = '\0';
		bool ok = ParseTimezone(tzStr, &tzh, &tzm, strict);
		if(!ok) return false;
		//cout << "tz " << tzh << "," << tzm << endl;
	}
	if(timezoneOffsetMin != NULL)
		*timezoneOffsetMin = (tzh*60) + tzm;

	//Format 1 full time
	if(btl >= 8 && (MatchPattern(baseTime, "dd:dd:f") || MatchPattern(baseTime, "dd:dd:dd")))
	{
		int ret = sscanf(baseTime, "%2d:%2d:%f%100s", &h, &m, &s, excess);
		if(ret == 3)
		{
			if(strict && !ValidTimeWithSeconds(h, m, s, tzh, tzm))
				return false;
			tmout->tm_hour = h;
			tmout->tm_min = m;
			tmout->tm_sec = round(s);
			//ApplyTimezone(tmout, tzh, tzm);
			Normalize(tmout);
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
			if(strict && !ValidTime(h, mf, 0))
				return false;
			tmout->tm_hour = h;
			tmout->tm_min = mf;
			tmout->tm_sec = round((mf - (int)(mf)) * 60.0);
			//ApplyTimezone(tmout, tzh, tzm);
			Normalize(tmout);
			return true;
		}
	}

	//Format 3 full time with no dashes
	if(btl >= 6 && (MatchPattern(baseTime, "ddddf") || MatchPattern(baseTime, "dddddd")))
	{
		h = 0; m = 0; s = 0.0f;
		int ret4 = sscanf(baseTime, "%2d%2d%f%100s", &h, &m, &s, excess);
		if(ret4 == 3)
		{
			if(strict && !ValidTimeWithSeconds(h, m, s, tzh, tzm))
				return false;
			tmout->tm_hour = h;
			tmout->tm_min = m;
			tmout->tm_sec = round(s);
			//ApplyTimezone(tmout, tzh, tzm);
			Normalize(tmout);
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
			if(strict && !ValidTime(h, mf, 0))
				return false;
			tmout->tm_hour = h;
			tmout->tm_min = (int)(mf);
			tmout->tm_sec = round((mf - tmout->tm_min)*60.0);
			//ApplyTimezone(tmout, tzh, tzm);
			Normalize(tmout);
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
			if(strict && !ValidTime(hf, 0, 0))
				return false;
			tmout->tm_hour = (int)(hf);
			float mins = (hf - (float)tmout->tm_hour) * 60.0f;
			tmout->tm_min = (int)mins;
			tmout->tm_sec = round((mins - (float)tmout->tm_min) * 60.0f);
			//ApplyTimezone(tmout, tzh, tzm);
			Normalize(tmout);
			return true;
		}
	}

	return false;
}

static bool ParseDatetime(const char *str, struct tm *tmout, int *timezoneOffsetMin, bool strict)
{
	//cout << str << endl;
	memset(tmout,0x00,sizeof(struct tm));
	tmout->tm_isdst = 0;
	if(timezoneOffsetMin != NULL)
		*timezoneOffsetMin = 0;

	const char *tChar = strchr (str, 'T');	
	bool ok = true;
	if(tChar != NULL)
	{
		char dateStr[TMP_STRING_LEN];
		int dateLen = tChar - str;
		if(dateLen >= (int)sizeof(dateStr))
			return false; //Input too long
		strncpy(dateStr, str, dateLen);
		dateStr[dateLen] = '\0';

		int timeLen = strlen(str) - dateLen - 1;
		char timeStr[TMP_STRING_LEN];
		if(timeLen >= (int)sizeof(timeStr))
			return false; //Input too long
		strncpy(timeStr, tChar+1, timeLen);
		timeStr[timeLen] = '\0';

		ok = ParseDate(dateStr, tmout, strict);
		if(!ok) return false;
		ok = ParseTime(timeStr, tmout, timezoneOffsetMin, strict);
		if(!ok) return false;
	}
	else
	{
		ok = ParseDate(str, tmout, strict); //Only the date is specified
		if(!ok) return false;
	}

	Normalize(tmout);

	return ok;
}

bool ParseIso8601Timezone(const char *str, int *h, int *m)
{
	return ParseTimezone(str, h, m, false);
}

bool ParseIso8601Date(const char *str, struct tm *tmout)
{
	return ParseDate(str, tmout, false);
}

bool ParseIso8601DateStrict(const char *str, struct tm *tmout)
{
	return ParseDate(str, tmout, true);
}

bool ParseIso8601Time(const char *str, struct tm *tmout, int *timezoneOffsetMin)
{
	return ParseTime(str, tmout, timezoneOffsetMin, false);
}

bool ParseIso8601TimeStrict(const char *str, struct tm *tmout, int *timezoneOffsetMin)
{
	return ParseTime(str, tmout, timezoneOffsetMin, true);
}

bool ParseIso8601Datetime(const char *str, struct tm *tmout, int *timezoneOffsetMin)
{
	return ParseDatetime(str, tmout, timezoneOffsetMin, false);
}

bool ParseIso8601DatetimeStrict(const char *str, struct tm *tmout, int *timezoneOffsetMin)
{
	return ParseDatetime(str, tmout, timezoneOffsetMin, true);
}
