#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////
std::string timeToISO8601(const time_t& TheTime, const bool LocalTime)
{
	std::ostringstream ISOTime;
	struct tm UTC;
	struct tm* timecallresult(nullptr);
	if (LocalTime)
#ifdef localtime_r
		timecallresult = localtime_r(&TheTime, &UTC);
#else
		#pragma warning(suppress : 4996)
		timecallresult = localtime(&TheTime);
#endif
	else
#ifdef gmtime_r
		timecallresult = gmtime_r(&TheTime, &UTC);
#else
		#pragma warning(suppress : 4996)
		timecallresult = gmtime(&TheTime);
#endif
	if (nullptr != timecallresult)
	{
#ifndef gmtime_r
		UTC = *timecallresult;
#endif // !gmtime_r

		ISOTime.fill('0');
		if (!((UTC.tm_year == 70) && (UTC.tm_mon == 0) && (UTC.tm_mday == 1)))
		{
			ISOTime << UTC.tm_year + 1900 << "-";
			ISOTime.width(2);
			ISOTime << UTC.tm_mon + 1 << "-";
			ISOTime.width(2);
			ISOTime << UTC.tm_mday << "T";
		}
		ISOTime.width(2);
		ISOTime << UTC.tm_hour << ":";
		ISOTime.width(2);
		ISOTime << UTC.tm_min << ":";
		ISOTime.width(2);
		ISOTime << UTC.tm_sec;
	}
	return(ISOTime.str());
}
std::string timeToISO8601(const CTimeSpan& TheTimeSpan)
{
	std::ostringstream ISOTime;
	ISOTime << "P";
	if (TheTimeSpan.GetDays() > 0)
		ISOTime << TheTimeSpan.GetDays() << "D";
	ISOTime << "T";
	if (TheTimeSpan.GetHours() > 0)
		ISOTime << TheTimeSpan.GetHours() << "H";
	if (TheTimeSpan.GetMinutes() > 0)
		ISOTime << TheTimeSpan.GetMinutes() << "M";
	if (TheTimeSpan.GetSeconds() > 0)
		ISOTime << TheTimeSpan.GetSeconds() << "S";
	return(ISOTime.str());
}
std::string timeToExcelDate(const time_t& TheTime)
{
	std::ostringstream ISOTime;
	struct tm UTC;
	gmtime_s(&UTC, &TheTime);
	ISOTime.fill('0');
	ISOTime << UTC.tm_year + 1900 << "-";
	ISOTime.width(2);
	ISOTime << UTC.tm_mon + 1 << "-";
	ISOTime.width(2);
	ISOTime << UTC.tm_mday << " ";
	ISOTime.width(2);
	ISOTime << UTC.tm_hour << ":";
	ISOTime.width(2);
	ISOTime << UTC.tm_min << ":";
	ISOTime.width(2);
	ISOTime << UTC.tm_sec;
	return(ISOTime.str());
}
std::string getTimeISO8601(const bool LocalTime)
{
	time_t timer;
	time(&timer);
	std::string isostring(timeToISO8601(timer, LocalTime));
	std::string rval;
	rval.assign(isostring.begin(), isostring.end());
	return(rval);
}
std::string getTimeRFC1123(void)
{
	//InternetTimeFromSystemTime(&sysTime, INTERNET_RFC1123_FORMAT, tchInternetTime, sizeof(tchInternetTime));
	//HttpResponse << "Date: " << CStringA(CString(tchInternetTime)).GetString() << "\r\n";
	time_t timer;
	#pragma warning(suppress : 4996)
	time(&timer);
	#pragma warning(suppress : 4996)
	std::string RFCTime(asctime(gmtime(&timer)));
	RFCTime.pop_back();	// gets rid of the \n that asctime puts at the end of the line.
	RFCTime.append(" GMT");
	return(RFCTime);
}
time_t ISO8601totime(const std::string& ISOTime)
{
	struct tm UTC;
	UTC.tm_year = atol(ISOTime.substr(0, 4).c_str()) - 1900;
	UTC.tm_mon = atol(ISOTime.substr(5, 2).c_str()) - 1;
	UTC.tm_mday = atol(ISOTime.substr(8, 2).c_str());
	UTC.tm_hour = atol(ISOTime.substr(11, 2).c_str());
	UTC.tm_min = atol(ISOTime.substr(14, 2).c_str());
	UTC.tm_sec = atol(ISOTime.substr(17, 2).c_str());
#ifdef _MSC_VER
	_tzset();
	int hours = 0;
	if (0 != _get_daylight(&hours))
		hours = 0;
	long SecondsFromUTC = 0;
	if (0 != _get_timezone(&SecondsFromUTC))
		SecondsFromUTC = 0;
	long dstbias = 0;
	if (0 != _get_dstbias(&dstbias))
		dstbias = 0;
	UTC.tm_isdst = hours;
#endif
	time_t timer = mktime(&UTC);
#ifdef _MSC_VER
	timer -= SecondsFromUTC;
	timer -= hours * dstbias;
#endif
	return(timer);
}
/////////////////////////////////////////////////////////////////////////////
std::wstring getwTimeISO8601(const bool LocalTime)
{
	std::string isostring(getTimeISO8601(LocalTime));
	std::wstring rval;
	rval.assign(isostring.begin(), isostring.end());

	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
//std::string timeToISO8601(const CTime & TheTime)
//{
//	time_t TheOtherTime;
//	//mktime(
//	//TheTime.
//	return(timeToISO8601(TheOtherTime));
//}