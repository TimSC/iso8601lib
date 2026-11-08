//libFuzzer harness for iso8601lib. Build and run with: make fuzz
//
//Checks that no input crashes or triggers AddressSanitizer or
//UndefinedBehaviorSanitizer, and that strict and permissive parsing agree:
//anything strict parsing accepts must be accepted by permissive parsing with
//the same UTC time.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "iso8601.h"

#if defined(_WIN32) || defined(_WIN64)
#define timegm _mkgmtime
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char *str = (char *)malloc(size + 1);
	if(str == NULL)
		return 0;
	memcpy(str, data, size);
	str[size] = '\0';

	struct tm permissive, strict;
	int permissiveTz = 0, strictTz = 0;
	bool permissiveOk = ParseIso8601Datetime(str, &permissive, &permissiveTz);
	bool strictOk = ParseIso8601DatetimeStrict(str, &strict, &strictTz);
	if(strictOk)
	{
		if(!permissiveOk)
			abort(); //Strict accepted input that permissive rejected
		TmToUtc(&permissive, permissiveTz);
		TmToUtc(&strict, strictTz);
		if(timegm(&permissive) != timegm(&strict))
			abort(); //Strict and permissive disagree on a valid input
	}

	//Exercise the date and time entry points directly too
	struct tm tmp;
	int tz = 0;
	memset(&tmp, 0x00, sizeof(struct tm));
	ParseIso8601Date(str, &tmp);
	ParseIso8601DateStrict(str, &tmp);
	ParseIso8601Time(str, &tmp, &tz);
	ParseIso8601TimeStrict(str, &tmp, &tz);

	free(str);
	return 0;
}
