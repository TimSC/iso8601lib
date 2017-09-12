# cppiso8601
Parsing a subset of iso8601 datetimes, dates and times in C++

Only a subset of ISO 8601 is implemented: certain datetimes, dates or times are handled. Timezone is supported.
Output tm struct is in UTC.

No week formats. No "representation with reduced accuracy".
No ordinal dates. No leap seconds. No commas for floats.

Inspired by https://stackoverflow.com/a/26896792/4288232

Supported formats:

* 2017-09-11
* 2017-09-11T21:52:13+00:00
* 2017-09-11T21:52:13Z
* 20170911T215213Z
* 2009-12T12:34

Subset of test cases from https://www.myintervals.com/blog/2009/05/20/iso-8601-date-validation-that-doesnt-suck/

* 2009
* 2009-05-19
* 20090519
* 2009-05
* 2009-05-19T14:39Z
* 20090621T0545Z
* 2007-04-06T00:00
* 2007-04-05T24:00
* 2010-02-18T16:23:48.5
* 2010-02-18T16:23.4
* 2010-02-18T16:23.33+0600
* 2010-02-18T16:23.33-0530
* 2010-02-18T16.23334444

