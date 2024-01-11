#pragma once

#include "resource.h"

std::string timeToISO8601(const time_t& TheTime, const bool LocalTime = false);
std::string timeToISO8601(const CTimeSpan& TheTimeSpan);
std::string timeToExcelDate(const time_t& TheTime);
std::string getTimeISO8601(const bool LocalTime = false);
std::string getTimeRFC1123(void);
time_t ISO8601totime(const std::string& ISOTime);
std::wstring getwTimeISO8601(const bool LocalTime = false);
