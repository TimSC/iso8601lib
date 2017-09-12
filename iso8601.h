#ifndef _ISO8601_H
#define _ISO8601_H

#include <vector>
#include <string>

bool ParseIso8601Date(const char *str, struct tm &tmout);
bool ParseIso8601Time(const char *str, struct tm &tmout);
bool ParseIso8601Datetime(const char *str, struct tm &tmout);
void Iso8601TestCases(std::vector<std::string> &testStrs);

#endif //_ISO8601_H

