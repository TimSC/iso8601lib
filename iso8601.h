#ifndef _ISO8601_H
#define _ISO8601_H

#include <vector>
#include <string>

//Not thread safe if normalize is true!
bool ParseIso8601Date(const char *str, struct tm &tmout, bool normalize = true);

//Not thread safe if normalize is true!
bool ParseIso8601Time(const char *str, struct tm &tmout, bool normalize = true);

//Not thread safe if normalize is true!
bool ParseIso8601Datetime(const char *str, struct tm &tmout, bool normalize = true);

void Iso8601TestCases(std::vector<std::string> &testStrs);

#endif //_ISO8601_H

