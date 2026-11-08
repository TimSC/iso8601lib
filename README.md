# iso8601lib
Parsing a subset of iso8601 datetimes, dates and times in C. Simply compile iso8601.c into your project and include iso8601.h.

Only a subset of ISO 8601 is implemented: certain datetimes, dates or times are handled. Timezone is supported.
Output tm struct with timezone offset. This can be converted to UTC using TmToUtc(). tm structs can be converted to POSIX time using mktime.

No commas for floats.

## Permissive and strict parsing

Each parsing function has a permissive and a strict version:

| Permissive | Strict |
|---|---|
| `ParseIso8601Datetime()` | `ParseIso8601DatetimeStrict()` |
| `ParseIso8601Date()` | `ParseIso8601DateStrict()` |
| `ParseIso8601Time()` | `ParseIso8601TimeStrict()` |

Both versions require the input to match one of the supported formats below, and both reject malformed input such as `2009-05-19T14a39` or a timezone with trailing characters (`+0100junk`).

They differ in how they treat field values that are out of range:

* **Permissive** functions accept out of range values and normalize them into a valid date and time, the same way `timegm()` does. For example:
    * `2023-02-30` becomes 2023-03-02
    * `2023-13` becomes 2024-01
    * `2023-366` (2023 is not a leap year) becomes 2024-01-01
    * `2023-W53-1` (2023 has 52 ISO weeks) becomes 2024-01-01
    * `T12:60` becomes 13:00, and `T25:00` becomes 01:00 on the following day
    * a timezone of `+01:60` is treated as `+02:00`
* **Strict** functions return false if any field is out of range: month, day of month (taking month lengths and leap years into account), ordinal day, ISO week number (52 or 53 depending on the year), weekday, hour, minute, second, and timezone offset (hours up to 14, minutes up to 59).

Both versions accept `24:00` (with zero minutes and seconds), which is midnight at the end of the day.

Leap seconds: POSIX time has no leap seconds, so second 60 rolls over to the next minute (`2016-12-31T23:59:60Z` gives 2017-01-01T00:00:00Z). Strict parsing accepts second 60 only in the last minute of a UTC day, after applying the timezone offset. It does not check whether a leap second actually happened on that date.

Inspired by https://stackoverflow.com/a/26896792/4288232 and https://www.myintervals.com/blog/2009/05/20/iso-8601-date-validation-that-doesnt-suck/

Supported formats:

* 2017-09-11
* 2017-09-11T21:52:13+00:00
* 2017-09-11T21:52:13Z
* 20170911T215213Z
* 2009-12T12:34
* 2009
* 2009-05-19
* 20090519
* 2009-05
* 2009-123
* 2009-W51-1
* 2009-W33
* 2009W511
* 2009-05-19T14:39Z
* 20090621T0545Z
* 2007-04-06T00:00
* 2007-04-05T24:00
* 2010-02-18T16:23:48.5
* 2010-02-18T16:23.4
* 2010-02-18T16:23.33+0600
* 2010-02-18T16:23.33-0530
* 2010-02-18T16.23334444

