// WimTiVoServer.cpp : Defines the entry point for the console application.
//

// Other Things to look at:
// StreamBaby http://code.google.com/p/streambaby/
// TiVoStream http://code.google.com/p/tivostream/wiki/native_video_formats
// http://www.tivocommunity.com/tivo-vb/showthread.php?t=403066
// TiVoSDKs .Net Libraries http://code.google.com/p/tivo-sdks/source/browse/trunk/Tivo.Hme/Tivo.Hmo/Calypso16.cs?spec=svn112&r=112
// TiVoServer http://tivoserver.cvs.sourceforge.net/viewvc/tivoserver/tivoserver/BeaconManager.cc?revision=1.16&view=markup
// TiVo Disk Space Viewer http://peterkellner.net/2008/01/18/tivospaceviewerwithlinq/
// According to this page, the TiVo times are HEX encoded seconds since January 1, 1970 0:00:00 UTC. http://www.tivocommunity.com/tivo-vb/archive/index.php/t-314742.html
// http://porkrind.org/missives/tivo-desktop-on-linux/ is somethign useful I found after resetting my TiVo too many times.


// ffmpeg getting started: http://ffmpeg.org/trac/ffmpeg/wiki/Using%20libav*
// some possible settings info: http://pytivo.sourceforge.net/forum/hd-tivo-ideal-settings-t40.html

// Logon to Tivo with username: "tivo" password: MAK
// Wim's TiVoHD (MAK) Media Access Key: 1760168186
// Mom & Dad's TiVoHD (MAK) Media Access Key: 9371539867
// [2012-12-26T03:10:42] 192.168.1.11 tivoconnect=1 swversion=11.0k-01-2-652 method=broadcast identity=6520201806EEAAE machine=Living Room platform=tcd/Series3 services=TiVoMediaServer:80/http
// [2012-12-26T06:32:43] 192.168.1.48 TiVoConnect=1 Machine=HPQUAD Identity={E2C4F367-F194-4C8D-84B6-EBDF47CCF26E} Method=Broadcast Platform=pc/WinNT:6.1.7601 Services=TiVoMediaServer:8080/http
// [2013-01-26T03:47:21] 192.168.0.5 TiVoConnect=1 Machine=INSPIRON Identity={9DA17E3D-FEC4-4533-9F0F-E2DB35EAA9B1} Method=Broadcast Platform=pc/WinNT:6.1.7601 Services=TiVoMediaServer:8080/http
// [2013-01-26T03:47:21] 192.168.0.108 tivoconnect=1 swversion=11.0k-01-2-652 method=broadcast identity=6520001802E00C3 machine=TivoHD platform=tcd/Series3 services=TiVoMediaServer:80/http

// STL Reference I like to use: http://cplusplus.com/reference/fstream/ofstream/ofstream/
// I'm going to try to use XMLLite for XML Processing. http://msdn.microsoft.com/en-us/library/windows/desktop/ms752864(v=vs.85).aspx
// XMLLite processes IStream objects. 
// Is an MSDN article that I want to read more fully. I think it'll have good shortcuts for proper ways of dealing with IStreams. http://msdn.microsoft.com/en-us/magazine/cc163436.aspx#S5
// SHCreateMemStream function http://msdn.microsoft.com/en-us/library/windows/desktop/bb773831(v=vs.85).aspx
// SHCreateStreamOnFile function http://msdn.microsoft.com/en-us/library/windows/desktop/bb759864(v=vs.85).aspx

// Smart Pointer Reference: http://msdn.microsoft.com/en-us/library/hh279674.aspx
// WinINet vs. WinHTTP: http://msdn.microsoft.com/en-us/library/windows/desktop/hh227298(v=vs.85).aspx

// robocopy c:\Users\Wim\Downloads\ffmpeg-20130318-git-519ebb5-win64-shared\bin $(TargetDir) *.dll
// robocopy c:\Users\Wim\Downloads\ffmpeg-20130318-git-519ebb5-win64-shared\bin c:\Users\Wim\Documents\ss\WimTiVoServer\x64\Release *.dll
// robocopy c:\Users\Wim\Downloads\ffmpeg-20130318-git-519ebb5-win64-shared\bin c:\Users\Wim\Documents\ss\WimTiVoServer\x64\Debug *.dll

// Here are some FFMPEG command lines used by pyTiVo to transfer files to the tivo: (The first was the only 1080i wtv file)
// ffmpeg.exe -i D:\RECORD~1\Grimm_KINGDT_2013_03_15_21_00_00.wtv -vcodec copy -b 8922k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec copy -map 0:1 -map 0:0 -report -f vob -
// "D:\\pytivo\\bin\\ffmpeg.exe" -i "D:\\Videos\\TAKEN_2\\Taken_2_t00.mkv" -vcodec copy -b 6726k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec copy -map 0:0 -map 0:1 -report -f vob -
// "D:\\pytivo\\bin\\ffmpeg.exe" -i "D:\\Videos\\archer\\Archer.2009.S03E12.HDTV.x264.mp4" -vcodec mpeg2video -b 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec ac3 -copyts -map 0:0 -map 0:1 -report -f vob -
// "D:\\pytivo\\bin\\ffmpeg.exe" -i "D:\\Videos\\THE_INTOUCHABLES\\THE_INTOUCHABLES_t08.mkv" -vcodec copy -b 7017k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec copy -map 0:0 -map 0:1 -report -f vob -

// This one managed to encode the subtitles from the DVD correctly, but the image quality was poor
// "c:\\Users\\Wim\\Downloads\\ffmpeg-20130306-git-28adecf-win64-static\\bin\\ffmpeg.exe" -report -i THE_INTOUCHABLES_t08.mkv -filter_complex "[0:0][0:2]overlay" -acodec copy THE_INTOUCHABLES.vob
// This one I copied some of the commands from the pyTiVo examples and it came out looking pretty good.
// "c:\\Users\\Wim\\Downloads\\ffmpeg-20130306-git-28adecf-win64-static\\bin\\ffmpeg.exe" -report -i THE_INTOUCHABLES_t08.mkv -vcodec mpeg2video -b 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -filter_complex "[0:0][0:2]overlay" -acodec copy THE_INTOUCHABLES.vob
// "c:\\Users\\Wim\\Downloads\\ffmpeg-20130306-git-28adecf-win64-static\\bin\\ffmpeg.exe" -report -i THE_INTOUCHABLES_t08.mkv -vcodec mpeg2video -b 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -filter_complex "[0:0][0:2]overlay" -acodec copy -sn THE_INTOUCHABLES.mkv
// ffmpeg.exe -report -i \\Acid\Videos\LIFE_OF_PI\Life_of_Pi_t00.mkv -vcodec mpeg2video -b 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -filter_complex "[0:0][0:7]overlay" -acodec copy -sn \\Acid\TiVo\Life_of_Pi.mkv

// Here's an experiment in making a decent quality file for the ipad with subtitles, it seemed to compare favorbly with the original.
// "c:\\Users\\Wim\\Downloads\\ffmpeg-20130318-git-519ebb5-win64-static\\bin\\ffmpeg.exe" -report -i THE_INTOUCHABLES_t08.mkv -filter_complex "[0:0][0:2]overlay" -ac 2 THE_INTOUCHABLES.mp4


#include "stdafx.h"
#include "WimTiVoServer.h"
#include "CTiVo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object

CWinApp theApp;

using namespace std;

//////////////////////////////////////////////////////////////////////////////
HANDLE terminateEvent_http = NULL;
HANDLE terminateEvent_beacon = NULL;
HANDLE terminateEvent_populate = NULL;
SERVICE_STATUS_HANDLE serviceStatusHandle = NULL;
bool pauseService = false;
CWinThread * threadHandle = NULL;
SOCKET ControlSocket = INVALID_SOCKET;
bool bConsoleExists = false;
HANDLE ApplicationLogHandle = NULL;
cTiVoServer myServer;
CCriticalSection ccTiVoFileListCritSec;
std::vector<cTiVoFile> TiVoFileList;
/////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "version")
CString GetFileVersion(const CString & filename, const int digits = 4)
{
	CString rval;
	// get The Version number of the file
	DWORD dwVerHnd = 0;
	DWORD nVersionInfoSize = ::GetFileVersionInfoSize((LPTSTR)filename.GetString(), &dwVerHnd);
	if (nVersionInfoSize > 0)
	{
		UINT *puVersionLen = new UINT;
		LPVOID pVersionInfo = new char[nVersionInfoSize];
		BOOL bTest = ::GetFileVersionInfo((LPTSTR)filename.GetString(), dwVerHnd, nVersionInfoSize, pVersionInfo);
		// Pull out the version number
		if (bTest)
		{
			LPVOID pVersionNum = NULL;
			bTest = ::VerQueryValue(pVersionInfo, _T("\\"), &pVersionNum, puVersionLen);
			if (bTest)
			{
				DWORD dwFileVersionMS = ((VS_FIXEDFILEINFO *)pVersionNum)->dwFileVersionMS;
				DWORD dwFileVersionLS = ((VS_FIXEDFILEINFO *)pVersionNum)->dwFileVersionLS;
				switch (digits)
				{
				default:
				case 4:
					rval.Format(_T("%d.%d.%d.%d"), HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS), HIWORD(dwFileVersionLS), LOWORD(dwFileVersionLS));
					break;
				case 3:
					rval.Format(_T("%d.%d.%d"), HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS), HIWORD(dwFileVersionLS));
					break;
				case 2:
					rval.Format(_T("%d.%d"), HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS));
					break;
				case 1:
					rval.Format(_T("%d"), HIWORD(dwFileVersionMS));
				}
			}
		}
		delete puVersionLen;
		delete [] pVersionInfo;	
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
string timeToISO8601(const time_t & TheTime)
{
	ostringstream ISOTime;
	time_t timer = TheTime;
	struct tm * UTC = gmtime(&timer);
	if (UTC != NULL)
	{
		ISOTime.fill('0');
		ISOTime << UTC->tm_year+1900 << "-";
		ISOTime.width(2);
		ISOTime << UTC->tm_mon+1 << "-";
		ISOTime.width(2);
		ISOTime << UTC->tm_mday << "T";
		ISOTime.width(2);
		ISOTime << UTC->tm_hour << ":";
		ISOTime.width(2);
		ISOTime << UTC->tm_min << ":";
		ISOTime.width(2);
		ISOTime << UTC->tm_sec;
	}
	return(ISOTime.str());
}
string timeToExcelDate(const time_t & TheTime)
{
	ostringstream ISOTime;
	time_t timer = TheTime;
	struct tm * UTC = gmtime(&timer);
	if (UTC != NULL)
	{
		ISOTime.fill('0');
		ISOTime << UTC->tm_year+1900 << "-";
		ISOTime.width(2);
		ISOTime << UTC->tm_mon+1 << "-";
		ISOTime.width(2);
		ISOTime << UTC->tm_mday << " ";
		ISOTime.width(2);
		ISOTime << UTC->tm_hour << ":";
		ISOTime.width(2);
		ISOTime << UTC->tm_min << ":";
		ISOTime.width(2);
		ISOTime << UTC->tm_sec;
	}
	return(ISOTime.str());
}
string getTimeISO8601(void)
{
	time_t timer;
	time(&timer);
	return(timeToISO8601(timer));
}
string getTimeRFC1123(void)
{
	//InternetTimeFromSystemTime(&sysTime, INTERNET_RFC1123_FORMAT, tchInternetTime, sizeof(tchInternetTime));
	//HttpResponse << "Date: " << CStringA(CString(tchInternetTime)).GetString() << "\r\n";
	time_t timer;
	time(&timer);
	std::string RFCTime(asctime(gmtime(&timer)));
	RFCTime.pop_back();	// gets rid of the \n that asctime puts at the end of the line.
	RFCTime.append(" GMT");
	return(RFCTime);
}
time_t ISO8601totime(const string & ISOTime)
{
	struct tm UTC;
	UTC.tm_year = atol(ISOTime.substr(0,4).c_str())-1900;
	UTC.tm_mon = atol(ISOTime.substr(5,2).c_str())-1;
	UTC.tm_mday = atol(ISOTime.substr(8,2).c_str());
	UTC.tm_hour = atol(ISOTime.substr(11,2).c_str());
	UTC.tm_min = atol(ISOTime.substr(14,2).c_str());
	UTC.tm_sec = atol(ISOTime.substr(17,2).c_str());
	#ifdef _MSC_VER
	_tzset();
	UTC.tm_isdst = _daylight;
	#endif
	time_t timer = mktime(&UTC);
	#ifdef _MSC_VER
	timer -= _timezone;
	timer += _daylight*_dstbias;
	#endif
	return(timer);
}
/////////////////////////////////////////////////////////////////////////////
CString FindEXEFromPath(const CString & csEXE)
{
	CString csFullPath;
	//PathFindOnPath();
	CFileFind finder;
	if (finder.FindFile(csEXE))
	{
		finder.FindNextFile();
		csFullPath = finder.GetFilePath();
		finder.Close();
	}
	else
	{
		TCHAR filename[MAX_PATH];
		unsigned long buffersize = sizeof(filename) / sizeof(TCHAR);
		// Get the file name that we are running from.
		GetModuleFileName(AfxGetResourceHandle(), filename, buffersize );
		PathRemoveFileSpec(filename);
		PathAppend(filename, csEXE);
		if (finder.FindFile(filename))
		{
			finder.FindNextFile();
			csFullPath = finder.GetFilePath();
			finder.Close();
		}
		else
		{
			CString csPATH;
			csPATH.GetEnvironmentVariable(_T("PATH"));
			int iStart = 0;
			CString csToken(csPATH.Tokenize(_T(";"), iStart));
			while (csToken != _T(""))
			{
				if (csToken.Right(1) != _T("\\"))
					csToken.AppendChar(_T('\\'));
				csToken.Append(csEXE);
				if (finder.FindFile(csToken))
				{
					finder.FindNextFile();
					csFullPath = finder.GetFilePath();
					finder.Close();
					break;
				}
				csToken = csPATH.Tokenize(_T(";"), iStart);
			}
		}
	}
	return(csFullPath);
}
CString ReplaceExtension(const CString & OriginalPath, const CString & NewExtension)
{
	CString rVal(OriginalPath);
	rVal.Truncate(rVal.ReverseFind(_T('.')));
	rVal.Append(NewExtension);
	while (rVal.Replace(_T(".."),_T(".")) > 0);
	return(rVal);
}
/////////////////////////////////////////////////////////////////////////////
const CString QuoteFileName(const CString & Original)
{
	CString csQuotedString(Original);
	if (csQuotedString.Find(_T(" ")) >= 0)
	{
		csQuotedString.Insert(0,_T('"'));
		csQuotedString.AppendChar(_T('"'));
	}
	return(csQuotedString);
}
/////////////////////////////////////////////////////////////////////////////
void PopulateTiVoFileList(std::vector<cTiVoFile> & TiVoFileList, CCriticalSection & ccTiVoFileListCritSec, std::string FileSpec, bool Recurse = false)
{
	#ifdef _DEBUG
	TRACE("%s %s %s\n", CStringA(CTime::GetCurrentTime().Format(_T("[%Y-%m-%dT%H:%M:%S]"))).GetString(), __FUNCTION__, FileSpec.c_str());
	std::cout << "[" << getTimeISO8601() << "] " << __FUNCTION__ << " " << FileSpec.c_str() << std::endl;
	#endif
	CFileFind finder;
	
	BOOL bWorking = finder.FindFile(CString(CStringA(FileSpec.c_str())));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots())
			continue;
		if (finder.IsHidden())
			continue;
		if (finder.IsSystem())
			continue;
		if (finder.IsTemporary())
			continue;
		if (finder.IsDirectory())
		{
			if (Recurse)
			{
				std::stringstream ss;
				ss << CStringA(finder.GetFilePath()).GetString() << "\\*";
				PopulateTiVoFileList(TiVoFileList, ccTiVoFileListCritSec, ss.str());
			}
			continue;
		}
		cTiVoFile myFile;
		if (finder.GetLength() > 0)
		{
			bool bNotInList = true;
			ccTiVoFileListCritSec.Lock();
			for (auto TiVoFile = TiVoFileList.begin(); TiVoFile != TiVoFileList.end(); TiVoFile++)
				if (!TiVoFile->GetPathName().CompareNoCase(finder.GetFilePath()))
				{
					CTime FinderTime;
					if (finder.GetLastWriteTime(FinderTime))
						if (FinderTime > TiVoFile->GetLastWriteTime())
							TiVoFile->SetPathName(finder);
					bNotInList = false;
					break;
				}
			ccTiVoFileListCritSec.Unlock();
			if (bNotInList)
			{
				myFile.SetPathName(finder);
				if (!myFile.GetSourceFormat().Left(6).CompareNoCase(_T("video/")))
				{
					ccTiVoFileListCritSec.Lock();
					TiVoFileList.push_back(myFile);
					ccTiVoFileListCritSec.Unlock();
				}
			}
		}
	}
	finder.Close();
}
void CleanTiVoFileList(std::vector<cTiVoFile> & TiVoFileList, CCriticalSection & ccTiVoFileListCritSec)
{
	#ifdef _DEBUG
	TRACE("%s %s\n", CStringA(CTime::GetCurrentTime().Format(_T("[%Y-%m-%dT%H:%M:%S]"))).GetString(), __FUNCTION__);
	std::cout << "[" << getTimeISO8601() << "] " << __FUNCTION__ << std::endl;
	#endif
	ccTiVoFileListCritSec.Lock();
	for (auto TiVoFile = TiVoFileList.begin(); TiVoFile != TiVoFileList.end(); TiVoFile++)
	{
		CFileStatus status;
		if (!CFile::GetStatus(TiVoFile->GetPathName(), status))
		{
			TiVoFileList.erase(TiVoFile);
			TiVoFile = TiVoFileList.begin();
		}
	}
	ccTiVoFileListCritSec.Unlock();
}
/////////////////////////////////////////////////////////////////////////////
void printerr(TCHAR * errormsg)
{
	if (bConsoleExists)
		_ftprintf(stderr, _T("%s\n"), errormsg);
	else if (ApplicationLogHandle != NULL) 
	{
		LPCTSTR lpStrings[] = { errormsg, NULL };
		ReportEvent(ApplicationLogHandle, EVENTLOG_INFORMATION_TYPE, 0, WIMSWORLD_EVENT_GENERIC, NULL, 1, 0, lpStrings, NULL);
	}
}
int GetTiVoQueryFormats(SOCKET DataSocket, const char * InBuffer)
{
	#ifdef _DEBUG
	TRACE("%s %s\n", CStringA(CTime::GetCurrentTime().Format(_T("[%Y-%m-%dT%H:%M:%S]"))).GetString(), __FUNCTION__);
	CStringA csInBuffer(InBuffer);
	int pos = csInBuffer.FindOneOf("\r\n");
	if (pos > 0)
		csInBuffer.Delete(pos,csInBuffer.GetLength());
	struct sockaddr_in adr_inet;/* AF_INET */  
	int sa_len = sizeof(adr_inet);
	getpeername(DataSocket, (struct sockaddr *)&adr_inet, &sa_len);
	std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\t" << inet_ntoa(adr_inet.sin_addr) << " " << csInBuffer.GetString() << endl;
	#endif
	int rval = 0;
	char MyHostName[255] = {0}; // winsock hostname used for data recordkeeping
	gethostname(MyHostName,sizeof(MyHostName)); 
	char XMLDataBuff[1024*11];
	CComPtr<IStream> spMemoryStream(::SHCreateMemStream(NULL, 0));
	CComPtr<IXmlWriter> pWriter;
	if (SUCCEEDED(CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
	{
		pWriter->SetOutput(spMemoryStream);
		pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
		pWriter->WriteStartDocument(XmlStandalone_Omit);
		pWriter->WriteStartElement(NULL,_T("TiVoFormats"),NULL);
			pWriter->WriteStartElement(NULL,_T("Format"),NULL);
				pWriter->WriteElementString(NULL, _T("ContentType"), NULL, _T("video/x-tivo-mpeg"));
				pWriter->WriteElementString(NULL, _T("Description"), NULL, _T("TiVo Recording"));
			pWriter->WriteEndElement();
			//pWriter->WriteStartElement(NULL,_T("Format"),NULL);
			//	pWriter->WriteElementString(NULL, _T("ContentType"), NULL, _T("video/mpeg"));
			//	pWriter->WriteElementString(NULL, _T("Description"), NULL, _T("MPEG-2 Video"));
			//pWriter->WriteEndElement();
			//pWriter->WriteStartElement(NULL,_T("Format"),NULL);
			//	pWriter->WriteElementString(NULL, _T("ContentType"), NULL, _T("video/x-tivo-mpeg-ts"));
			//	pWriter->WriteElementString(NULL, _T("Description"), NULL, _T("TiVo TS Recording"));
			//pWriter->WriteEndElement();
			//pWriter->WriteStartElement(NULL,_T("Format"),NULL);
			//	pWriter->WriteElementString(NULL, _T("ContentType"), NULL, _T("video/x-tivo-raw-tts"));
			//	pWriter->WriteElementString(NULL, _T("Description"), NULL, NULL);
			//pWriter->WriteEndElement();
		pWriter->WriteFullEndElement();
		pWriter->WriteComment(L" Copyright © 2013 William C Bonner ");
		pWriter->WriteEndDocument();
		pWriter->Flush();
		// Allocates enough memeory for the xml content.
		STATSTG ssStreamData = {0};
		spMemoryStream->Stat(&ssStreamData, STATFLAG_NONAME);
		SIZE_T cbSize = ssStreamData.cbSize.LowPart;
		if (cbSize >= sizeof(XMLDataBuff))
			cbSize = sizeof(XMLDataBuff)-1;
		// Copies the content from the stream to the buffer.
		LARGE_INTEGER position;
		position.QuadPart = 0;
		spMemoryStream->Seek(position, STREAM_SEEK_SET, NULL);
		ULONG cbRead;
		spMemoryStream->Read(XMLDataBuff, cbSize, &cbRead);
		XMLDataBuff[cbSize] = '\0';
	}

	/* Create HTTP Header */
	std::stringstream HttpResponse;
	HttpResponse << "HTTP/1.1 200 OK\r\n";
	HttpResponse << "Content-Type: text/xml\r\n";
	HttpResponse << "Connection: close\r\n";
	HttpResponse << "Date: " << getTimeRFC1123() << "\r\n";
	HttpResponse << "Content-Length: " << strlen(XMLDataBuff) << "\r\n";
	HttpResponse << "\r\n";
	send(DataSocket, HttpResponse.str().c_str(), HttpResponse.str().length(),0);
	send(DataSocket, XMLDataBuff, strlen(XMLDataBuff),0);
	return(0);
}
UINT PopulateTiVoFileList(LPVOID lvp)
{
	HANDLE LocalTerminationEventHandle = lvp;
	if (ApplicationLogHandle != NULL) 
	{
		CString csSubstitutionText(__FUNCTION__);
		csSubstitutionText.Append(_T(" Has Started"));
		LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
		ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
	}
	CString csLocalContainers;
	if (csLocalContainers.IsEmpty())
	{
		CString csRegKey(_T("Software\\WimsWorld\\"));
		csRegKey.Append(theApp.m_pszAppName);
		TCHAR vData[1024];
		DWORD cbData = sizeof(vData);
		if (ERROR_SUCCESS == RegGetValue(HKEY_LOCAL_MACHINE, csRegKey, _T("Container"), RRF_RT_REG_SZ, NULL, vData, &cbData))
			csLocalContainers = CString(vData);
	}
	do {
		ccTiVoFileListCritSec.Lock(); 
		auto TiVoFileListSizeBefore = TiVoFileList.size();
		ccTiVoFileListCritSec.Unlock();
		CleanTiVoFileList(TiVoFileList, ccTiVoFileListCritSec);
		int iPos = 0;
		CString csToken(csLocalContainers.Tokenize(_T(";"), iPos));
		while (!csToken.IsEmpty())
		{
			PopulateTiVoFileList(TiVoFileList, ccTiVoFileListCritSec, CStringA(csToken).GetString(), true);
			ccTiVoFileListCritSec.Lock(); 
			std::sort(TiVoFileList.begin(), TiVoFileList.end(), cTiVoFileCompareDate);
			ccTiVoFileListCritSec.Unlock();
			csToken = csLocalContainers.Tokenize(_T(";"), iPos);
		}
		std::stringstream ss;
		ccTiVoFileListCritSec.Lock();
		auto TiVoFileListSizeAfter = TiVoFileList.size();
		ss << "[" << getTimeISO8601() << "] " __FUNCTION__ " TiVoFileListSizeBefore: " << TiVoFileListSizeBefore << " TiVoFileListSizeAfter: " << TiVoFileListSizeAfter << std::endl;
		ccTiVoFileListCritSec.Unlock();
		std::cout << ss.str().c_str();
		TRACE(ss.str().c_str());
		if ((ApplicationLogHandle != NULL) && (TiVoFileListSizeBefore != TiVoFileListSizeAfter))
		{
			CString csSubstitutionText(__FUNCTION__);
			csSubstitutionText.Append(_T("\n"));
			csSubstitutionText.Append(CString(ss.str().c_str()));
			LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
			ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
		}
	} while (WAIT_TIMEOUT == WaitForSingleObject(LocalTerminationEventHandle, 15*60*1000));
	if (ApplicationLogHandle != NULL) 
	{
		CString csSubstitutionText(__FUNCTION__);
		csSubstitutionText.Append(_T(" Has Stopped"));
		LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
		ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
	}
	return(0);
}
int GetTivoQueryContainer(SOCKET DataSocket, const char * InBuffer)
{
	TRACE("%s %s\n", CStringA(CTime::GetCurrentTime().Format(_T("[%Y-%m-%dT%H:%M:%S]"))).GetString(), __FUNCTION__);
	#ifdef _DEBUG
	CStringA csInBuffer(InBuffer);
	int pos = csInBuffer.FindOneOf("\r\n");
	if (pos > 0)
		csInBuffer.Delete(pos,csInBuffer.GetLength());
	struct sockaddr_in adr_inet;/* AF_INET */  
	int sa_len = sizeof(adr_inet);
	getpeername(DataSocket, (struct sockaddr *)&adr_inet, &sa_len);
	std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\t" << inet_ntoa(adr_inet.sin_addr) << " " << csInBuffer.GetString() << endl;
	#endif
	int rval = 0;
	const int XMLDataBuffSize = 1024;
	char* XMLDataBuff = new char[XMLDataBuffSize];
	XMLDataBuff[0] = 0;
	CComPtr<IXmlWriter> pWriter;
	CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), NULL);
	// from: http://stackoverflow.com/questions/3037946/how-can-i-store-xml-in-buffer-using-xmlite
	CComPtr<IStream> spMemoryStream(::SHCreateMemStream(NULL, 0));
	if ((pWriter != NULL) && (spMemoryStream != NULL))
	{
		TCHAR buffer[256] = TEXT("");
		DWORD dwSize = sizeof(buffer);
		GetComputerNameEx(ComputerNameDnsHostname, buffer, &dwSize);
		CString csMyHostName(buffer);
		CString csContainer;
		int iAnchorOffset = 0;
		CString csAnchorItem;
		ccTiVoFileListCritSec.Lock();
		if (!TiVoFileList.empty())
			csAnchorItem = TiVoFileList.begin()->GetURL();
		int iItemCount = TiVoFileList.size();
		ccTiVoFileListCritSec.Unlock();
		CStringA csCommand(InBuffer);
		int curPos = 0;
		CStringA csToken(csCommand.Tokenize("& ?",curPos));
		while (csToken != "")
		{
			CStringA csKey(csToken.Left(csToken.Find("=")));
			CStringA csValue(csToken.Right(csToken.GetLength() - (csToken.Find("=")+1)));
			if (!csKey.CompareNoCase("Container"))
			{
				std::cout << "[" << getTimeISO8601() << "] " << csToken.GetString() << std::endl;
				csContainer = csValue;
			}
			else if (!csKey.CompareNoCase("Recurse"))
				std::cout << "[                   ] " << csToken.GetString() << std::endl;
			else if (!csKey.CompareNoCase("SortOrder"))
			{
				std::cout << "[                   ] " << csToken.GetString() << std::endl;
				//std::sort(TiVoFileList.begin(),TiVoFileList.end(),cTiVoFileCompareDate);
			}
			else if (!csKey.CompareNoCase("RandomSeed"))
				std::cout << "[                   ] " << csToken.GetString() << std::endl;
			else if (!csKey.CompareNoCase("RandomStart"))
				std::cout << "[                   ] " << csToken.GetString() << std::endl;
			else if (!csKey.CompareNoCase("AnchorOffset"))
			{
				std::cout << "[                   ] " << csToken.GetString() << std::endl;
				iAnchorOffset = atoi(csValue.GetString());
				iAnchorOffset++; // Simplify for the -1 offset that the TiVo actually uses.
			}
			else if (!csKey.CompareNoCase("Filter"))
			{
				csToken.Replace("%2F","/");
				std::cout << "[                   ] " << csToken.GetString() << std::endl;
			}
			else if (!csKey.CompareNoCase("ItemCount"))
			{
				std::cout << "[                   ] " << csToken.GetString() << std::endl;
				iItemCount = min(iItemCount, atoi(CStringA(csValue).GetString()));
			}
			else if (!csKey.CompareNoCase("AnchorItem"))
			{
				std::cout << "[                   ] " << csToken.GetString() << std::endl;
				csAnchorItem = csValue;
				TCHAR lpszBuffer[_MAX_PATH];
				DWORD dwBufferLength = sizeof(lpszBuffer) / sizeof(TCHAR);
				InternetCanonicalizeUrl(csAnchorItem.GetString(), lpszBuffer, &dwBufferLength, ICU_DECODE | ICU_NO_ENCODE);
				csAnchorItem = CString(lpszBuffer, dwBufferLength);
				//URL_COMPONENTS crackedURL;
				//crackedURL.dwStructSize = sizeof(URL_COMPONENTS);
				//crackedURL.lpszScheme = NULL;			// pointer to scheme name
				//crackedURL.dwSchemeLength = 0;			// length of scheme name
				//crackedURL.nScheme;						// enumerated scheme type (if known)
				//crackedURL.lpszHostName = NULL;			// pointer to host name
				//crackedURL.dwHostNameLength = 0;		// length of host name
				//crackedURL.nPort;						// converted port number
				//crackedURL.lpszUserName = NULL;			// pointer to user name
				//crackedURL.dwUserNameLength = 0;		// length of user name
				//crackedURL.lpszPassword = NULL;			// pointer to password
				//crackedURL.dwPasswordLength = 0;		// length of password
				//crackedURL.lpszUrlPath = lpszBuffer;	// pointer to URL-path
				//crackedURL.dwUrlPathLength = _MAX_PATH;	// length of URL-path
				//crackedURL.lpszExtraInfo = NULL;		// pointer to extra information (e.g. ?foo or #foo)
				//crackedURL.dwExtraInfoLength = 0;		// length of extra information
				//InternetCrackUrl(csAnchorItem.GetString(),csAnchorItem.GetLength(),ICU_DECODE,&crackedURL);
				//csAnchorItem = CString(lpszBuffer);
				//dwBufferLength = sizeof(lpszBuffer) / sizeof(TCHAR);
				InternetCanonicalizeUrl(csAnchorItem.GetString(), lpszBuffer, &dwBufferLength, 0);
				csAnchorItem = CString(lpszBuffer, dwBufferLength);
			}
			csToken = csCommand.Tokenize("& ?",curPos);
		}
		pWriter->SetOutput(spMemoryStream);
		pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
		pWriter->WriteStartDocument(XmlStandalone_Omit);
			pWriter->WriteStartElement(NULL,L"TiVoContainer",L"http://www.tivo.com/developer/calypso-protocol-1.6/");
			if (csContainer.Compare(_T("%2F")) == 0)
			{
				pWriter->WriteStartElement(NULL,_T("Details"),NULL);
					pWriter->WriteElementString(NULL,_T("Title"),NULL, csMyHostName.GetString());
					pWriter->WriteElementString(NULL,_T("ContentType"),NULL, _T("x-tivo-container/tivo-server"));
					pWriter->WriteElementString(NULL,_T("SourceFormat"),NULL, _T("x-tivo-container/folder"));
					pWriter->WriteElementString(NULL,_T("TotalItems"),NULL, _T("1"));
				pWriter->WriteEndElement();	// Details
				pWriter->WriteStartElement(NULL,_T("Item"),NULL);
					pWriter->WriteStartElement(NULL,_T("Details"),NULL);
						pWriter->WriteElementString(NULL,_T("Title"),NULL, csMyHostName.GetString());
						pWriter->WriteElementString(NULL,_T("ContentType"),NULL, _T("x-tivo-container/tivo-videos"));
						pWriter->WriteElementString(NULL,_T("SourceFormat"),NULL, _T("x-tivo-container/folder"));
					pWriter->WriteEndElement();	// Details
					pWriter->WriteStartElement(NULL,_T("Links"),NULL);
						pWriter->WriteStartElement(NULL,_T("Content"),NULL);
							pWriter->WriteElementString(NULL,_T("Url"),NULL, _T("/TiVoConnect?Command=QueryContainer&Container=%2FTiVoNowPlaying"));
							pWriter->WriteElementString(NULL,_T("ContentType"),NULL, _T("x-tivo-container/tivo-videos"));
						pWriter->WriteFullEndElement();	// Content
					pWriter->WriteFullEndElement();	// Links
				pWriter->WriteFullEndElement();	// Item
				pWriter->WriteElementString(NULL,_T("ItemStart"),NULL, _T("0"));
				pWriter->WriteElementString(NULL,_T("ItemCount"),NULL, _T("1"));
			}
			else
			{
				ccTiVoFileListCritSec.Lock();
				// If Anchoritem not empty 
				//		set pointer to anchor item in list
				//		move pointer to anchoroffset
				//		while itemcount > 0 and not at list end
				//			output xml
				//			move pointer forward
				//			decrement itemcount
				auto pItem = TiVoFileList.begin();
				for (pItem = TiVoFileList.begin(); pItem != TiVoFileList.end(); pItem++)
					if (!pItem->GetURL().CompareNoCase(csAnchorItem))
					{
						std::wcout << L"[                   ] Anchoritem Found: " << csAnchorItem.GetString() << std::endl;
						break;
					}
				if (pItem == TiVoFileList.end())
				{
					std::wcout << L"[                   ] Anchoritem NOT Found: " << csAnchorItem.GetString() << std::endl;
					pItem = TiVoFileList.begin();
				}
				while ((pItem != TiVoFileList.begin()) && (pItem != TiVoFileList.end()) && (iAnchorOffset != 0))
					if (iAnchorOffset < 0)
					{
						pItem--;
						iAnchorOffset++;
					}
					else
					{
						pItem++;
						iAnchorOffset--;
					}
				CString csTemporary;
				csTemporary.Format(_T("%d"), pItem - TiVoFileList.begin());
				pWriter->WriteElementString(NULL,L"ItemStart",NULL, csTemporary.GetString());
				csTemporary.Format(_T("%d"), iItemCount);
				pWriter->WriteElementString(NULL,L"ItemCount",NULL, csTemporary.GetString());
				pWriter->WriteStartElement(NULL,L"Details",NULL);
					pWriter->WriteElementString(NULL,L"Title",NULL, csMyHostName.GetString());
					pWriter->WriteElementString(NULL,L"ContentType",NULL, L"x-tivo-container/folder");
					pWriter->WriteElementString(NULL,L"SourceFormat",NULL, L"x-tivo-container/folder");
					csTemporary.Format(_T("%d"), TiVoFileList.size());
					pWriter->WriteElementString(NULL,L"TotalItems",NULL, csTemporary.GetString());
				pWriter->WriteEndElement();
				while ((pItem != TiVoFileList.end()) && (iItemCount > 0))
				{
					std::wcout << L"[                   ] Item: " << pItem->GetPathName().GetString() << std::endl;
					pItem->GetTiVoItem(pWriter);
					pItem++;
					iItemCount--;
				}
				ccTiVoFileListCritSec.Unlock();
			}
			pWriter->WriteEndElement();	// TiVoContainer
		pWriter->WriteComment(L" Copyright © 2013 William C Bonner ");
		pWriter->WriteEndDocument();
		pWriter->Flush();

		// Allocates enough memeory for the xml content.
		STATSTG ssStreamData = {0};
		spMemoryStream->Stat(&ssStreamData, STATFLAG_NONAME);
		SIZE_T cbSize = ssStreamData.cbSize.LowPart;
		if (cbSize >= XMLDataBuffSize)
		{
			delete[] XMLDataBuff;
			XMLDataBuff = new char[cbSize+1];
		}
		// Copies the content from the stream to the buffer.
		LARGE_INTEGER position;
		position.QuadPart = 0;
		spMemoryStream->Seek(position, STREAM_SEEK_SET, NULL);
		ULONG cbRead;
		spMemoryStream->Read(XMLDataBuff, cbSize, &cbRead);
		XMLDataBuff[cbSize] = '\0';
	}
	/* Create HTTP Header */
	std::stringstream HttpResponse;
	HttpResponse << "HTTP/1.1 200 OK\r\n";
	HttpResponse << "Content-Type: text/xml\r\n";
	HttpResponse << "Connection: close\r\n";
	HttpResponse << "Date: " << getTimeRFC1123() << "\r\n";
	HttpResponse << "Content-Length: " << strlen(XMLDataBuff) << "\r\n";
	HttpResponse << "\r\n";
	send(DataSocket, HttpResponse.str().c_str(), HttpResponse.str().length(),0);
	send(DataSocket, XMLDataBuff, strlen(XMLDataBuff), 0);
	delete[] XMLDataBuff;

#ifdef _DEBUG
	std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\texiting" << endl;
#endif
	return(0);
}
int GetTiVoTVBusQuery(SOCKET DataSocket, const char * InBuffer)
{
	TRACE("%s %s\n", CStringA(CTime::GetCurrentTime().Format(_T("[%Y-%m-%dT%H:%M:%S]"))).GetString(), __FUNCTION__);
	#ifdef _DEBUG
	CStringA csInBuffer(InBuffer);
	int pos = csInBuffer.FindOneOf("\r\n");
	if (pos > 0)
		csInBuffer.Delete(pos,csInBuffer.GetLength());
	struct sockaddr_in adr_inet;/* AF_INET */  
	int sa_len = sizeof(adr_inet);
	getpeername(DataSocket, (struct sockaddr *)&adr_inet, &sa_len);
	std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\t" << inet_ntoa(adr_inet.sin_addr) << " " << csInBuffer.GetString() << endl;
	#endif
	int rval = 0;
	CString csUrl;
	CString csCommand(InBuffer);
	int curPos = 0;
	CString csToken(csCommand.Tokenize(_T("& "),curPos));
	while (csToken != _T(""))
	{
		if (!csToken.Left(4).CompareNoCase(_T("Url=")))
		{
			csToken.Delete(0,4);
			csUrl = csToken;
			TCHAR lpszBuffer[_MAX_PATH];
			DWORD dwBufferLength = sizeof(lpszBuffer) / sizeof(TCHAR);
			InternetCanonicalizeUrl(csUrl.GetString(), lpszBuffer, &dwBufferLength, ICU_DECODE);
			csUrl = CString(lpszBuffer, dwBufferLength);
			break; // minor optimization that helps in debugging
		}
		csToken = csCommand.Tokenize(_T("& "),curPos);
	}

	char XMLDataBuff[1024*11] = {0};
	if (!csUrl.Right(5).CompareNoCase(_T(".tivo")))
	{
		// This is just demo code making sure I can create a temporary file properly for future use with tdcat or tivodecode
		TCHAR lpTempPathBuffer[MAX_PATH];
		DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
		if (dwRetVal > MAX_PATH || (dwRetVal == 0))
		{
			std::cout << "[" << getTimeISO8601() << "] GetTempPath failed" << endl;
			_tcscpy(lpTempPathBuffer,_T("."));
		}
		//  Generates a temporary file name. 
		TCHAR szTempFileName[MAX_PATH];  
		UINT uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
			AfxGetAppName(),	// temp file name prefix 
			0,					// create unique name 
			szTempFileName);	// buffer for name 
		if (uRetVal == 0)
			std::cout << "[" << getTimeISO8601() << "] GetTempFileName failed" << endl;
		else
		{
			wcout << L"[                   ]  GetTempFileName: " << szTempFileName << endl;
			csUrl.Delete(0,csUrlPrefix.GetLength());
			while (0 < csUrl.Replace(_T("%20"),_T(" "))); // take care of spaces that are still encoded
			csUrl.Insert(0,_T("\""));
			csUrl.Append(_T("\""));
			if (-1 == _tspawnlp(_P_WAIT, _T("tdcat.exe"), _T("tdcat.exe"), _T("--mak"), _T("1760168186"), _T("--out"), szTempFileName, _T("--chunk-2"), csUrl.GetString(), NULL))
				std::cout << "[" << getTimeISO8601() << "] _tspawnlp failed: " << _sys_errlist[errno] << endl;
			std::ifstream FileToTransfer;
			FileToTransfer.open(CStringA(CString(szTempFileName)).GetString(), ios_base::in | ios_base::binary);
			if (FileToTransfer.good())
				FileToTransfer.read(XMLDataBuff, sizeof(XMLDataBuff)-1);
			DeleteFile(szTempFileName);	// I must delete this file because the zero in the unique field up above causes a file to be created.
		}
	}
	else
	{
		CComPtr<IXmlWriter> pWriter;
		CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), NULL);
		// from: http://stackoverflow.com/questions/3037946/how-can-i-store-xml-in-buffer-using-xmlite
		CComPtr<IStream> spMemoryStream(::SHCreateMemStream(NULL, 0));
		if ((pWriter != NULL) && (spMemoryStream != NULL))
		{
			HRESULT hr = S_OK;
			pWriter->SetProperty(XmlWriterProperty_ConformanceLevel, XmlConformanceLevel_Fragment);
			pWriter->SetOutput(spMemoryStream);
			pWriter->SetProperty(XmlWriterProperty_Indent, FALSE);
			pWriter->WriteStartDocument(XmlStandalone_Omit);
				//pWriter->WriteStartElement(NULL, L"TvBusEnvelope", L"http://tivo.com/developer/xml/idl/TvBusMarshalledStruct");
				//if (FAILED(hr = pWriter->WriteStartElement(L"TvBusMarshalledStruct", L"TvBusEnvelope", L"http://tivo.com/developer/xml/idl/TvBusMarshalledStruct TvBusMarshalledStruct.xsd")))
				//	std::cout << "Error, Method: WriteStartElement, error is " << hex << hr << dec <<std::endl;
				//pWriter->WriteAttributeString(L"xmlns", L"xs", NULL, L"http://www.w3.org/2001/XMLSchema-instance");
				//pWriter->WriteAttributeString(L"xmlns", L"TvBusMarshalledStruct", NULL, L"http://tivo.com/developer/xml/idl/TvBusMarshalledStruct");
				//pWriter->WriteAttributeString(L"xmlns", L"TvPgdRecording", NULL, L"http://tivo.com/developer/xml/idl/TvPgdRecording");
				//pWriter->WriteAttributeString(L"xmlns", L"TvBusDuration", NULL, L"http://tivo.com/developer/xml/idl/TvBusDuration");
				//pWriter->WriteAttributeString(L"xmlns", L"TvPgdShowing", NULL, L"http://tivo.com/developer/xml/idl/TvPgdShowing");
				//pWriter->WriteAttributeString(L"xmlns", L"TvDbShowingBit", NULL, L"http://tivo.com/developer/xml/idl/TvDbShowingBit");
				//pWriter->WriteAttributeString(L"xmlns", L"TvBusDateTime", NULL, L"http://tivo.com/developer/xml/idl/TvBusDateTime");
				//pWriter->WriteAttributeString(L"xmlns", L"TvPgdProgram", NULL, L"http://tivo.com/developer/xml/idl/TvPgdProgram");
				//pWriter->WriteAttributeString(L"xmlns", L"TvDbColorCode", NULL, L"http://tivo.com/developer/xml/idl/TvDbColorCode");
				//pWriter->WriteAttributeString(L"xmlns", L"TvPgdSeries", NULL, L"http://tivo.com/developer/xml/idl/TvPgdSeries");
				//pWriter->WriteAttributeString(L"xmlns", L"TvDbShowType", NULL, L"http://tivo.com/developer/xml/idl/TvDbShowType");
				//pWriter->WriteAttributeString(L"xmlns", L"TvPgdChannel", NULL, L"http://tivo.com/developer/xml/idl/TvPgdChannel");
				//pWriter->WriteAttributeString(L"xmlns", L"TvDbTvRating", NULL, L"http://tivo.com/developer/xml/idl/TvDbTvRating");
				//pWriter->WriteAttributeString(L"xmlns", L"TvDbRecordQuality", NULL, L"http://tivo.com/developer/xml/idl/TvDbRecordQuality");
				//pWriter->WriteAttributeString(L"xmlns", L"TvDbBitstreamFormat", NULL, L"http://tivo.com/developer/xml/idl/TvDbBitstreamFormat");
				//pWriter->WriteAttributeString(L"xmlns", L"schemaLocation", NULL, L"http://tivo.com/developer/xml/idl/TvBusMarshalledStruct TvBusMarshalledStruct.xsd http://tivo.com/developer/xml/idl/TvPgdRecording TvPgdRecording.xsd http://tivo.com/developer/xml/idl/TvBusDuration TvBusDuration.xsd http://tivo.com/developer/xml/idl/TvPgdShowing TvPgdShowing.xsd http://tivo.com/developer/xml/idl/TvDbShowingBit TvDbShowingBit.xsd http://tivo.com/developer/xml/idl/TvBusDateTime TvBusDateTime.xsd http://tivo.com/developer/xml/idl/TvPgdProgram TvPgdProgram.xsd http://tivo.com/developer/xml/idl/TvDbColorCode TvDbColorCode.xsd http://tivo.com/developer/xml/idl/TvPgdSeries TvPgdSeries.xsd http://tivo.com/developer/xml/idl/TvDbShowType TvDbShowType.xsd http://tivo.com/developer/xml/idl/TvPgdChannel TvPgdChannel.xsd http://tivo.com/developer/xml/idl/TvDbTvRating TvDbTvRating.xsd http://tivo.com/developer/xml/idl/TvDbRecordQuality TvDbRecordQuality.xsd http://tivo.com/developer/xml/idl/TvDbBitstreamFormat TvDbBitstreamFormat.xsd");
				//pWriter->WriteAttributeString(L"xmlns", L"type", NULL, L"TvPgdRecording:TvPgdRecording");
				ccTiVoFileListCritSec.Lock();
				for (auto MyFile = TiVoFileList.begin(); MyFile != TiVoFileList.end(); MyFile++)
					if (!MyFile->GetURL().CompareNoCase(csUrl))
					{
						MyFile->GetTvBusEnvelope(pWriter);
						break;
					}
				ccTiVoFileListCritSec.Unlock();
				//pWriter->WriteFullEndElement();
			//pWriter->WriteComment(L" Copyright © 2013 William C Bonner ");
			//pWriter->WriteEndDocument();
			pWriter->Flush();

			// Allocates enough memeory for the xml content.
			STATSTG ssStreamData = {0};
			spMemoryStream->Stat(&ssStreamData, STATFLAG_NONAME);
			SIZE_T cbSize = ssStreamData.cbSize.LowPart;
			if (cbSize >= sizeof(XMLDataBuff))
				cbSize = sizeof(XMLDataBuff)-1;
			// Copies the content from the stream to the buffer.
			LARGE_INTEGER position;
			position.QuadPart = 0;
			spMemoryStream->Seek(position, STREAM_SEEK_SET, NULL);
			ULONG cbRead;
			spMemoryStream->Read(XMLDataBuff, cbSize, &cbRead);
			XMLDataBuff[cbSize] = '\0';
		}
	}
	/* Create HTTP Header */
	std::stringstream HttpResponse;
	HttpResponse << "HTTP/1.1 200 OK\r\n";
	HttpResponse << "Content-Type: text/xml\r\n";
	HttpResponse << "Connection: close\r\n";
	HttpResponse << "Date: " << getTimeRFC1123() << "\r\n";
	HttpResponse << "Content-Length: " << strlen(XMLDataBuff) << "\r\n";
	HttpResponse << "\r\n";
	send(DataSocket, HttpResponse.str().c_str(), HttpResponse.str().length(),0);
	send(DataSocket, XMLDataBuff, strlen(XMLDataBuff),0);
	#ifdef _DEBUG
	std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\t" << inet_ntoa(adr_inet.sin_addr) << " Content-Length: " << strlen(XMLDataBuff) << endl;
	#endif
	return(0);
}
int GetFile(SOCKET DataSocket, const char * InBuffer)
{
	TRACE("%s %s\n", CStringA(CTime::GetCurrentTime().Format(_T("[%Y-%m-%dT%H:%M:%S]"))).GetString(), __FUNCTION__);
	#ifdef _DEBUG
	CStringA csInBuffer(InBuffer);
	int pos = csInBuffer.FindOneOf("\r\n");
	if (pos > 0)
		csInBuffer.Delete(pos,csInBuffer.GetLength());
	struct sockaddr_in adr_inet;/* AF_INET */  
	int sa_len = sizeof(adr_inet);
	getpeername(DataSocket, (struct sockaddr *)&adr_inet, &sa_len);
	std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\t" << inet_ntoa(adr_inet.sin_addr) << " " << csInBuffer.GetString() << endl;
	#endif
	int rval = 0;

	cTiVoFile TiVoFileToSend;

	CString csCommand(InBuffer);
	if (0 < csCommand.FindOneOf(_T("\r\n")))
		csCommand.Delete(csCommand.FindOneOf(_T("\r\n")),csCommand.GetLength());

	int curPos = 0;
	CString csToken(csCommand.Tokenize(_T("&? "),curPos));
	while (csToken != _T(""))
	{
		if (!csUrlPrefix.Compare(csToken.Left(csUrlPrefix.GetLength())))
		{
			ccTiVoFileListCritSec.Lock();
			for (auto MyFile = TiVoFileList.begin(); MyFile != TiVoFileList.end(); MyFile++)
				if (!MyFile->GetURL().CompareNoCase(csToken))
				{
					std::wcout << L"[                   ] Found File: " << MyFile->GetPathName().GetString() << std::endl;
					TiVoFileToSend = *MyFile;
					break;
				}
			ccTiVoFileListCritSec.Unlock();			
			if (TiVoFileToSend.GetSourceSize() == 0)
				std::wcout << L"[                   ] Not Found File: " << csToken.GetString() << std::endl;
		}
		csToken = csCommand.Tokenize(_T("&? "),curPos);
	}
	if (TiVoFileToSend.GetSourceSize() == 0)
		send(DataSocket,"HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n",41,0);
	else
	{
		/* Create HTTP Header */
		std::stringstream HttpResponse;
		//HttpResponse << "HTTP/1.1 200 OK\r\n";
		HttpResponse << "HTTP/1.1 206 Partial Content\r\n";
		HttpResponse << "Server: Wims_TiVo_Server/1.0.0.1\r\n";
		HttpResponse << "Date: " << getTimeRFC1123() << "\r\n";
		HttpResponse << "Transfer-Encoding: chunked\r\n";
		if (TiVoFileToSend.GetDuration() > 0) HttpResponse << "TiVo-Estimated-Length: " << max(TiVoFileToSend.GetSourceSize(), (TiVoFileToSend.GetDuration() * 1024)) << "\r\n"; // Since the Duration is 1/1000 of a second, and at least one example I transferred came in at roughly this multiple, 700bytes for every millisecond, worth a try.
		HttpResponse << "Content-Type: video/x-tivo-mpeg\r\n";
		HttpResponse << "Connection: close\r\n";
		HttpResponse << "\r\n";
		send(DataSocket, HttpResponse.str().c_str(), HttpResponse.str().length(),0);

		if (!TiVoFileToSend.GetPathName().Right(5).CompareNoCase(_T(".tivo")))
		{
			int nRet;
			std::ifstream FileToTransfer;
			FileToTransfer.open(CStringA(TiVoFileToSend.GetPathName()).GetString(), ios_base::in | ios_base::binary);
			if (FileToTransfer.good())
			{
				std::cout << "[                   ] Sending File: " << CStringA(TiVoFileToSend.GetPathName()).GetString() << endl;
				std::cout << HttpResponse.str() << endl;
				bool bSoFarSoGood = true;
				long long bytessent = 0;
				char * RAWDataBuff = new char[0x400000];
				std::stringstream ssChunkHeader;
				while (!FileToTransfer.eof() && (bSoFarSoGood))
				{
					FileToTransfer.read(RAWDataBuff, 0x400000);
					int BytesToSendInBuffer = FileToTransfer.gcount();
					int offset = 0;
					ssChunkHeader.str("");
					ssChunkHeader << hex << (BytesToSendInBuffer-offset) << "\r\n";
					send(DataSocket, ssChunkHeader.str().c_str(), ssChunkHeader.str().length(), 0);
					nRet = send(DataSocket, RAWDataBuff+offset, BytesToSendInBuffer-offset, 0);
					bytessent += nRet;
					bSoFarSoGood = nRet == BytesToSendInBuffer;
					ssChunkHeader.str("");
					ssChunkHeader << "\r\n";
					send(DataSocket, ssChunkHeader.str().c_str(), ssChunkHeader.str().length(), 0);
				}
				delete[] RAWDataBuff;
				std::cout << "[                   ] Finished Sending File, bSoFarSoGood=" << boolalpha << bSoFarSoGood << " BytesSent(" << bytessent << ")" << endl;
				ssChunkHeader.str("");
				ssChunkHeader << hex << 0 << "\r\n\r\n";	// 0\r\n is last chunk ending, and \r\n is the trailer.
				send(DataSocket, ssChunkHeader.str().c_str(), ssChunkHeader.str().length(), 0);
			}
		}
		else
		{
			char XMLDataBuff[1024*11] = {0};
			CComPtr<IXmlWriter> pWriter;
			CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), NULL);
			CComPtr<IStream> spMemoryStream(::SHCreateMemStream(NULL, 0));
			if ((pWriter != NULL) && (spMemoryStream != NULL))
			{
				pWriter->SetProperty(XmlWriterProperty_ConformanceLevel, XmlConformanceLevel_Fragment);
				pWriter->SetOutput(spMemoryStream);
				pWriter->SetProperty(XmlWriterProperty_Indent, FALSE);
				pWriter->WriteStartDocument(XmlStandalone_Omit);
				TiVoFileToSend.GetTvBusEnvelope(pWriter);
				pWriter->Flush();

				// Allocates enough memeory for the xml content.
				STATSTG ssStreamData = {0};
				spMemoryStream->Stat(&ssStreamData, STATFLAG_NONAME);
				SIZE_T cbSize = ssStreamData.cbSize.LowPart;
				if (cbSize >= sizeof(XMLDataBuff))
					cbSize = sizeof(XMLDataBuff)-1;
				// Copies the content from the stream to the buffer.
				LARGE_INTEGER position;
				position.QuadPart = 0;
				spMemoryStream->Seek(position, STREAM_SEEK_SET, NULL);
				ULONG cbRead;
				spMemoryStream->Read(XMLDataBuff, cbSize, &cbRead);
				XMLDataBuff[cbSize] = '\0';
			}
#pragma pack(show)
#pragma pack(2)
#pragma pack(show)
			auto ld = strlen(XMLDataBuff);
			auto chunklen = ld * 2 + 44;
			auto padding = 2048 - chunklen % 1024;
			#define SIZEOF_STREAM_HEADER 16
			struct {
				char           filetype[4];       /* the string 'TiVo' */
				/* all fields are in network byte order */
				unsigned short dummy_0004;
				unsigned short dummy_0006;
				unsigned short dummy_0008;
				unsigned int   mpeg_offset;   /* 0-based offset of MPEG stream */
				unsigned short chunks;        /* Number of metadata chunks */
			} tivo_stream_header;
			ASSERT(sizeof(tivo_stream_header) == SIZEOF_STREAM_HEADER);
			std::string("TiVo").copy(tivo_stream_header.filetype, 4);
			tivo_stream_header.dummy_0004 = htons(4);
			tivo_stream_header.dummy_0006 = htons(13); // mime = video/x-tivo-mpeg so flag is 13. If mime = video/x-tivo-mpeg-ts, flag would be 45
			tivo_stream_header.dummy_0008 = htons(0);
			tivo_stream_header.mpeg_offset = htonl(padding + chunklen);
			tivo_stream_header.chunks = htons(2);
			#define SIZEOF_STREAM_CHUNK 12
			struct {
				unsigned int   chunk_size;    /* Size of chunk */
				unsigned int   data_size;     /* Length of the payload */
				unsigned short id;            /* Chunk ID */
				unsigned short type;          /* Subtype */
				//unsigned char  data[1];       /* Variable length data */
			} tivo_stream_chunk;
			ASSERT(sizeof(tivo_stream_chunk) == SIZEOF_STREAM_CHUNK);
			tivo_stream_chunk.chunk_size = htonl(ld + sizeof(tivo_stream_chunk) + 4);
			tivo_stream_chunk.data_size = htonl(ld);
			tivo_stream_chunk.id = htons(1);
			tivo_stream_chunk.type = htons(0);
			struct {
				unsigned int   chunk_size;    /* Size of chunk */
				unsigned int   data_size;     /* Length of the payload */
				unsigned short id;            /* Chunk ID */
				unsigned short type;          /* Subtype */
				//unsigned char  data[1];       /* Variable length data */
			} tivo_stream_chunk2;
			ASSERT(sizeof(tivo_stream_chunk) == SIZEOF_STREAM_CHUNK);
			tivo_stream_chunk2.chunk_size = htonl(ld + sizeof(tivo_stream_chunk2) + 7);
			tivo_stream_chunk2.data_size = htonl(ld);
			tivo_stream_chunk2.id = htons(2);
			tivo_stream_chunk2.type = htons(0);
			auto TiVoChunkBufferSize = sizeof(tivo_stream_header) + sizeof(tivo_stream_chunk) + strlen(XMLDataBuff) + 4 + sizeof(tivo_stream_chunk2) + strlen(XMLDataBuff) + padding;
			char * TiVoChunkBuffer = new char[TiVoChunkBufferSize];
			char * pTiVoChunkBuffer = TiVoChunkBuffer;
			memcpy(pTiVoChunkBuffer, &tivo_stream_header, sizeof(tivo_stream_header));
			pTiVoChunkBuffer += sizeof(tivo_stream_header);
			memcpy(pTiVoChunkBuffer, &tivo_stream_chunk, sizeof(tivo_stream_chunk));
			pTiVoChunkBuffer += sizeof(tivo_stream_chunk);
			memcpy(pTiVoChunkBuffer, XMLDataBuff, strlen(XMLDataBuff));
			pTiVoChunkBuffer += strlen(XMLDataBuff);
			for (auto index = 0; index < 4; index++)
				*(++pTiVoChunkBuffer) = '\0';
			memcpy(pTiVoChunkBuffer, &tivo_stream_chunk2, sizeof(tivo_stream_chunk2));
			pTiVoChunkBuffer += sizeof(tivo_stream_chunk2);
			memcpy(pTiVoChunkBuffer, XMLDataBuff, strlen(XMLDataBuff));
			pTiVoChunkBuffer += strlen(XMLDataBuff);
			for (auto index = 1; index < padding; index++)
				*(++pTiVoChunkBuffer) = '\0';
			std::stringstream ssChunkHeader;
			ssChunkHeader << hex << TiVoChunkBufferSize << "\r\n";
			send(DataSocket, ssChunkHeader.str().c_str(), ssChunkHeader.str().length(), 0);
			send(DataSocket, TiVoChunkBuffer, TiVoChunkBufferSize, 0);
			send(DataSocket, "\r\n", 2, 0);	// Chunk Footer
			delete[] TiVoChunkBuffer;
#pragma pack()
#pragma pack(show)

			// Set the bInheritHandle flag so pipe handles are inherited. 
			SECURITY_ATTRIBUTES saAttr;  
			saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
			saAttr.bInheritHandle = TRUE; 
			saAttr.lpSecurityDescriptor = NULL; 

			// Create a pipe for the child process's STDOUT. 
			HANDLE g_hChildStd_OUT_Rd = NULL;
			HANDLE g_hChildStd_OUT_Wr = NULL;
			if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0x800000) ) 
				std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\t ERROR: StdoutRd CreatePipe" << endl;
			else
			{
				// Ensure the read handle to the pipe for STDOUT is not inherited.
				if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
					std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\t ERROR: Stdout SetHandleInformation" << endl;
				else
				{
					// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682512(v=vs.85).aspx is the simple version of CreateProcess
					// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx is the CreateProcess example with redirection of stdout and stdin.
					// Create a child process that uses the previously created pipes for STDIN and STDOUT.
					// Set up members of the PROCESS_INFORMATION structure.  
					PROCESS_INFORMATION piProcInfo; 
					ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
					// Set up members of the STARTUPINFO structure. 
					// This structure specifies the STDIN and STDOUT handles for redirection.
					STARTUPINFO siStartInfo;
					ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
					siStartInfo.cb = sizeof(STARTUPINFO); 
					siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
					siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
					siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
					siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
 
					static const CString csFFMPEGPath(FindEXEFromPath(_T("ffmpeg.exe")));
					CString csCommandLine(TiVoFileToSend.GetFFMPEGCommandLine(csFFMPEGPath));
					std::cout << "[" << getTimeISO8601() << "] CreateProcess: ";
					std::wcout << csCommandLine.GetString() << std::endl;
					TRACE(_T("CreateProcess: %s\n"), csCommandLine.GetString());
					if (ApplicationLogHandle != NULL) 
					{
						CString csSubstitutionText(__FUNCTION__);
						csSubstitutionText.AppendFormat(_T("\nCreateProcess: %s\n"), csCommandLine.GetString());
						LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
						ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
					}
					// Create the child process.
					BOOL bSuccess = CreateProcess(NULL, 
						(LPTSTR) csCommandLine.GetString(),     // command line 
						NULL,          // process security attributes 
						NULL,          // primary thread security attributes 
						TRUE,          // handles are inherited 
						0,             // creation flags 
						NULL,          // use parent's environment 
						NULL,          // use parent's current directory 
						&siStartInfo,  // STARTUPINFO pointer 
						&piProcInfo);  // receives PROCESS_INFORMATION 
   
					// If an error occurs, exit the application. 
					if ( bSuccess ) 
					{
						CloseHandle(g_hChildStd_OUT_Wr);	// If I don't do this, then the parent will never exit!
						//BOOL on = 1;
						//if (SOCKET_ERROR != setsockopt(DataSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on)))	// Attempt to see if this is related to why many of my transfers are failing.
						//	TRACE("NAGLE Disabled on socket\n");
						//else
						//	TRACE("NAGLE NOT Disabled on socket\n");
						long long bytessent = 0;
						const int RAWDataBuffSize = 0x80000;	// 0x80000 is half a megabyte
						//const int RAWDataBuffSize = 0x800;	// 0x800 is 2k
						char * RAWDataBuff = new char[RAWDataBuffSize];
						CTime ctStart(CTime::GetCurrentTime());
						CTimeSpan ctsTotal = CTime::GetCurrentTime() - ctStart;
						unsigned long long CurrentFileSize = 0;
						unsigned long long ChunkCount = 0;
						int MaxChunkSize = 0;
						int MinChunkSize = INT_MAX;
						for (;;) 
						{ 
							DWORD dwRead = 0;
							BOOL bSuccess = ReadFile(g_hChildStd_OUT_Rd, RAWDataBuff, RAWDataBuffSize, &dwRead, NULL);
							if( (!bSuccess) || (dwRead == 0)) break; 
							MaxChunkSize = max(MaxChunkSize, dwRead);
							MinChunkSize = min(MinChunkSize, dwRead);
							if (DataSocket != INVALID_SOCKET) 
							{
								ChunkCount++;
								ssChunkHeader.str("");
								ssChunkHeader << hex << dwRead << "\r\n";
								int nRet = 0;
								send(DataSocket, ssChunkHeader.str().c_str(), ssChunkHeader.str().length(), 0);
								nRet = send(DataSocket, RAWDataBuff, dwRead, 0);
								if (SOCKET_ERROR == nRet)
								{
									TerminateProcess(piProcInfo.hProcess, 0);
									int errCode = WSAGetLastError();
									//std::cout << "[                   ] WSAGetLastError(): " << errCode << std::endl;
									// ..and the human readable error string!!
									// Interesting:  Also retrievable by net helpmsg 10060
									LPTSTR errString = NULL;  // will be allocated and filled by FormatMessage  
									int size = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // use windows internal message table
										0,			// 0 since source is internal message table
										errCode,	// this is the error code returned by WSAGetLastError()
													// Could just as well have been an error code from generic
													// Windows errors from GetLastError()
										MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// auto-determine language to use
										(LPTSTR)&errString, // this is WHERE we want FormatMessage
													// to plunk the error string.  Note the
													// peculiar pass format:  Even though
													// errString is already a pointer, we
													// pass &errString (which is really type LPSTR* now)
													// and then CAST IT to (LPSTR).  This is a really weird
													// trip up.. but its how they do it on msdn:
													// http://msdn.microsoft.com/en-us/library/ms679351(VS.85).aspx
										0,			// min size for buffer
										NULL );		// 0, since getting message from system tables
									std::cout << "\n\r[                   ] Error code: " << errCode << " " << CStringA(errString, size).Trim().GetString() << std::endl;
									LocalFree(errString);	// if you don't do this, you will get an
															// ever so slight memory leak, since we asked
															// FormatMessage to FORMAT_MESSAGE_ALLOCATE_BUFFER,
															// and it does so using LocalAlloc
															// Gotcha!  I guess.
									WSASetLastError(0);		// Reset this so that subsequent calls may be accurate
									break;
								}
								send(DataSocket, "\r\n", 2, 0);	// Chunk Footer
								bytessent += nRet;
								CurrentFileSize += nRet;
								ctsTotal = CTime::GetCurrentTime() - ctStart;
								// This is another experiment trying to find out why I'm failing to send files to the TiVo
								// I was initially going to use select() on the socket, but later decided that the ioctlsocket() call mould be simpler.
								// http://developerweb.net/viewtopic.php?id=2933
								//u_long iMode = 0;
								//if (SOCKET_ERROR != ioctlsocket(DataSocket, FIONREAD, &iMode))
								//	if (iMode > 0)
								//	{
								//		char *JunkBuffer = new char[iMode+1];
								//		recv(DataSocket, JunkBuffer, iMode, 0);
								//		JunkBuffer[iMode] = '\0';
								//		std::cout << "\n\r[                   ] Unexpected Stuff came from TiVo: " << JunkBuffer << std::endl;
								//		delete[] JunkBuffer;
								//	}
							}
						} 
						send(DataSocket, "0\r\n\r\n", 5, 0); // 0\r\n is last chunk ending, and \r\n is the trailer.
						delete[] RAWDataBuff;
						// Close handles to the child process and its primary thread.
						// Some applications might keep these handles to monitor the status
						// of the child process, for example. 
						CloseHandle(piProcInfo.hProcess);
						CloseHandle(piProcInfo.hThread);
						auto TotalSeconds = ctsTotal.GetTotalSeconds();
						std::stringstream ss;
						if (TotalSeconds > 0)
							ss << "[" << getTimeISO8601() << "] Finished Sending File, BytesSent(" << bytessent << ")" << " Speed: " << (CurrentFileSize / TotalSeconds) << " B/s, " << CStringA(ctsTotal.Format(_T("%H:%M:%S"))).GetString() << std::endl;
						else
							ss << "[" << getTimeISO8601() << "] Finished Sending File, BytesSent(" << bytessent << ")" << std::endl;
						ss << "[                   ] ChunkCount: " << ChunkCount << " AvgChunkSize: " << (CurrentFileSize / ChunkCount) << std::endl;
						ss << "[                   ] MaxChunkSize: " << MaxChunkSize << " MinChunkSize: " << MinChunkSize << std::endl;
						std::cout << ss.str().c_str();
						if (ApplicationLogHandle != NULL) 
						{
							CString csSubstitutionText(__FUNCTION__);
							csSubstitutionText.AppendFormat(_T("\nCreateProcess: %s\n"), csCommandLine.GetString());
							csSubstitutionText.Append(CString(ss.str().c_str()));
							LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
							ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
						}
					}
				}
				CloseHandle(g_hChildStd_OUT_Rd);
			}
		}
	}
	return(0);
}
UINT HTTPChild(LPVOID lvp)
{
	SOCKET remoteSocket = (SOCKET) lvp;
	if (SUCCEEDED(CoInitializeEx(0, COINIT_MULTITHREADED))) // COINIT_APARTMENTTHREADED
	{
		char InBuff[1024];
		int count = recv(remoteSocket,InBuff,sizeof(InBuff),0);
		InBuff[count] = '\0';
		if (strncmp(InBuff,"GET /TiVoConnect?Command=QueryContainer",39) == 0)
			GetTivoQueryContainer(remoteSocket, InBuff);
		else if (strncmp(InBuff,"GET /TiVoConnect?Command=QueryFormats",37) == 0)
			GetTiVoQueryFormats(remoteSocket, InBuff);
		else if (strncmp(InBuff,"GET /TiVoConnect?Command=TVBusQuery",35) == 0)
			GetTiVoTVBusQuery(remoteSocket, InBuff);
		else if (strncmp(InBuff,"GET /TiVoConnect/TivoNowPlaying/",32) == 0)
			GetFile(remoteSocket, InBuff);
		else
		{
			#ifdef _DEBUG
			CStringA csInBuffer(InBuff);
			int pos = csInBuffer.FindOneOf("\r\n");
			if (pos > 0)
				csInBuffer.Delete(pos,csInBuffer.GetLength());
			struct sockaddr_in adr_inet;/* AF_INET */  
			int sa_len = sizeof(adr_inet);
			getpeername(remoteSocket, (struct sockaddr *)&adr_inet, &sa_len);
			std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\t" << inet_ntoa(adr_inet.sin_addr) << "HTTP/1.1 404 Not Found\t" << csInBuffer.GetString() << endl;
			#endif
			int nRet = send(remoteSocket,"HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n",41,0);
		}
		CoUninitialize();
	}
	if (shutdown(remoteSocket, SD_SEND) == SOCKET_ERROR)
		TRACE("shutdown(remoteSocket, SD_SEND) == SOCKET_ERROR\n");
	closesocket(remoteSocket);
	return(0);
}
UINT HTTPMain(LPVOID lvp)
{
	HANDLE LocalTerminationEventHandle = lvp;
	if (ApplicationLogHandle != NULL) 
	{
		CString csSubstitutionText(__FUNCTION__);
		csSubstitutionText.Append(_T(" Has Started"));
		LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
		ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
	}
	if (AfxSocketInit())
	{
		/* Open a listening socket */
		//SOCKET ControlSocket = INVALID_SOCKET;
		ControlSocket = socket(AF_INET,	/* Address family */
							SOCK_STREAM,	/* Socket type */
							IPPROTO_TCP);	/* Protocol */
		// If I created this as an attempt to create non-overlapped sockets to see if that would fix my problem. 
		//ControlSocket = WSASocket(AF_INET,	/* Address family */
		//					SOCK_STREAM,	/* Socket type */
		//					IPPROTO_TCP,	/* Protocol */
		//					NULL,
		//					0,
		//					0); // This is creating a non-overlapped socket.
		if (ControlSocket == INVALID_SOCKET)
			printerr(_T("Fatal Error: Socket could not be created"));
		else
		{
			DWORD on = TRUE;
			//DWORD off = FALSE;
			//DWORD BigBuffSize = 0x8000;
			setsockopt(ControlSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
			//setsockopt(ControlSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&off, sizeof(off));	// Attempt to see if this is related to why many of my transfers are failing.
			//setsockopt(ControlSocket, SOL_SOCKET, SO_SNDBUF, (char *)&BigBuffSize, sizeof(BigBuffSize));	// Attempt to see if this is related to why many of my transfers are failing.
			//setsockopt(ControlSocket, SOL_SOCKET, SO_RCVBUF, (char *)&BigBuffSize, sizeof(BigBuffSize));	// Attempt to see if this is related to why many of my transfers are failing.
			//setsockopt(ControlSocket, IPPROTO_TCP, TCP_EXPEDITED_1122, (char *)&on, sizeof(on));	// Attempt to see if this is related to why many of my transfers are failing.
			//setsockopt(ControlSocket, IPPROTO_TCP, TCP_BSDURGENT, (char *)&on, sizeof(on));	// Attempt to see if this is related to why many of my transfers are failing.
			//setsockopt(ControlSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&off, sizeof(off));	// Attempt to see if this is related to why many of my transfers are failing.
			SOCKADDR_IN saServer;
			saServer.sin_family = AF_INET;
			saServer.sin_addr.s_addr = INADDR_ANY;	/* Let WinSock supply address */
			CString csRegKey(_T("Software\\WimsWorld\\"));
			csRegKey.Append(theApp.m_pszAppName);
			DWORD vData = 0;
			DWORD cbData = sizeof(vData);
			if (ERROR_SUCCESS == RegGetValue(HKEY_LOCAL_MACHINE, csRegKey, _T("TCPPort"), RRF_RT_REG_DWORD, NULL, &vData, &cbData))
				saServer.sin_port = htons(vData);
			else
				#ifdef _DEBUG
				saServer.sin_port = htons(64321);
				#else
				saServer.sin_port = htons(0);			/* Use unique port */
				#endif
			int nRet = bind(ControlSocket,			/* Socket */
						(LPSOCKADDR)&saServer,		/* Our address */
						sizeof(struct sockaddr));	/* Size of address structure */
			if (nRet == SOCKET_ERROR)
			{
				closesocket(ControlSocket);
				ControlSocket = INVALID_SOCKET;
				printerr(_T("Fatal Error: Socket could not be bound"));
			}
			else
			{
				struct sockaddr addr;
				addr.sa_family = AF_UNSPEC;
				socklen_t addr_len = sizeof(addr);
				if (ControlSocket != INVALID_SOCKET)
					getsockname(ControlSocket, &addr, &addr_len);
				if (addr.sa_family == AF_INET)
				{
					struct sockaddr_in * saServer = (sockaddr_in *)&addr;
					std::stringstream ss;
					ss << "TiVoMediaServer:" << ntohs(saServer->sin_port) << "/http";
					myServer.m_services = ss.str();
					long errCode;
					HKEY hKey;
					if (ERROR_SUCCESS == (errCode = RegCreateKeyEx(HKEY_LOCAL_MACHINE, csRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL)))
					{
						vData = ntohs(saServer->sin_port);
						cbData = sizeof(vData);
						RegSetValueEx(hKey, _T("TCPPort"), 0, REG_DWORD, (const BYTE *)vData, cbData);
						RegCloseKey(hKey);
					}
					else
					{
						LPTSTR errString = NULL;  // will be allocated and filled by FormatMessage  
						int size = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, // use windows internal message table
							0,			// 0 since source is internal message table
							errCode,	// this is the error code returned by WSAGetLastError()
										// Could just as well have been an error code from generic
										// Windows errors from GetLastError()
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// auto-determine language to use
							(LPTSTR)&errString, // this is WHERE we want FormatMessage
										// to plunk the error string.  Note the
										// peculiar pass format:  Even though
										// errString is already a pointer, we
										// pass &errString (which is really type LPSTR* now)
										// and then CAST IT to (LPSTR).  This is a really weird
										// trip up.. but its how they do it on msdn:
										// http://msdn.microsoft.com/en-us/library/ms679351(VS.85).aspx
							0,			// min size for buffer
							NULL );		// 0, since getting message from system tables
						std::stringstream ss;
						ss << "[" << getTimeISO8601() << "] " << __FUNCTION__ << " RegCreateKeyEx() Error code: " << errCode << " " << CStringA(errString, size).Trim().GetString() << std::endl;
						LocalFree(errString);	// if you don't do this, you will get an
												// ever so slight memory leak, since we asked
												// FormatMessage to FORMAT_MESSAGE_ALLOCATE_BUFFER,
												// and it does so using LocalAlloc
												// Gotcha!  I guess.
						TRACE(ss.str().c_str());
						std::cout << ss.str().c_str();
					}
				}
				/* Set the socket to listen */
				nRet = listen(ControlSocket, SOMAXCONN);
				if (nRet == SOCKET_ERROR)
				{
					closesocket(ControlSocket);
					ControlSocket = INVALID_SOCKET;
					printerr(_T("Fatal Error: Socket could not be set to listen"));
				}
				else 
				{
					while (ControlSocket != INVALID_SOCKET)
					{
						SOCKET remoteSocket = accept(ControlSocket,NULL,NULL);
						if (remoteSocket != INVALID_SOCKET)
							AfxBeginThread(HTTPChild, (LPVOID)remoteSocket);
					}
				}
			}
		}
	}
	SetEvent(LocalTerminationEventHandle);
	if (ApplicationLogHandle != NULL) 
	{
		CString csSubstitutionText(__FUNCTION__);
		csSubstitutionText.Append(_T(" Has Stopped"));
		LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
		ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
	}
	return(0);
}
bool TiVoBeaconSend(const std::string csServerBroadcast)
{
	bool rval = false;
	// Create a UDP/IP datagram socket
	SOCKET theSocket = socket(AF_INET,		// Address family
						SOCK_DGRAM,			// Socket type
						IPPROTO_UDP);		// Protocol
	if (theSocket == INVALID_SOCKET)
	{
		TRACE("%s: %d\n","socket()",WSAGetLastError());
	}
	else
	{
		BOOL bBroadcastSocket = TRUE;
		int nRet = setsockopt(theSocket, SOL_SOCKET, SO_BROADCAST, (const char *)&bBroadcastSocket, sizeof(bBroadcastSocket));
		if (nRet == SOCKET_ERROR) 
			TRACE("%s: %d\n","socket()",WSAGetLastError());
		else
		{
			SOCKADDR_IN saBroadCast;
			saBroadCast.sin_family = AF_INET;
			saBroadCast.sin_addr.S_un.S_addr = INADDR_BROADCAST;
			saBroadCast.sin_port = htons(2190);	// Port number
			nRet = sendto(theSocket,			// Socket
				csServerBroadcast.c_str(),		// Data buffer
				csServerBroadcast.length(),		// Length of data
				0,								// Flags
				(LPSOCKADDR)&saBroadCast,		// Server address
				sizeof(struct sockaddr));		// Length of address
			if (nRet == SOCKET_ERROR) 
				TRACE("%s: %d\n","socket()",WSAGetLastError());
			else
				rval = true;
		}
		closesocket(theSocket);
	}
	return(rval);
}
UINT TiVoBeaconSendThread(LPVOID lvp)
{
	HANDLE LocalTerminationEventHandle = lvp;
	if (ApplicationLogHandle != NULL) 
	{
		CString csSubstitutionText(__FUNCTION__);
		csSubstitutionText.Append(_T(" Has Started"));
		LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
		ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
	}
	DWORD dwVersion = GetVersion();
	int dwMajorVersion = (int)(LOBYTE(LOWORD(dwVersion)));
	int dwMinorVersion = (int)(HIBYTE(LOWORD(dwVersion)));
	int dwBuild = 0;
	if (dwVersion < 0x80000000)              
		dwBuild = (int)(HIWORD(dwVersion));

	TCHAR buffer[256] = TEXT("");
	DWORD dwSize = sizeof(buffer);
	GetComputerNameEx(ComputerNameDnsHostname, buffer, &dwSize);
	CString csMyHostName(buffer);

	const char * BuildDateTime = "<BuildDateTime>" __DATE__ " " __TIME__ "</BuildDateTime>";
	char cmonth[4];
	int year, day, hour, min, sec;
	sscanf(&(BuildDateTime[15]),"%s %d %d %d:%d:%d",cmonth,&day,&year,&hour,&min,&sec);
	const char * Months[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
	int month = 0;
	while (strcmp(cmonth,Months[month]) != 0)
		month++;
	month++;
	CString csBuildDateTime;
	csBuildDateTime.Format(_T("%04d%02d%02d%02d%02d%02d"),year,month,day,hour,min,sec);

	CString csMyProgramGuid;
	CString csRegKey(_T("Software\\WimsWorld\\"));
	csRegKey.Append(theApp.m_pszAppName);
	TCHAR vData[1024];
	DWORD cbData = sizeof(vData);
	if (ERROR_SUCCESS == RegGetValue(HKEY_LOCAL_MACHINE, csRegKey, _T("GUID"), RRF_RT_REG_SZ, NULL, vData, &cbData))
		csMyProgramGuid = CString(vData);
	if (csMyProgramGuid.IsEmpty())
	{
		GUID MyProgramGuid;
		if (SUCCEEDED(CoCreateGuid(&MyProgramGuid)))
		{
			OLECHAR MyProgramGuidString[40] = {0};
			StringFromGUID2(MyProgramGuid, MyProgramGuidString, sizeof(MyProgramGuidString) / sizeof(OLECHAR));
			csMyProgramGuid = CString(MyProgramGuidString);
			HKEY hKey;
			if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, csRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL))
			{
				DWORD cbData = (csMyProgramGuid.GetLength() + 1) * sizeof(TCHAR);
				RegSetValueEx(hKey, _T("GUID"), 0, REG_SZ, (const BYTE *)csMyProgramGuid.GetString(), cbData);
				RegCloseKey(hKey);
			}
		}
	}

	myServer.m_method = true?"broadcast":"connect";
	std::stringstream ss;
	ss << "pc/WinNT:" << dwMajorVersion << "." << dwMinorVersion << "." << dwBuild;
	myServer.m_platform = ss.str();
	myServer.m_machine = CStringA(csMyHostName).GetString();
	myServer.m_identity = CStringA(csMyProgramGuid).GetString();
	myServer.m_swversion = CStringA(csBuildDateTime).GetString();

	if (ApplicationLogHandle != NULL) 
	{
		CString csSubstitutionText(__FUNCTION__);
		csSubstitutionText.Append(_T(" TiVoBeacon: "));
		csSubstitutionText.Append(CString(myServer.WriteTXT().c_str()));
		LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
		ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
	}

	do {
		TiVoBeaconSend(myServer.WriteTXT('\n'));
	} while (WAIT_TIMEOUT == WaitForSingleObject(LocalTerminationEventHandle, 60*1000));
	if (ApplicationLogHandle != NULL) 
	{
		CString csSubstitutionText(__FUNCTION__);
		csSubstitutionText.Append(_T(" Has Stopped"));
		LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
		ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
	}
	return(0);
}
/////////////////////////////////////////////////////////////////////////////
BOOL SendStatusToSCM(DWORD dwCurrentState, 
					 DWORD dwWin32ExitCode, 
					 DWORD dwServiceSpecificExitCode,
					 DWORD dwCheckPoint,
					 DWORD dwWaitHint)
{
	SERVICE_STATUS service_Status;
	service_Status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	service_Status.dwCurrentState = dwCurrentState;
	if (service_Status.dwCurrentState == SERVICE_START_PENDING)
		service_Status.dwControlsAccepted = 0;
	else
		service_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
	if (dwServiceSpecificExitCode == 0)
		service_Status.dwWin32ExitCode = dwWin32ExitCode;
	else
		service_Status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	service_Status.dwServiceSpecificExitCode = dwServiceSpecificExitCode;
	service_Status.dwCheckPoint = dwCheckPoint;
	service_Status.dwWaitHint = dwWaitHint;
	BOOL success = SetServiceStatus(serviceStatusHandle,&service_Status);
	return(success);
}
DWORD WINAPI ServiceCtrlHandler(
  DWORD controlCode,
  DWORD dwEventType,
  LPVOID lpEventData,
  LPVOID lpContext)
{
	DWORD currentState = 0;
	BOOL success;
	CString csSubstitutionText;
	switch(controlCode)
	{
	case SERVICE_CONTROL_STOP:
		currentState = SERVICE_STOP_PENDING;
		success = SendStatusToSCM(SERVICE_STOP_PENDING, NO_ERROR, 0, 1, 5000);
		csSubstitutionText.Format(_T("Service %s is stopping"),theApp.m_pszAppName);
		closesocket(ControlSocket);
		ControlSocket = INVALID_SOCKET;
		//		SetEvent(terminateEvent_http);
		SetEvent(terminateEvent_populate);
		SetEvent(terminateEvent_beacon);
		break;
	case SERVICE_CONTROL_PAUSE:
		if (pauseService == false)
		{
			SendStatusToSCM(SERVICE_PAUSE_PENDING, NO_ERROR, 0, 0, 0);
			currentState = SERVICE_PAUSED;
			pauseService = true;
			threadHandle->SuspendThread();
			csSubstitutionText.Format(_T("Service %s is paused"),theApp.m_pszAppName);
		}
		SendStatusToSCM(currentState,NO_ERROR,0,0,0);
		break;
	case SERVICE_CONTROL_CONTINUE:
		if (pauseService == true)
		{
			SendStatusToSCM(SERVICE_CONTINUE_PENDING,NO_ERROR,0,1,1000);
			currentState = SERVICE_RUNNING;
			pauseService = false;
			threadHandle->ResumeThread();
			csSubstitutionText.Format(_T("Service %s has resumed"),theApp.m_pszAppName);
		}
		SendStatusToSCM(currentState,NO_ERROR,0,0,0);
		break;
	case SERVICE_CONTROL_INTERROGATE:
		SendStatusToSCM(currentState,NO_ERROR,0,0,0);
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		break;
	default:
		SendStatusToSCM(currentState,NO_ERROR,0,0,0);
		break;
	}
	//if ((ApplicationLogHandle != NULL) & (csSubstitutionText.GetLength() > 0))
	//{
	//	LPCTSTR lpStrings[] = { LPCTSTR(csSubstitutionText), NULL };
	//	ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
	//}
	return(NO_ERROR);
}
VOID ServiceMain(DWORD argc, LPTSTR * argv)
// I believe that ServiceMain is what gets run if the program is run as a
// service.  These comments are being added when it's been a year since 
// I've last looked at this code, so I'm remembering things and may need
// to correct them later.
// On further reading the process happens that _tmain is the starting 
// point in all cases, but if it is running as a service the control is 
// passed to this function by a call to the service manager.
// In any case the important work of this function is that it starts a thread 
// running the function HTTPMain().
{
	serviceStatusHandle = RegisterServiceCtrlHandlerEx(theApp.m_pszAppName, (LPHANDLER_FUNCTION_EX)ServiceCtrlHandler, NULL);
	if (serviceStatusHandle != 0)
	{
		BOOL success = SendStatusToSCM(SERVICE_START_PENDING, NO_ERROR, 0, 1, 5000);
		if (success == FALSE) // error
			SendStatusToSCM(SERVICE_STOPPED,GetLastError(),0,0,0);
		else
		{
			terminateEvent_http = CreateEvent(0,TRUE,FALSE,0);
			if (terminateEvent_http == NULL) // error
				SendStatusToSCM(SERVICE_STOPPED,GetLastError(),0,0,0);
			else
			{
				success = SendStatusToSCM(SERVICE_START_PENDING, NO_ERROR, 0, 2, 1000);
				if (success == FALSE) // error
				{
					CloseHandle(terminateEvent_http);
					SendStatusToSCM(SERVICE_STOPPED, GetLastError(), 0, 0, 0);
				}
				else
				{
					av_register_all(); // FFMPEG initialization
					ApplicationLogHandle = RegisterEventSource(NULL, theApp.m_pszAppName);
					terminateEvent_populate = CreateEvent(0,TRUE,FALSE,0);
					AfxBeginThread(PopulateTiVoFileList, terminateEvent_populate);
					terminateEvent_beacon = CreateEvent(0,TRUE,FALSE,0);
					AfxBeginThread(TiVoBeaconSendThread, terminateEvent_beacon);
					//threadHandle = AfxBeginThread(HTTPMain, NULL, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
					threadHandle = AfxBeginThread(HTTPMain, terminateEvent_http);
					if (threadHandle != NULL)
					{
						//threadHandle->m_bAutoDelete = false;
						//threadHandle->ResumeThread();
						success = SendStatusToSCM(SERVICE_RUNNING, NO_ERROR, 0, 0, 0);
						if (success != FALSE) 
						{
							if (ApplicationLogHandle != NULL) 
							{
								TCHAR UserNameBuff[256];
								DWORD UserNameSize = sizeof(UserNameBuff)/sizeof(TCHAR);
								GetUserName(UserNameBuff,&UserNameSize); // this is "wim" when I run it.
								CString csSubstitutionText;
								csSubstitutionText.Format(_T("Service %s has been started by %s"), theApp.m_pszAppName, UserNameBuff);
								LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
								ReportEvent(ApplicationLogHandle, EVENTLOG_INFORMATION_TYPE, 0, WIMSWORLD_EVENT_GENERIC, NULL, 1, 0, lpStrings, NULL);
							}
							WaitForSingleObject(terminateEvent_http, INFINITE);
							SetEvent(terminateEvent_populate);
							SetEvent(terminateEvent_beacon);
						}
						//if (threadHandle)
						//{
						//	delete threadHandle;
						//	threadHandle = NULL;
						//}
						if (serviceStatusHandle)
							SendStatusToSCM(SERVICE_STOPPED,GetLastError(),0,0,500);
					}
					if (terminateEvent_populate)
						CloseHandle(terminateEvent_populate);
					if (terminateEvent_beacon)
						CloseHandle(terminateEvent_beacon);
				}
				if (terminateEvent_http)
					CloseHandle(terminateEvent_http);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);
	if (hModule == NULL)
		std::cout << "[" << getTimeISO8601() << "] Fatal Error: GetModuleHandle failed " << ++nRetCode << endl;
	else if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0)) // initialize MFC and print and error on failure
		std::cout << "[" << getTimeISO8601() << "] Fatal Error: MFC initialization failed " << ++nRetCode << endl;
	else if (!AfxSocketInit())
		std::cout << "[" << getTimeISO8601() << "] Fatal Error: Sockets initialization failed " << ++nRetCode << endl;
	else
	{
		SERVICE_TABLE_ENTRY serviceTable[] =
		{
			//{ m_pszAppName, (LPSERVICE_MAIN_FUNCTION) ServiceMain },
			{ NULL, (LPSERVICE_MAIN_FUNCTION) ServiceMain },
			{ NULL, NULL }
		};
		serviceTable[0].lpServiceName = (LPWSTR) theApp.m_pszAppName;
		if (0 == StartServiceCtrlDispatcher(serviceTable))
			if (GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
		{
			// This error is returned if the program is being run as a console application rather than as a service. If the program will be run as a console application for debugging purposes, structure it such that service-specific code is not called when this error is returned. According to http://msdn.microsoft.com/en-us/library/windows/desktop/ms686324(v=vs.85).aspx ERROR_FAILED_SERVICE_CONTROLLER_CONNECT
			std::cout << "[" << getTimeISO8601() << "] Running Application from the command line." << endl;
			std::cout << "[" << getTimeISO8601() << "] Built on " << __DATE__ << " at " <<  __TIME__ << endl;
			CString Parameters(theApp.m_lpCmdLine);
			if (argc > 1)
				Parameters = argv[1];

			if (Parameters.CompareNoCase( _T("-?") ) == 0)
			{
				CString csBox;
				csBox += _T("Optional Parameters:\n");
				csBox += _T("\t-install\n");
				csBox += _T("\t-remove\n");
				csBox += _T("\t-ClearRegistry\n");
				_tprintf(csBox);
			}
			// install this program in the service control manager
			else if (Parameters.CompareNoCase( _T("-install") ) == 0)
			{
				SC_HANDLE scm = OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
				if (scm != NULL)
				{
					// Get the file name that we are running from.
					TCHAR tcModuleFileName[MAX_PATH];
					unsigned long buffersize = sizeof(tcModuleFileName)/sizeof(TCHAR);
					GetModuleFileName(AfxGetResourceHandle(), tcModuleFileName, buffersize );
					DWORD ModuleFileNameBytes = (_tcslen(tcModuleFileName) + 1) * sizeof(TCHAR);
					SC_HANDLE newService = CreateService(scm, theApp.m_pszAppName, _T("WimTiVoServer"), SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, tcModuleFileName, 0,0,0,0,0);
					if (newService != NULL)
					{
						// successfully installed the service
						// Add a description to the entry.
						SERVICE_DESCRIPTION servdesc;
						servdesc.lpDescription = _TEXT("WimsWorld TiVo Server");
						ChangeServiceConfig2(newService, SERVICE_CONFIG_DESCRIPTION, &servdesc);

						HKEY hKey;
						// With Server 2008 and above, Adding the DelayedAutostart key improves boot times since our process is not critical.
						CString csRegKey(_T("SYSTEM\\CurrentControlSet\\Services\\"));
						csRegKey.Append(theApp.m_pszAppName);
						if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, csRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL))
						{
							DWORD dwData = 1;
							RegSetValueEx(hKey, _T("DelayedAutostart"), 0, REG_DWORD, (LPBYTE) &dwData, sizeof(DWORD));
							RegCloseKey(hKey);
						}

						// Put Default Folder Locations in the registry
						csRegKey = _T("Software\\WimsWorld\\");
						csRegKey.Append(theApp.m_pszAppName);
						if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, csRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL))
						{
							TCHAR tcHostName[256] = TEXT("");
							DWORD dwSize = sizeof(tcHostName);
							GetComputerNameEx(ComputerNameDnsHostname, tcHostName, &dwSize);
							CString csContainers(_T("C:/Users/Public/Videos/*"));
							csContainers.AppendFormat(_T(";//%s/Recorded TV/*"), tcHostName);
							csContainers.AppendFormat(_T(";//%s/TiVo/*"), tcHostName);
							csContainers.AppendFormat(_T(";//%s/Videos/*"), tcHostName);
							DWORD cbData = (csContainers.GetLength() + 1) * sizeof(TCHAR);
							RegSetValueEx(hKey, _T("Container"), 0, REG_SZ, (const BYTE *)csContainers.GetString(), cbData);
							RegCloseKey(hKey);
						}

						// set up the event log stuff.
						// Add your source name as a subkey under the Application 
						// key in the EventLog registry key. 
						csRegKey = _T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
						csRegKey.Append(theApp.m_pszAppName);
						if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, csRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL))
						{
							// Add the name to the EventMessageFile subkey. 
							RegSetValueEx(hKey,		// subkey handle 
								_T("EventMessageFile"),	// value name 
								0,					// must be zero 
								REG_EXPAND_SZ,		// value type 
								(LPBYTE) tcModuleFileName,	// pointer to value data 
								ModuleFileNameBytes);// length of value data 
							// Set the supported event types in the TypesSupported subkey.
							DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
							RegSetValueEx(hKey,		// subkey handle 
								_T("TypesSupported"),	// value name 
								0,					// must be zero 
								REG_DWORD,			// value type 
								(LPBYTE) &dwData,	// pointer to value data 
								sizeof(DWORD));		// length of value data 
							RegCloseKey(hKey); 
							std::cout << "Sucessfully installed " << CStringA(tcModuleFileName).GetString() << " as a service." << std::endl;
							// I should add a rule to the Windows Firewall here so that the service is actually accessable without manually adding the rule.
							// http://msdn.microsoft.com/en-us/library/windows/desktop/dd339604(v=vs.85).aspx

							// Here I atempt to write a message to the event log
							HANDLE h = RegisterEventSource(NULL, theApp.m_pszAppName);
							if (h != NULL) 
							{
								CString csSubstitutionText(tcModuleFileName);
								LPCTSTR lpStrings[] = { LPCTSTR(csSubstitutionText), NULL };
								ReportEvent(h,			// event log handle 
									EVENTLOG_INFORMATION_TYPE,// event type 
									0,					// category zero 
									WIMSWORLD_EVENT_INSTALL,// event identifier 
									NULL,				// no user security identifier 
									1,					// one substitution string 
									0,					// no data 
									lpStrings,			// pointer to string array 
									NULL);				// pointer to data 
								DeregisterEventSource(h);
							}
						}
						CloseServiceHandle(newService);
					}
					CloseServiceHandle(scm);
				}
			}
			// remove this program from the service control manager
			else if (Parameters.CompareNoCase( _T("-remove") ) == 0)
			{
				SC_HANDLE scm = OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
				if (scm != NULL)
				{
					SC_HANDLE theService = OpenService(
						scm,
						theApp.m_pszAppName,
						SERVICE_ALL_ACCESS | DELETE);
					if (theService != NULL)
					{
						SERVICE_STATUS status;
						if (QueryServiceStatus(theService,&status))
						{
							if (status.dwCurrentState != SERVICE_STOPPED)
							{
								ControlService(theService,SERVICE_CONTROL_STOP,&status);
								Sleep(5000);
							}
							DeleteService(theService);
							// Here I atempt to write a message
							HANDLE h = RegisterEventSource(NULL,  // uses local computer 
										theApp.m_pszAppName);	// source name 
							if (h != NULL) 
							{
								CString csSubstitutionText(theApp.m_pszAppName);
								LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
								ReportEvent(h,			// event log handle 
									EVENTLOG_INFORMATION_TYPE,// event type 
									0,					// category zero 
									WIMSWORLD_EVENT_REMOVE,	// event identifier 
									NULL,				// no user security identifier 
									1,					// one substitution string 
									0,					// no data 
									lpStrings,			// pointer to string array 
									NULL);				// pointer to data 
								DeregisterEventSource(h);
							}
							// remove the ability to write to the application log
							CString csRegKey(_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"));
							csRegKey.Append(theApp.m_pszAppName);
							RegDeleteKey(HKEY_LOCAL_MACHINE,csRegKey);
							_tprintf(_T("Sucessfully removed the service %s\n"),theApp.m_pszAppName);
						}
						CloseServiceHandle(theService);
					}
					CloseServiceHandle(scm);
				}
			}
			else if (Parameters.CompareNoCase( _T("-ClearRegistry") ) == 0)
			{
				std::cout << "Removing Registry Entries" << std::endl;
				CString csRegKey;
				csRegKey.Format(_T("Software\\WimsWorld\\%s"), theApp.m_pszAppName);
				theApp.DelRegTree(HKEY_CURRENT_USER, csRegKey);
				theApp.DelRegTree(HKEY_LOCAL_MACHINE, csRegKey);
			}
			else
			{			
				bConsoleExists = true;
				av_register_all(); // FFMPEG initialization

				terminateEvent_populate = CreateEvent(0,TRUE,FALSE,0);
				AfxBeginThread(PopulateTiVoFileList, terminateEvent_populate);

				ccTiVoFileListCritSec.Lock();
				std::sort(TiVoFileList.begin(),TiVoFileList.end(),cTiVoFileCompareDate);
				std::cout << "[" << getTimeISO8601() << "] TiVoFileList Size: " << TiVoFileList.size() << endl;
				ccTiVoFileListCritSec.Unlock();

				terminateEvent_http = CreateEvent(0,TRUE,FALSE,0);
				if (terminateEvent_http != NULL) 
				{
					threadHandle = AfxBeginThread(HTTPMain, terminateEvent_http);
					if (threadHandle != NULL)
					{
						terminateEvent_beacon = CreateEvent(0,TRUE,FALSE,0);
						AfxBeginThread(TiVoBeaconSendThread, terminateEvent_beacon);
						#ifdef _DEBUG
						std::cout << "[" << getTimeISO8601() << "] Running for 30 minutes" << std::endl;
						Sleep(30 * 60 * 1000); // Sleep 30 minutes
						#else
						std::cout << "[" << getTimeISO8601() << "] Running for 12 hours" << std::endl;
						Sleep(12 * 60 * 60 * 1000); // Sleep 12 hours
						#endif
						SetEvent(terminateEvent_beacon);
						SetEvent(terminateEvent_populate);
						closesocket(ControlSocket);
						ControlSocket = INVALID_SOCKET;
						TRACE(__FUNCTION__ " Waiting for Thread to end\n");
						WaitForSingleObject(terminateEvent_http,INFINITE);
						if (terminateEvent_beacon)
							CloseHandle(terminateEvent_beacon);
					}
					if (terminateEvent_http)
						CloseHandle(terminateEvent_http);
				}
				if (terminateEvent_populate)
					CloseHandle(terminateEvent_populate);
			}
			std::cout << "[" << getTimeISO8601() << "] Exiting" << std::endl;
		}
	}
	TRACE("%s %s Exiting\n", CStringA(CTime::GetCurrentTime().Format(_T("[%Y-%m-%dT%H:%M:%S]"))).GetString(), __FUNCTION__);
	return nRetCode;
}
