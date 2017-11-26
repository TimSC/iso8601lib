#ifndef _ISO8601_H
#define _ISO8601_H

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

bool ParseIso8601Date(const char *str, struct tm *tmout);

bool ParseIso8601Time(const char *str, struct tm *tmout, int *timezoneOffsetMin);

bool ParseIso8601Datetime(const char *str, struct tm *tmout, int *timezoneOffsetMin);

//Convert tm struct to UTC timezone
void TmToUtc(struct tm *tmout, int timezoneOffsetMin);

#endif //_ISO8601_H

