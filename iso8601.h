#ifndef _ISO8601_H
#define _ISO8601_H

#include <vector>
#include <string>
#include <stdint.h>

bool ParseIso8601Date(const char *str, struct tm &tmout, bool normalize = true);

bool ParseIso8601Time(const char *str, struct tm &tmout, bool normalize = true);

bool ParseIso8601Datetime(const char *str, struct tm &tmout, bool normalize = true);

#endif //_ISO8601_H

