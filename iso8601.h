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

ISO8601LIB_API bool ParseIso8601Date(const char *str, struct tm *tmout);
ISO8601LIB_API bool ParseIso8601Time(const char *str, struct tm *tmout, int *timezoneOffsetMin);
ISO8601LIB_API bool ParseIso8601Datetime(const char *str, struct tm *tmout, int *timezoneOffsetMin);
//Convert tm struct to UTC timezone
ISO8601LIB_API void TmToUtc(struct tm *tmout, int timezoneOffsetMin);

#ifdef __cplusplus
}
#endif

#endif //_ISO8601_H

