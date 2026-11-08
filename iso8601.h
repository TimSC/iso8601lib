#ifndef _ISO8601_H
#define _ISO8601_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef ISO8601LIB_EXPORT
    #include "iso8601lib_export.h"
#else
    #ifndef ISO8601LIB_API
        #define ISO8601LIB_API
    #endif
#endif

//Permissive parsing: the input must match a supported format, but out of range
//field values are accepted and normalized (e.g. 2023-02-30 becomes 2023-03-02,
//12:60 becomes 13:00).
ISO8601LIB_API bool ParseIso8601Date(const char *str, struct tm *tmout);
ISO8601LIB_API bool ParseIso8601Time(const char *str, struct tm *tmout, int *timezoneOffsetMin);
ISO8601LIB_API bool ParseIso8601Datetime(const char *str, struct tm *tmout, int *timezoneOffsetMin);

//Strict parsing: as above, but returns false if any field is out of range
//(month, day of month, ordinal day, week, weekday, hour, minute, second or
//timezone offset). 24:00 and leap seconds (23:59:60 UTC) are accepted.
ISO8601LIB_API bool ParseIso8601DateStrict(const char *str, struct tm *tmout);
ISO8601LIB_API bool ParseIso8601TimeStrict(const char *str, struct tm *tmout, int *timezoneOffsetMin);
ISO8601LIB_API bool ParseIso8601DatetimeStrict(const char *str, struct tm *tmout, int *timezoneOffsetMin);
//Convert tm struct to UTC timezone
ISO8601LIB_API void TmToUtc(struct tm *tmout, int timezoneOffsetMin);

#ifdef __cplusplus
}
#endif

#endif //_ISO8601_H

