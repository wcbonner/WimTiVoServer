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
HANDLE terminateEvent = NULL;
SERVICE_STATUS_HANDLE serviceStatusHandle = NULL;
bool pauseService = false;
CWinThread * threadHandle = NULL;
SOCKET ControlSocket = INVALID_SOCKET;
bool bConsoleExists = false;
HANDLE ApplicationLogHandle = NULL;

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
HRESULT WriteAttributes(IXmlReader* pReader) 
{ 
    const WCHAR* pwszPrefix; 
    const WCHAR* pwszLocalName; 
    const WCHAR* pwszValue; 
    HRESULT hr = pReader->MoveToFirstAttribute(); 
 
    if (S_FALSE == hr) 
        return hr; 
    if (S_OK != hr) 
    { 
        wprintf(L"Error moving to first attribute, error is %08.8lx", hr); 
        return hr; 
    } 
    else 
    { 
        while (TRUE) 
        { 
            if (!pReader->IsDefault()) 
            { 
                UINT cwchPrefix; 
                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) 
                { 
                    wprintf(L"Error getting prefix, error is %08.8lx", hr); 
                    return hr; 
                } 
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) 
                { 
                    wprintf(L"Error getting local name, error is %08.8lx", hr); 
                    return hr; 
                } 
                if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
                { 
                    wprintf(L"Error getting value, error is %08.8lx", hr); 
                    return hr; 
                } 
                if (cwchPrefix > 0) 
                    wprintf(L"Attr: %s:%s=\"%s\" \n", pwszPrefix, pwszLocalName, pwszValue); 
                else 
                    wprintf(L"Attr: %s=\"%s\" \n", pwszLocalName, pwszValue); 
            } 
 
            if (S_OK != pReader->MoveToNextAttribute()) 
                break; 
        } 
    } 
    return hr; 
} 
void XML_Test_Read(const CString csFileName = CString(_T("WimTivoServer.0.xml")))
{
	HRESULT hr = S_OK;
	CComPtr<IXmlReader> pReader; 
	if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL))) 
		std::cout << "[" << getTimeISO8601() << "] Error creating xml reader, error is: " << hex << hr << endl;
	else
	{
		if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) 
			std::cout << "[" << getTimeISO8601() << "] Error setting XmlReaderProperty_DtdProcessing, error is: " << hex << hr << endl;
		else
		{
			//Open read-only input stream 
			CComPtr<IStream> pFileStream;
			if (FAILED(hr = SHCreateStreamOnFile(csFileName.GetString(), STGM_READ, &pFileStream))) 
				std::cout << "[" << getTimeISO8601() << "] Error creating file reader, error is: " << hex << hr << endl;
			else
			{
				if (FAILED(hr = pReader->SetInput(pFileStream))) 
					std::cout << "[" << getTimeISO8601() << "] Error setting input for reader, error is: " << hex << hr << endl;
				else
				{
				 	XmlNodeType nodeType; 
					const WCHAR* pwszPrefix; 
					const WCHAR* pwszLocalName; 
					const WCHAR* pwszValue; 
					UINT cwchPrefix; 
					//read until there are no more nodes 
					while (S_OK == (hr = pReader->Read(&nodeType))) 
					{ 
						switch (nodeType) 
						{ 
						case XmlNodeType_XmlDeclaration: 
							wprintf(L"XmlDeclaration\n"); 
							if (FAILED(hr = WriteAttributes(pReader))) 
								wprintf(L"Error writing attributes, error is %08.8lx", hr); 
							break; 
						case XmlNodeType_Element: 
							if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) 
								wprintf(L"Error getting prefix, error is %08.8lx", hr); 
							if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) 
								wprintf(L"Error getting local name, error is %08.8lx", hr); 
							if (cwchPrefix > 0) 
								wprintf(L"Element: %s:%s\n", pwszPrefix, pwszLocalName); 
							else 
								wprintf(L"Element: %s\n", pwszLocalName);  
							if (FAILED(hr = WriteAttributes(pReader))) 
								wprintf(L"Error writing attributes, error is %08.8lx", hr); 
							if (pReader->IsEmptyElement() ) 
								wprintf(L" (empty)"); 
							break; 
						case XmlNodeType_EndElement: 
							if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) 
								wprintf(L"Error getting prefix, error is %08.8lx", hr); 
							if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) 
								wprintf(L"Error getting local name, error is %08.8lx", hr); 
							if (cwchPrefix > 0) 
								wprintf(L"End Element: %s:%s\n", pwszPrefix, pwszLocalName); 
							else 
								wprintf(L"End Element: %s\n", pwszLocalName); 
							break; 
						case XmlNodeType_Text: 
						case XmlNodeType_Whitespace: 
							if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
								wprintf(L"Error getting value, error is %08.8lx", hr); 
							wprintf(L"Text: >%s<\n", pwszValue); 
							break; 
						case XmlNodeType_CDATA: 
							if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
								wprintf(L"Error getting value, error is %08.8lx", hr); 
							wprintf(L"CDATA: %s\n", pwszValue); 
							break; 
						case XmlNodeType_ProcessingInstruction: 
							if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) 
								wprintf(L"Error getting name, error is %08.8lx", hr); 
							if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
								wprintf(L"Error getting value, error is %08.8lx", hr); 
							wprintf(L"Processing Instruction name:%s value:%s\n", pwszLocalName, pwszValue); 
							break; 
						case XmlNodeType_Comment: 
							if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
								wprintf(L"Error getting value, error is %08.8lx", hr); 
							wprintf(L"Comment: %s\n", pwszValue); 
							break; 
						case XmlNodeType_DocumentType: 
							wprintf(L"DOCTYPE is not printed\n"); 
							break; 
						} 
					} 
				}
			}
		}
	}
}
std::wstring Indent(const int count = 1)
{
	std::wstring rval;
	for (auto i = 0; i < count; i++)
		rval.append(L"  ");
	return(rval);
}
void XML_Test_Read_ElementsOnly(CString csFileName = CString(_T("WimTivoServer.0.xml")))
{
	HRESULT hr = S_OK;
	CComPtr<IXmlReader> pReader; 
	if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL))) 
		std::cout << "[" << getTimeISO8601() << "] Error creating xml reader, error is: " << hex << hr << endl;
	else
	{
		if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) 
			std::cout << "[" << getTimeISO8601() << "] Error setting XmlReaderProperty_DtdProcessing, error is: " << hex << showbase << setfill('0') << setw(8) << hr << endl;
		else
		{
			//Open read-only input stream 
			CComPtr<IStream> pFileStream;
			if (FAILED(hr = SHCreateStreamOnFile(csFileName.GetString(), STGM_READ, &pFileStream))) 
				std::cout << "[" << getTimeISO8601() << "] Error creating file reader, error is: " << hex << hr << endl;
			else
			{
				if (FAILED(hr = pReader->SetInput(pFileStream))) 
					std::cout << "[" << getTimeISO8601() << "] Error setting input for reader, error is: " << hex << hr << endl;
				else
				{
					int indentlevel = 0;
				 	XmlNodeType nodeType; 
					const WCHAR* pwszPrefix; 
					const WCHAR* pwszLocalName; 
					const WCHAR* pwszValue; 
					UINT cwchPrefix; 
					//read until there are no more nodes 
					while (S_OK == (hr = pReader->Read(&nodeType))) 
					{ 
						switch (nodeType) 
						{ 
						case XmlNodeType_XmlDeclaration: 
							std::cout << "[                   ] XmlDeclaration" << endl;
							//wprintf(L"XmlDeclaration\n"); 
							//if (FAILED(hr = WriteAttributes(pReader))) 
							//	wprintf(L"Error writing attributes, error is %08.8lx", hr); 
							break; 
						case XmlNodeType_Element: 
							if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) 
								std::cout << "[                   ] Error getting prefix, error is " << hex << hr << endl;
							if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) 
								std::cout << "[                   ] Error getting local name, error is" << hex << hr << endl;
							if (cwchPrefix > 0) 
								std::wcout << L"[                   ] " << Indent(indentlevel) << L"Element: " << pwszPrefix << L":" << pwszLocalName << endl;
							else 
								std::wcout << L"[                   ] " << Indent(indentlevel) << L"Element: " << pwszLocalName << endl;
							//if (FAILED(hr = WriteAttributes(pReader))) 
							//	wprintf(L"Error writing attributes, error is %08.8lx", hr); 
							if (pReader->IsEmptyElement() ) 
								std::cout << "[                   ] (empty)" << endl;
							indentlevel++;
							break; 
						case XmlNodeType_EndElement: 
							if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) 
								std::cout << "[                   ] Error getting prefix, error is " << hex << hr << endl;
							if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) 
								std::cout << "[                   ] Error getting local name, error is" << hex << hr << endl;
							if (cwchPrefix > 0) 
								std::wcout << L"[                   ] " << Indent(indentlevel) << L"End Element: " << pwszPrefix << L":" << pwszLocalName << endl;
							else 
								std::wcout << L"[                   ] " << Indent(indentlevel) << L"End Element: " << pwszLocalName << endl;
							indentlevel--;
							break; 
						case XmlNodeType_Text: 
						case XmlNodeType_Whitespace: 
							if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
								std::cout << "[                   ] Error getting value, error is " << hex << hr << endl;
							std::wcout << L"[                   ] " << Indent(indentlevel) << L"Text: >" << pwszValue << L"<" << endl;
							break; 
						case XmlNodeType_CDATA: 
							//if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
							//	wprintf(L"Error getting value, error is %08.8lx", hr); 
							//wprintf(L"CDATA: %s\n", pwszValue); 
							break; 
						case XmlNodeType_ProcessingInstruction: 
							//if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) 
							//	wprintf(L"Error getting name, error is %08.8lx", hr); 
							//if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
							//	wprintf(L"Error getting value, error is %08.8lx", hr); 
							//wprintf(L"Processing Instruction name:%s value:%s\n", pwszLocalName, pwszValue); 
							break; 
						case XmlNodeType_Comment: 
							if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) 
								std::cout << "[                   ] Error getting value, error is " << hex << hr << endl;
							std::wcout << L"[                   ] " << Indent(indentlevel) << L"Comment: " << pwszValue << endl;
							break; 
						case XmlNodeType_DocumentType: 
							std::cout << "[                   ] DOCTYPE is not printed" << endl;
							break; 
						} 
					} 
				}
			}
		}
	}
}
void XML_Test_Write_InMemory(void)
{
	HRESULT hr = S_OK;
	CComPtr<IXmlWriter> pWriter;
	if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
		std::cout << "[" << getTimeISO8601() << "] Error creating xml writer, error is: " << hex << hr << endl;
	else
	{
		// from: http://stackoverflow.com/questions/3037946/how-can-i-store-xml-in-buffer-using-xmlite
		CComPtr<IStream> spMemoryStream(::SHCreateMemStream(NULL, 0));
		//// Opens writeable output stream.
		//spMemoryStream.Attach(::SHCreateMemStream(NULL, 0));
		//std::unique_ptr<IStream> spMemoryStream(::SHCreateMemStream(NULL, 0));
		if (spMemoryStream != NULL)
		{
			CComPtr<IXmlWriterOutput> pWriterOutput;
			CreateXmlWriterOutputWithEncodingName(spMemoryStream, NULL, L"utf-16", &pWriterOutput);
			pWriter->SetOutput(pWriterOutput);
			//pWriter->SetOutput(spMemoryStream);
			pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
			pWriter->WriteStartDocument(XmlStandalone_Omit);
				pWriter->WriteStartElement(NULL,_T("TiVoContainer"),_T("http://www.tivo.com/developer/calypso-protocol-1.6/"));
					pWriter->WriteStartElement(NULL,_T("Details"),NULL);
						pWriter->WriteStartElement(NULL,_T("ContentType"),NULL);
							pWriter->WriteString(_T("x-tivo-container/tivo-server"));
						pWriter->WriteFullEndElement();
						pWriter->WriteStartElement(NULL,_T("SourceFormat"),NULL);
							pWriter->WriteString(_T("x-tivo-container/tivo-dvr"));
						pWriter->WriteFullEndElement();
						pWriter->WriteStartElement(NULL,_T("Title"),NULL);
							pWriter->WriteString(_T("TivoHD"));
						pWriter->WriteFullEndElement();
						pWriter->WriteStartElement(NULL,_T("TotalItems"),NULL);
							pWriter->WriteString(_T("1"));
						pWriter->WriteFullEndElement();
					pWriter->WriteFullEndElement();	// Details

					pWriter->WriteStartElement(NULL,_T("ItemStart"),NULL);
						pWriter->WriteString(_T("0"));
					pWriter->WriteFullEndElement();
					pWriter->WriteStartElement(NULL,_T("ItemCount"),NULL);
						pWriter->WriteString(_T("1"));
					pWriter->WriteFullEndElement();

					pWriter->WriteStartElement(NULL,_T("Item"),NULL);
						pWriter->WriteStartElement(NULL,_T("Details"),NULL);
							pWriter->WriteStartElement(NULL,_T("ContentType"),NULL);
								pWriter->WriteString(_T("x-tivo-container/tivo-videos"));
							pWriter->WriteFullEndElement();
							pWriter->WriteStartElement(NULL,_T("SourceFormat"),NULL);
								pWriter->WriteString(_T("x-tivo-container/tivo-dvr"));
							pWriter->WriteFullEndElement();
							pWriter->WriteStartElement(NULL,_T("Title"),NULL);
								pWriter->WriteString(_T("TivoHD"));
							pWriter->WriteFullEndElement();
							pWriter->WriteStartElement(NULL,_T("UniqueId"),NULL);
								pWriter->WriteString(_T("TivoHD"));
							pWriter->WriteFullEndElement();
						pWriter->WriteFullEndElement();	// Details
						pWriter->WriteStartElement(NULL,_T("Links"),NULL);
							pWriter->WriteStartElement(NULL,_T("Content"),NULL);
								pWriter->WriteStartElement(NULL,_T("Url"),NULL);
									pWriter->WriteString(_T("https://192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=%2FNowPlaying"));
								pWriter->WriteFullEndElement();
								pWriter->WriteStartElement(NULL,_T("ContentType"),NULL);
									pWriter->WriteString(_T("x-tivo-container/tivo-videos"));
								pWriter->WriteFullEndElement();
							pWriter->WriteFullEndElement();	// Content
						pWriter->WriteFullEndElement();	// Links
					pWriter->WriteFullEndElement();	// Item
				pWriter->WriteFullEndElement();	// TiVoContainer
			pWriter->WriteEndDocument();
			pWriter->Flush();

			// Allocates enough memeory for the xml content.
			STATSTG ssStreamData = {0};
			spMemoryStream->Stat(&ssStreamData, STATFLAG_NONAME);
			SIZE_T cbSize = ssStreamData.cbSize.LowPart;
			LPWSTR pwszContent = (WCHAR*) new BYTE[cbSize + sizeof(WCHAR)];
			if (pwszContent != NULL)
			{
				// Copies the content from the stream to the buffer.
				LARGE_INTEGER position;
				position.QuadPart = 0;
				spMemoryStream->Seek(position, STREAM_SEEK_SET, NULL);
				ULONG cbRead;
				spMemoryStream->Read(pwszContent, cbSize, &cbRead);
				pwszContent[cbSize / sizeof(WCHAR)] = L'\0';

				wprintf(L"%s", pwszContent);
				delete[] pwszContent;
			}
			// Next I'm going to follow http://stackoverflow.com/questions/8804687/unable-to-read-xml-string-with-xmllite-using-memory-buffer
			// and figure out how to read the XML out of an internal memory buffer.
		}
	}
}
void XML_Test_Write(const CString csFileName = CString(_T("WimTivoServer.8.xml")))
{
	HRESULT hr = S_OK;
	CComPtr<IXmlWriter> pWriter;
	if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
		std::cout << "[" << getTimeISO8601() << "] Error creating xml writer, error is: " << hex << hr << endl;
	else
	{
		//Open writeable output stream 
		CComPtr<IStream> pOutFileStream;
		if (FAILED(hr = SHCreateStreamOnFile(csFileName.GetString(), STGM_CREATE | STGM_WRITE, &pOutFileStream))) 
			std::cout << "[" << getTimeISO8601() << "] Error creating file writer, error is: " << hex << hr << endl;
		else
		{ 
			if (FAILED(hr = pWriter->SetOutput(pOutFileStream))) 
				wprintf(L"Error, Method: SetOutput, error is %08.8lx", hr); 
			else
			{
				if (FAILED(hr = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE))) 
					wprintf(L"Error, Method: SetProperty XmlWriterProperty_Indent, error is %08.8lx", hr); 
				else
				{
					if (FAILED(hr = pWriter->WriteStartDocument(XmlStandalone_Omit))) 
						wprintf(L"Error, Method: WriteStartDocument, error is %08.8lx", hr); 
					else
					{
						pWriter->WriteStartElement(NULL,_T("TiVoContainer"),_T("http://www.tivo.com/developer/calypso-protocol-1.6/"));
							pWriter->WriteStartElement(NULL,_T("Details"),NULL);
								pWriter->WriteStartElement(NULL,_T("ContentType"),NULL);
									pWriter->WriteString(_T("x-tivo-container/tivo-server"));
								pWriter->WriteFullEndElement();
								pWriter->WriteStartElement(NULL,_T("SourceFormat"),NULL);
									pWriter->WriteString(_T("x-tivo-container/tivo-dvr"));
								pWriter->WriteFullEndElement();
								pWriter->WriteStartElement(NULL,_T("Title"),NULL);
									pWriter->WriteString(_T("TivoHD"));
								pWriter->WriteFullEndElement();
								pWriter->WriteStartElement(NULL,_T("TotalItems"),NULL);
									pWriter->WriteString(_T("1"));
								pWriter->WriteFullEndElement();
							pWriter->WriteFullEndElement();	// Details
							pWriter->WriteStartElement(NULL,_T("ItemStart"),NULL);
								pWriter->WriteString(_T("0"));
							pWriter->WriteFullEndElement();
							pWriter->WriteStartElement(NULL,_T("ItemCount"),NULL);
								pWriter->WriteString(_T("1"));
							pWriter->WriteFullEndElement();
							pWriter->WriteStartElement(NULL,_T("Item"),NULL);
								pWriter->WriteStartElement(NULL,_T("Details"),NULL);
									pWriter->WriteStartElement(NULL,_T("ContentType"),NULL);
										pWriter->WriteString(_T("x-tivo-container/tivo-videos"));
									pWriter->WriteFullEndElement();
									pWriter->WriteStartElement(NULL,_T("SourceFormat"),NULL);
										pWriter->WriteString(_T("x-tivo-container/tivo-dvr"));
									pWriter->WriteFullEndElement();
									pWriter->WriteStartElement(NULL,_T("Title"),NULL);
										pWriter->WriteString(_T("TivoHD"));
									pWriter->WriteFullEndElement();
									pWriter->WriteStartElement(NULL,_T("UniqueId"),NULL);
										pWriter->WriteString(_T("TivoHD"));
									pWriter->WriteFullEndElement();
								pWriter->WriteFullEndElement();	// Details
								pWriter->WriteStartElement(NULL,_T("Links"),NULL);
									pWriter->WriteStartElement(NULL,_T("Content"),NULL);
										pWriter->WriteStartElement(NULL,_T("Url"),NULL);
											pWriter->WriteString(_T("https://192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=%2FNowPlaying"));
										pWriter->WriteFullEndElement();
										pWriter->WriteStartElement(NULL,_T("ContentType"),NULL);
											pWriter->WriteString(_T("x-tivo-container/tivo-videos"));
										pWriter->WriteFullEndElement();
									pWriter->WriteFullEndElement();	// Content
								pWriter->WriteFullEndElement();	// Links
							pWriter->WriteFullEndElement();	// Item
						pWriter->WriteFullEndElement();	// TiVoContainer
						// WriteEndDocument closes any open elements or attributes 
						if (FAILED(hr = pWriter->WriteEndDocument())) 
							wprintf(L"Error, Method: WriteEndDocument, error is %08.8lx", hr); 
						if (FAILED(hr = pWriter->Flush())) 
							wprintf(L"Error, Method: Flush, error is %08.8lx", hr); 
					}
				}
			}
		} 
	} 
}
void XML_Test_FileReformat(const CString & Source = _T("D:\\Videos\\chunk-01-0001.xml"), const CString Destination = _T("D:/Videos/Evening-1.xml"))
{
	// crap to clean up a file...  
	HRESULT hr = S_OK;
	CComPtr<IXmlReader> pReader; 
	if (SUCCEEDED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL))) 
	{
		if (SUCCEEDED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) 
		{
			CComPtr<IStream> spFileStreamOne;
			if (SUCCEEDED(hr = SHCreateStreamOnFile(Source.GetString(), STGM_READ, &spFileStreamOne)))
			{
				if (SUCCEEDED(hr = pReader->SetInput(spFileStreamOne))) 
				{
					CComPtr<IXmlWriter> pWriter;
					if (SUCCEEDED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
					{
						CComPtr<IStream> spFileStreamTwo;
						if (SUCCEEDED(hr = SHCreateStreamOnFile(Destination.GetString(), STGM_CREATE | STGM_WRITE, &spFileStreamTwo)))
						{
							if (SUCCEEDED(hr = pWriter->SetOutput(spFileStreamTwo)))
							{
								pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
								while (S_OK == (hr = pWriter->WriteNode(pReader, TRUE)));	// loops over entire xml file, writing it out with indenting
								pWriter->Flush();
							}
						}
					}
				}
			}
		}
	}
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
void PopulateTiVoFileList(std::vector<cTiVoFile> & TiVoFileList, std::string FileSpec)
{
	#ifdef _DEBUG
	TRACE(__FUNCTION__ "\n");
	std::cout << "[" << getTimeISO8601() << "] " << __FUNCTION__ << endl;
	#endif
	CFileFind finder;
	
	BOOL bWorking = finder.FindFile(CString(CStringA(FileSpec.c_str())));
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots())
			continue;
		if (finder.IsDirectory())
			continue;
		if (finder.IsHidden())
			continue;
		if (finder.IsSystem())
			continue;
		if (finder.IsTemporary())
			continue;
		cTiVoFile myFile;
		if (finder.GetLength() > 0)
		{
			myFile.SetPathName(finder);
			if (!myFile.GetSourceFormat().Left(6).CompareNoCase(_T("video/")))
				TiVoFileList.push_back(myFile);
		}
	}
	finder.Close();
	#ifdef _DEBUG
	std::cout << "[" << getTimeISO8601() << "] "  << __FUNCTION__ << "\texiting" << endl;
	#endif
}
/////////////////////////////////////////////////////////////////////////////
void printerr(TCHAR * errormsg)
{
	if (bConsoleExists)
	{
		_ftprintf(stderr, _T("%s\n"), errormsg);
	}
	else
	{
		if (ApplicationLogHandle != NULL) 
		{
			LPCTSTR lpStrings[] = { errormsg, NULL };
			ReportEvent(ApplicationLogHandle, EVENTLOG_INFORMATION_TYPE, 0, WIMSWORLD_EVENT_GENERIC, NULL, 1, 0, lpStrings, NULL);
		}
	}
}
int GetTiVoQueryFormats(SOCKET DataSocket, const char * InBuffer)
{
	TRACE(__FUNCTION__ "\n");
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
std::vector<cTiVoFile> TiVoFileList;
std::queue<cTiVoFile> TiVoFilesToConvert;
int GetTivoQueryContainer(SOCKET DataSocket, const char * InBuffer)
{
	TRACE(__FUNCTION__ "\n");
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
		char MyHostName[80] = {0}; // winsock hostname used for data recordkeeping
		gethostname(MyHostName,sizeof(MyHostName));
		CString csContainer;
		CString csAnchorItem;
		if (!TiVoFileList.empty())
			csAnchorItem = TiVoFileList.begin()->GetURL();
		int iAnchorOffset = 0;
		int iItemCount = TiVoFileList.size();
		CString csCommand(InBuffer);
		int curPos = 0;
		CString csToken(csCommand.Tokenize(_T("& ?"),curPos));
		while (csToken != _T(""))
		{
			CString csKey(csToken.Left(csToken.Find(_T("="))));
			CString csValue(csToken.Right(csToken.GetLength() - (csToken.Find(_T("="))+1)));
			if (!csKey.CompareNoCase(_T("Container")))
			{
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
				csContainer = csValue;
			}
			else if (!csKey.CompareNoCase(_T("Recurse")))
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
			else if (!csKey.CompareNoCase(_T("SortOrder")))
			{
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
				//std::sort(TiVoFileList.begin(),TiVoFileList.end(),cTiVoFileCompareDate);
			}
			else if (!csKey.CompareNoCase(_T("RandomSeed")))
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
			else if (!csKey.CompareNoCase(_T("RandomStart")))
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
			else if (!csKey.CompareNoCase(_T("AnchorOffset")))
			{
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
				iAnchorOffset = atoi(CStringA(csValue).GetString());
				iAnchorOffset++; // Simplify for the -1 offset that the TiVo actually uses.
			}
			else if (!csKey.CompareNoCase(_T("Filter")))
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
			else if (!csKey.CompareNoCase(_T("ItemCount")))
			{
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
				iItemCount = atoi(CStringA(csValue).GetString());
			}
			else if (!csKey.CompareNoCase(_T("AnchorItem")))
			{
				std::wcout << L"[                   ] " << csToken.GetString() << endl;
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
			csToken = csCommand.Tokenize(_T("& ?"),curPos);
		}
		iItemCount = min(iItemCount, TiVoFileList.size());
		CString csTemporary;
		pWriter->SetOutput(spMemoryStream);
		pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
		pWriter->WriteStartDocument(XmlStandalone_Omit);
			pWriter->WriteStartElement(NULL,L"TiVoContainer",L"http://www.tivo.com/developer/calypso-protocol-1.6/");
			if (csContainer.Compare(_T("%2F")) == 0)
			{
				pWriter->WriteStartElement(NULL,_T("Details"),NULL);
					csTemporary = CStringA(MyHostName);
					csTemporary.Append(_T(" (WimTiVoServer)"));
					pWriter->WriteElementString(NULL,_T("Title"),NULL, csTemporary.GetString());
					pWriter->WriteElementString(NULL,_T("ContentType"),NULL, _T("x-tivo-container/tivo-server"));
					pWriter->WriteElementString(NULL,_T("SourceFormat"),NULL, _T("x-tivo-container/folder"));
					pWriter->WriteElementString(NULL,_T("TotalItems"),NULL, _T("1"));
				pWriter->WriteEndElement();	// Details
				pWriter->WriteStartElement(NULL,_T("Item"),NULL);
					pWriter->WriteStartElement(NULL,_T("Details"),NULL);
						csTemporary = CStringA(MyHostName);
						csTemporary.Append(_T(" (WimTiVoServer)"));
						pWriter->WriteElementString(NULL,_T("Title"),NULL, csTemporary.GetString());
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
				csTemporary.Format(_T("%d"), pItem - TiVoFileList.begin());
				pWriter->WriteElementString(NULL,L"ItemStart",NULL, csTemporary.GetString());
				csTemporary.Format(_T("%d"), iItemCount);
				pWriter->WriteElementString(NULL,L"ItemCount",NULL, csTemporary.GetString());
				pWriter->WriteStartElement(NULL,L"Details",NULL);
					csTemporary = CStringA(MyHostName);
					csTemporary.Append(_T(" (WimTiVoServer)"));
					pWriter->WriteElementString(NULL,L"Title",NULL, csTemporary.GetString());
					pWriter->WriteElementString(NULL,L"ContentType",NULL, L"x-tivo-container/folder");
					pWriter->WriteElementString(NULL,L"SourceFormat",NULL, L"x-tivo-container/folder");
					csTemporary.Format(_T("%d"), TiVoFileList.size());
					pWriter->WriteElementString(NULL,L"TotalItems",NULL, csTemporary.GetString());
				pWriter->WriteEndElement();
				while ((pItem != TiVoFileList.end()) && (iItemCount > 0))
				{
					std::wcout << L"[                   ] Item: " << pItem->GetPathName().GetString() << std::endl;
					pItem->GetXML(pWriter);
					pItem++;
					iItemCount--;
				}
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
	TRACE(__FUNCTION__ "\n");
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
				for (auto MyFile = TiVoFileList.begin(); MyFile != TiVoFileList.end(); MyFile++)
					if (!MyFile->GetURL().CompareNoCase(csUrl))
					{
						MyFile->GetTvBusEnvelope(pWriter);
						break;
					}
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
	TRACE(__FUNCTION__ "\n");
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
			for (auto MyFile = TiVoFileList.begin(); MyFile != TiVoFileList.end(); MyFile++)
				if (!MyFile->GetURL().CompareNoCase(csToken))
				{
					std::wcout << L"[                   ] Found File: " << MyFile->GetTitle().GetString() << std::endl;
					TiVoFileToSend = *MyFile;
					//MyFile->GetTvBusEnvelope(pWriter);
					break;
				}
			if (TiVoFileToSend.GetSourceSize() == 0)
				std::wcout << L"[                   ] Not Found File: " << csToken.GetString() << std::endl;
			//csToken.Delete(0,csUrlPrefix.GetLength());
			//TCHAR lpszBuffer[_MAX_PATH];
			//DWORD dwBufferLength = sizeof(lpszBuffer) / sizeof(TCHAR);
			//InternetCanonicalizeUrl(csToken.GetString(), lpszBuffer, &dwBufferLength, ICU_DECODE | ICU_NO_ENCODE);
			//csFileName = CString(lpszBuffer, dwBufferLength);
			//if (!csFileName.Left(7).CompareNoCase(_T("file://"))) 
			//	csFileName.Delete(0,7);
			//while (0 < csFileName.Replace(_T("%20"),_T(" "))); // take care of spaces that are still encoded
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
		HttpResponse << "HTTP/1.0 206 Partial Content\r\n";
		HttpResponse << "Server: Wims TiVo Server/1.0.0.1\r\n";
		HttpResponse << "Date: " << getTimeRFC1123() << "\r\n";
		HttpResponse << "Transfer-Encoding: chunked\r\n";
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
				while (!FileToTransfer.eof() && (bSoFarSoGood))
				{
					FileToTransfer.read(RAWDataBuff, 0x400000);
					int BytesToSendInBuffer = FileToTransfer.gcount();
					int offset = 0;
					nRet = send(DataSocket, RAWDataBuff+offset, BytesToSendInBuffer-offset, 0);
					bytessent += nRet;
					bSoFarSoGood = nRet == BytesToSendInBuffer;
				}
				delete[] RAWDataBuff;
				std::cout << "[                   ] Finished Sending File, bSoFarSoGood=" << boolalpha << bSoFarSoGood << " BytesSent(" << bytessent << ")" << endl;
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
			if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0x80000) ) 
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
					//siStartInfo.hStdError = g_hChildStd_OUT_Wr;
					//siStartInfo.hStdInput = g_hChildStd_IN_Rd;
					siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
					siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
					siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
					siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
 
					// "D:\\pytivo\\bin\\ffmpeg.exe" -i "D:\\Videos\\archer\\Archer.2009.S03E12.HDTV.x264.mp4" -vcodec mpeg2video -b 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec ac3 -copyts -map 0:0 -map 0:1 -report -f vob -
					std::wstringstream ss;
					//ss << L"ffmpeg.exe -i " << QuoteFileName(csFileName).GetString() << L" -vcodec mpeg2video -acodec ac3 -copyts -report -f vob -";
					// -vcodec copy -b 10871k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec copy -map 0:1 -map 0:0 -report -f vob -
					//ss << L"ffmpeg.exe -i " << QuoteFileName(TiVoFileToSend.GetPathName()).GetString() << L" -vcodec mpeg2video -b:v 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec ac3 -copyts -report -f vob -";
					if (!TiVoFileToSend.GetSourceFormat().Compare(_T("video/mpeg2video")))
						ss << L"ffmpeg.exe -i " << QuoteFileName(TiVoFileToSend.GetPathName()).GetString() << L" -vcodec copy -b:v 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec copy -report -f vob -";
					else
						ss << L"ffmpeg.exe -i " << QuoteFileName(TiVoFileToSend.GetPathName()).GetString() << L" -vcodec mpeg2video -b:v 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec ac3 -copyts -report -f vob -";
						//ss << L"ffmpeg.exe -i " << QuoteFileName(TiVoFileToSend.GetPathName()).GetString() << L" -vcodec mpeg2video -b:v 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000 -acodec ac3 -copyts -report -f vob -";
					TCHAR szCmdline[1024];
					szCmdline[ss.str().copy(szCmdline, (sizeof(szCmdline)/sizeof(TCHAR))-sizeof(TCHAR))] = _T('\0');
					std::wcout << L"[                   ] CreateProcess: " << szCmdline << std::endl;

					// Create the child process.     
					//TCHAR szCmdline[]=TEXT("child");
					BOOL bSuccess = CreateProcess(NULL, 
						szCmdline,     // command line 
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
						long long bytessent = 0;
						char * RAWDataBuff = new char[0x80000];
						DWORD dwRead, dwWritten; 
						BOOL bSuccess = FALSE;
						CTime ctStart(CTime::GetCurrentTime());
						CTimeSpan ctsTotal = CTime::GetCurrentTime() - ctStart;
						unsigned long long CurrentFileSize = 0;
						for (;;) 
						{ 
							bSuccess = ReadFile(g_hChildStd_OUT_Rd, RAWDataBuff, 0x80000, &dwRead, NULL);
							if( (!bSuccess) || (dwRead == 0)) break; 
							if (DataSocket != INVALID_SOCKET) 
							{
								ssChunkHeader.str("");
								ssChunkHeader << hex << "\r\n" << dwRead << "\r\n";
								send(DataSocket, ssChunkHeader.str().c_str(), ssChunkHeader.str().length(), 0);
								int nRet = send(DataSocket, RAWDataBuff, dwRead, 0);
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
												// WSAECONNRESET is 10054L
												//[                   ] Error code: 10054 An existing connection was forcibly closed by the remote host.
									LocalFree(errString);	// if you don't do this, you will get an
															// ever so slight memory leak, since we asked
															// FormatMessage to FORMAT_MESSAGE_ALLOCATE_BUFFER,
															// and it does so using LocalAlloc
															// Gotcha!  I guess.
									WSASetLastError(0);		// Reset this so that subsequent calls may be accurate
									break;
								}
								bytessent += nRet;
								if (nRet != dwRead)
								{
									std::cout << "[                   ] Not all Read Data was Sent. Read: " << dwRead << " Send: " << nRet << std::endl;
									char * ptrData = RAWDataBuff + nRet;
									int DataToSend = dwRead - nRet;
									while ((DataSocket != INVALID_SOCKET) && (DataToSend > 0) && (SOCKET_ERROR != nRet))
									{
										nRet = send(DataSocket, ptrData, DataToSend, 0);
										ptrData += nRet;
										DataToSend -= nRet;
									}
								}
								CurrentFileSize += nRet;
								ctsTotal = CTime::GetCurrentTime() - ctStart;
								// This is another experiment trying to find out why I'm failing to send files to the TiVo
								// I was initially going to use select() on the socket, but later decided that the ioctlsocket() call mould be simpler.
								// http://developerweb.net/viewtopic.php?id=2933
								u_long iMode = 0;
								if (SOCKET_ERROR != ioctlsocket(DataSocket, FIONREAD, &iMode))
									if (iMode > 0)
									{
										char *JunkBuffer = new char[iMode+1];
										recv(DataSocket, JunkBuffer, iMode, 0);
										JunkBuffer[iMode] = '\0';
										std::cout << "\n\r[                   ] Unexpected Stuff came from TiVo: " << JunkBuffer << std::endl;
										delete[] JunkBuffer;
									}
							}
						} 
						ssChunkHeader.str("");
						ssChunkHeader << hex << "\r\n" << 0 << "\r\n\r\n";	// \r\n ends previous chunk, 0\r\n is last chunk ending, and \r\n is the trailer.
						send(DataSocket, ssChunkHeader.str().c_str(), ssChunkHeader.str().length(), 0);
						delete[] RAWDataBuff;
						// Close handles to the child process and its primary thread.
						// Some applications might keep these handles to monitor the status
						// of the child process, for example. 
						CloseHandle(piProcInfo.hProcess);
						CloseHandle(piProcInfo.hThread);
						auto TotalSeconds = ctsTotal.GetTotalSeconds();
						if (TotalSeconds > 0)
							std::cout << "[" << getTimeISO8601() << "] Finished Sending File, BytesSent(" << bytessent << ")" << " Speed: " << (CurrentFileSize / TotalSeconds) << " B/s, " << CStringA(ctsTotal.Format(_T("%H:%M:%S"))).GetString() << std::endl;
						else
							std::cout << "[" << getTimeISO8601() << "] Finished Sending File, BytesSent(" << bytessent << ")" << std::endl;
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
	closesocket(remoteSocket);
	return(0);
}
UINT HTTPMain(LPVOID lvp)
{
	if (AfxSocketInit())
	{
		/* Open a listening socket */
		ControlSocket = socket(AF_INET,	/* Address family */
							SOCK_STREAM,	/* Socket type */
							IPPROTO_TCP);	/* Protocol */
		if (ControlSocket == INVALID_SOCKET)
			printerr(_T("Fatal Error: Socket could not be created"));
		else
		{
			int on = 1;
			SOCKADDR_IN saServer;
			int nRet;
			setsockopt(ControlSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
			saServer.sin_family = AF_INET;
			saServer.sin_addr.s_addr = INADDR_ANY;	/* Let WinSock supply address */
			#ifdef _DEBUG
			saServer.sin_port = htons(64321);
			#else
			saServer.sin_port = htons(0);			/* Use unique port */
			#endif
			nRet = bind(ControlSocket,				/* Socket */
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
						{
							setsockopt(remoteSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));	// Attempt to see if this is related to why many of my transfers are failing.
							#ifdef _WIM_THREADED_
							AfxBeginThread(HTTPChild, (LPVOID)remoteSocket);
							#else
							HTTPChild((LPVOID)remoteSocket);
							#endif
						}
					}
				}
			}
		}
	}
	SetEvent(terminateEvent);
	return(0);
}
UINT TiVoConvertFileThread(LPVOID lvp)
{
	TRACE(__FUNCTION__ "\n");
	CString m_csTDCatPath(FindEXEFromPath(_T("tdcat.exe")));
	CString m_csTiVoDecodePath(FindEXEFromPath(_T("tivodecode.exe")));
	CString m_csFFMPEGPath(FindEXEFromPath(_T("ffmpeg.exe")));
	while (!TiVoFilesToConvert.empty())
	{
		cTiVoFile TiVoFile(TiVoFilesToConvert.front());
		TiVoFilesToConvert.pop();
		CString csTiVoFileName(TiVoFile.GetPathName());
		CFileStatus status;
		if (TRUE == CFile::GetStatus(csTiVoFileName, status)) // Test to make sure the .TiVo file exists!
		{
			CString csMPEGPathName(ReplaceExtension(csTiVoFileName, _T(".mpeg")));
			CString csXMLPathName(ReplaceExtension(csTiVoFileName, _T(".xml")));
			CString csMP4PathName(ReplaceExtension(csTiVoFileName, _T(".mp4")));
			if (TRUE != CFile::GetStatus(csMPEGPathName, status))
			{
				if (!m_csTDCatPath.IsEmpty())
				{
					std::cout << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(m_csTDCatPath).GetString() << " " << CStringA(m_csTDCatPath).GetString() << "  --mak  " << CStringA(TiVoFile.GetMAK()).GetString() << " --out " << CStringA(QuoteFileName(csXMLPathName)).GetString() << " " << CStringA(csXMLPathName).GetString() << " --chunk-2 " << CStringA(QuoteFileName(csTiVoFileName)).GetString() << std::endl;
					if (-1 == _tspawnl(_P_WAIT, m_csTDCatPath.GetString(), m_csTDCatPath.GetString(), _T("--mak"), TiVoFile.GetMAK(), _T("--out"), QuoteFileName(csXMLPathName).GetString(), _T("--chunk-2"), QuoteFileName(csTiVoFileName).GetString(), NULL))
						std::cout << "[                   ]  _tspawnlp failed: " /* << strerror(errno) */ << std::endl;
				}
				if (TRUE == CFile::GetStatus(csXMLPathName, status))
				{
					status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
					CFile::SetStatus(csXMLPathName, status);
				}

				std::cout << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(m_csTiVoDecodePath).GetString() << " " << CStringA(m_csTiVoDecodePath).GetString() << " --mak " << CStringA(TiVoFile.GetMAK()).GetString() << " --out " << CStringA(QuoteFileName(csMPEGPathName)).GetString() << " " << CStringA(QuoteFileName(csTiVoFileName)).GetString() << std::endl;
				if (-1 == _tspawnl(_P_WAIT, m_csTiVoDecodePath.GetString(), m_csTiVoDecodePath.GetString(), _T("--mak"), TiVoFile.GetMAK(), _T("--out"), QuoteFileName(csMPEGPathName).GetString(), QuoteFileName(csTiVoFileName).GetString(), NULL))
					std::cout << "[                   ]  _tspawnlp failed: " /* << strerror(errno) */ << std::endl;
				if (TRUE == CFile::GetStatus(csMPEGPathName, status))
				{
					status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
					CFile::SetStatus(csMPEGPathName, status);
					DeleteFile(csTiVoFileName);
					if (TRUE != CFile::GetStatus(csMP4PathName, status))
					{
						CString csTitle(TiVoFile.GetTitle()); while(0 < csTitle.Replace(_T("\""), _T("'"))); csTitle.Insert(0,_T("title=\""));csTitle.Append(_T("\""));
						CString csShow(TiVoFile.GetTitle()); while(0 < csShow.Replace(_T("\""), _T("'"))); csShow.Insert(0,_T("show=\""));csShow.Append(_T("\""));
						CString csDescription(TiVoFile.GetDescription()); while(0 < csDescription.Replace(_T("\""), _T("'"))); csDescription.Insert(0,_T("description=\""));csDescription.Append(_T("\""));
						CString csEpisodeID(TiVoFile.GetEpisodeTitle()); while(0 < csEpisodeID.Replace(_T("\""), _T("'"))); csEpisodeID.Insert(0,_T("episode_id=\""));csEpisodeID.Append(_T("\""));
						std::cout << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(m_csFFMPEGPath).GetString() << " " << CStringA(m_csFFMPEGPath).GetString() << " -i " << CStringA(QuoteFileName(csMPEGPathName)).GetString() << " -metadata " << CStringA(csTitle).GetString() << " -metadata " << CStringA(csShow).GetString() << " -metadata " << CStringA(csDescription).GetString() << " -metadata " << CStringA(csEpisodeID).GetString() << " -vcodec copy -acodec copy -y " << CStringA(QuoteFileName(csMP4PathName)).GetString() << std::endl;
						if (-1 == _tspawnlp(_P_WAIT, m_csFFMPEGPath.GetString(), m_csFFMPEGPath.GetString(), _T("-i"), QuoteFileName(csMPEGPathName).GetString(), 
							_T("-metadata"), csTitle.GetString(),
							_T("-metadata"), csShow.GetString(),
							_T("-metadata"), csDescription.GetString(),
							_T("-metadata"), csEpisodeID.GetString(),
							_T("-vcodec"), _T("copy"),
							_T("-acodec"), _T("copy"),
							_T("-y"), // Cause it to overwrite exiting output files
							QuoteFileName(csMP4PathName).GetString(), NULL))
							std::cout << "[                   ]  _tspawnlp failed: " /* << _sys_errlist[errno] */ << std::endl;
					}
					if (TRUE == CFile::GetStatus(csMP4PathName, status))
					{
						status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
						CFile::SetStatus(csMP4PathName, status);
						DeleteFile(csMPEGPathName);
					}
				}
			}
		}
	}
	TRACE(__FUNCTION__ " Exiting\n");
	return(0);
}
bool TiVoBeaconSend(const CStringA & csServerBroadcast)
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
		int nRet = setsockopt(theSocket,
			SOL_SOCKET,
			SO_BROADCAST,
			(const char *)&bBroadcastSocket,
			sizeof(bBroadcastSocket));
		if (nRet == SOCKET_ERROR) 
		{
			TRACE("%s: %d\n","socket()",WSAGetLastError());
		}
		else
		{
			SOCKADDR_IN saBroadCast;
			saBroadCast.sin_family = AF_INET;
			saBroadCast.sin_addr.S_un.S_addr = INADDR_BROADCAST;
			saBroadCast.sin_port = htons(2190);	// Port number
			nRet = sendto(theSocket,			// Socket
				csServerBroadcast.GetString(),	// Data buffer
				csServerBroadcast.GetLength(),	// Length of data
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
UINT TiVoBeacon(LPVOID lvp)
{
	if (!AfxSocketInit())
	{
		printerr(_T("Fatal Error: Sockets initialization failed\n"));
	}
	else
	{
		/* Open a listening socket */
		ControlSocket = socket(AF_INET,	/* Address family */
							SOCK_STREAM,	/* Socket type */
							IPPROTO_TCP);	/* Protocol */
		if (ControlSocket == INVALID_SOCKET)
			printerr(_T("Fatal Error: Socket could not be created"));
		else
		{
			int on = 1;
			SOCKADDR_IN saServer;
			int nRet;
			setsockopt(ControlSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
			saServer.sin_family = AF_INET;
			saServer.sin_addr.s_addr = INADDR_ANY;	/* Let WinSock supply address */
			saServer.sin_port = htons(8080);			/* Use port from command line */
			nRet = bind(ControlSocket,		/* Socket */
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
				/* Set the socket to listen */
				nRet = listen(ControlSocket,	/* Bound socket */
							SOMAXCONN);			/* Number of connection request queue */
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
						SOCKET remoteSocket;
						remoteSocket = accept(ControlSocket,NULL,NULL);
						//PostGroomedDataStopRequested = true;
						if (remoteSocket != INVALID_SOCKET)
						{
							char InBuff[1024];
							int count = recv(remoteSocket,InBuff,sizeof(InBuff),0);
							InBuff[count] = '\0';
							int nRet = send(remoteSocket,"HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n",41,0);
							if(bConsoleExists)
								printf("%s",InBuff);
							closesocket(remoteSocket);
						}
					}
				}
			}
		}
	}
	SetEvent(terminateEvent);
	return(0);
}
bool TiVoBeaconListen(SOCKADDR_IN &saServer)
{
	bool rval = false;
	// Create a UDP/IP datagram socket
	SOCKET theSocket = socket(AF_INET,		// Address family
							SOCK_DGRAM,		// Socket type
							IPPROTO_UDP);	// Protocol
	if (theSocket == INVALID_SOCKET)
	{
		TRACE("%s: %d\n","socket()",WSAGetLastError());
	}
	else
	{
		// Fill in the address structure
		SOCKADDR_IN saClient;
		saClient.sin_family = AF_INET;
		saClient.sin_addr.s_addr = INADDR_ANY;	// Let WinSock assign address
		saClient.sin_port = htons(2190);		// Use port passed from user
		// bind the name to the socket
		int nRet = bind(theSocket,		// Socket descriptor
				(LPSOCKADDR)&saClient,	// Address to bind to
				sizeof(SOCKADDR_IN)		// Size of address
				);
		if (nRet == SOCKET_ERROR)
		{
			TRACE("%s: %d\n","bind()",WSAGetLastError());
			closesocket(theSocket);
		}
		else
		{
			char szBuf[2048];
			int nLen = sizeof(SOCKADDR_IN);
			nRet = recvfrom(theSocket,			// Bound socket
						szBuf,					// Receive buffer
						sizeof(szBuf),			// Size of buffer in bytes
						0,						// Flags
						(LPSOCKADDR)&saServer,	// Buffer to receive client address 
						&nLen);					// Length of client address buffer

			if (nRet == INVALID_SOCKET)
			{
				TRACE("%s: %d\n","recvfrom()",WSAGetLastError());
			}
			else
			{
				// here's where I should look at what I recieve.
				CStringA csServerBroadcast(szBuf, nRet);
				csServerBroadcast.Replace("\n", " ");
				csServerBroadcast.Trim();
				std::cout << "[" << getTimeISO8601() << "] " << inet_ntoa(saServer.sin_addr) << " " << csServerBroadcast.GetString() << endl;
//				printf("%s\t%s\n", inet_ntoa(saServer.sin_addr), csServerBroadcast.GetString());
				rval = true;

				//if (strncmp(szBuf,csServerBroadcast,nRet) == 0)
				//{
				//	TRACE("Server at: %s\n",inet_ntoa(saServer.sin_addr));
				//	rval = true;
				//}
				//else
				//	TRACE("Someone not wanting to be decoded at: %s\n",inet_ntoa(saServer.sin_addr));
			}
			closesocket(theSocket);
		}
	}
	return(rval);
}
UINT TiVoBeaconListenThread(LPVOID lvp)
{
	//static CString csPutDecodedTags;
	//if (csPutDecodedTags.IsEmpty()) 
	//	csPutDecodedTags.LoadString(IDS_PUT_DECODED_TAG);
	//SOCKADDR_IN saServer;
	//while ((theApp.UDPClientListen(saServer) == true) && 
	//	(theApp.m_bDecodeRemoteStopRequested == false))
	//{
	//	bool bHappyWithCurrentServer = true;
	//	// Complete the address structure
	//	saServer.sin_family = AF_INET;
	//	saServer.sin_port = htons(4096);
	//	// these I want to optimize and only ask for if they changed
	//	CCorelateParameters D4Params;
	//	void * EPCParams = NULL;
	//	bool bD4Correlate = true;
	//	vector<CTagData> Hints;
	//	while ((bHappyWithCurrentServer == true) &&
	//		(theApp.m_bDecodeRemoteStopRequested == false))
	//	{
	//		CRawReadData RawData;
	//		bHappyWithCurrentServer = GetRawData(saServer,RawData);
	//		if (bHappyWithCurrentServer)
	//		{
	//			CTime OldTime(D4Params.m_ChangeTime.time);
	//			bHappyWithCurrentServer = GetParams(saServer,D4Params);
	//			if (!bHappyWithCurrentServer)
	//			{
	//				bHappyWithCurrentServer = GetParams(saServer,&EPCParams);
	//				if (bHappyWithCurrentServer)
	//				{
	//					bD4Correlate = false;
	//					D4Params.m_ChangeTime.time = theApp.EPCTagDLL.GetEPCModificationTime(EPCParams); // hack
	//				}
	//			}
	//			CTime NewTime(D4Params.m_ChangeTime.time);
	//			bool NewParams = (OldTime != NewTime);
	//			if (bHappyWithCurrentServer)
	//			{
	//				if (NewParams)
	//					bHappyWithCurrentServer = GetHints(saServer,Hints);
	//				if (bHappyWithCurrentServer)
	//				{
	//					if (NewParams)
	//					{
	//						D4Params.m_TagList.resize(Hints.size());
	//						for (int index = 0, stop = (int) Hints.size(); index < stop; index++)
	//						{
	//							D4Params.m_TagList[index].m_lTagNumber = Hints[index].EPC.i[3];
	//							bHappyWithCurrentServer = GetRefData(saServer,D4Params.m_TagList[index]);
	//						}
	//					}
	//					CTagData NewTags[30];
	//					// Then I decode the tags.
	//					int Count = 0;
	//					if (!Hints.empty())
	//					{
	//						if (bD4Correlate == true)
	//							Count = ReadTagD4(&RawData,NewTags,sizeof(NewTags),&(Hints[0]),sizeof(CTagData),&D4Params);
	//						else
	//							Count = theApp.EPCTagDLL.ReadTagEPC(&RawData,NewTags,sizeof(NewTags),&(Hints[0]),sizeof(CTagData),EPCParams);
	//					}
	//					else
	//					{
	//						if (bD4Correlate == true)
	//							Count = ReadTagD4(&RawData,NewTags,sizeof(NewTags),NULL,0,&D4Params);
	//						else
	//							Count = theApp.EPCTagDLL.ReadTagEPC(&RawData,NewTags,sizeof(NewTags),NULL,0,EPCParams);
	//					}
	//					TRACE("\nDetected %d Tags\n",Count);
	//					char xmltags[1024*25];
	//					int offset = sprintf(xmltags,"<RawDataInfo><TagCount %d><DataType %d><DateTime>%d.%03d %d %d</DateTime></RawDataInfo>\n",
	//						Count,
	//						RawData.DataType,
	//						RawData.m_FileTime.time,
	//						RawData.m_FileTime.millitm,
	//						RawData.m_FileTime.timezone,
	//						RawData.m_FileTime.dstflag);
	//					// Write the tags back to a buffer.
	//					for (int index = 0; index < Count; index++)
	//						offset += NewTags[index].XMLWrite(xmltags+offset,sizeof(xmltags)-offset);
	//					xmltags[offset] = '\0';

	//					CString csRequest;
	//					csRequest.Format("%s HTTP/1.1\r\nHost: %s\r\n",csPutDecodedTags,inet_ntoa(saServer.sin_addr));
	//					csRequest += "Connection: close\r\n";
	//					//csRequest += "User-Agent: EPCPalletReader\r\n";
	//					csRequest.Format("%sContent-Length: %d\r\n\r\n",csRequest,offset);
	//					csRequest += xmltags;

	//					// then I connect back to the server and upload the tagdata.
	//					// connect to the server
	//					// Create a TCP/IP stream socket
	//					SOCKET TagSocket = socket(AF_INET,		// Address family
	//										SOCK_STREAM,		// Socket type
	//										IPPROTO_TCP);		// Protocol
	//					if (TagSocket == INVALID_SOCKET) 
	//					{
	//						TRACE("%s: %d\n","socket()",WSAGetLastError());
	//						bHappyWithCurrentServer = false;
	//					}
	//					else
	//					{
	//						// connect to the server
	//						int nRet = connect(TagSocket,	// Socket
	//							(LPSOCKADDR)&saServer,		// Server address
	//							sizeof(struct sockaddr));	// Length of server address structure
	//						nRet = send(TagSocket,csRequest,csRequest.GetLength(),0);
	//						nRet = recv(TagSocket,xmltags,sizeof(xmltags),0);
	//						closesocket(TagSocket);
	//						#ifdef _DEBUG
	//						CString csTrace(csRequest,132);
	//						csTrace.Replace("\r\n"," ");
	//						TRACE("==> %s\n",csTrace);
	//						CString csResponse(xmltags,nRet);
	//						csResponse.Replace("\r\n"," ");
	//						TRACE("<== %s\n",csResponse);
	//						#endif
	//					}
	//				}
	//			}
	//		}
	//	}
	//	theApp.EPCTagDLL.DestroyHistoryEPC(EPCParams);
	//}
	//theApp.m_bDecodeRemoteRunning = false;
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
		success = SendStatusToSCM(SERVICE_STOP_PENDING,NO_ERROR,0,1,5000);
		csSubstitutionText.Format(_T("Service %s is stopping"),theApp.m_pszAppName);
		closesocket(ControlSocket);
		ControlSocket = INVALID_SOCKET;
		//		SetEvent(terminateEvent);
		break;
	case SERVICE_CONTROL_PAUSE:
		if (pauseService == false)
		{
			SendStatusToSCM(SERVICE_PAUSE_PENDING,NO_ERROR,0,0,0);
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
	serviceStatusHandle = RegisterServiceCtrlHandlerEx(theApp.m_pszAppName,
		(LPHANDLER_FUNCTION_EX)ServiceCtrlHandler,NULL);
	if (serviceStatusHandle != 0)
	{
		BOOL success = SendStatusToSCM(SERVICE_START_PENDING,NO_ERROR,0,1,5000);
		if (success == FALSE) // error
		{
			SendStatusToSCM(SERVICE_STOPPED,GetLastError(),0,0,0);
		}
		else
		{
			terminateEvent = CreateEvent(0,TRUE,FALSE,0);
			if (terminateEvent == NULL) // error
			{
				//#ifdef _DEBUG
				//if (ApplicationLogHandle != NULL) 
				//{
				//	LPCTSTR lpStrings[] = { _T("terminateEvent == NULL"), NULL };
				//	ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
				//}
				//#endif
				SendStatusToSCM(SERVICE_STOPPED,GetLastError(),0,0,0);
			}
			else
			{
				success = SendStatusToSCM(SERVICE_START_PENDING,NO_ERROR,0,2,1000);
				if (success == FALSE) // error
				{
					CloseHandle(terminateEvent);
					SendStatusToSCM(SERVICE_STOPPED,GetLastError(),0,0,0);
				}
				else
				{
					//theApp.FFTWLoad();
					//threadHandle = AfxBeginThread(HTTPDecodeClientThread, NULL);
					threadHandle = AfxBeginThread(HTTPMain, NULL, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
					if (threadHandle != NULL)
					{
						threadHandle->m_bAutoDelete = false;
						threadHandle->ResumeThread();
						success = SendStatusToSCM(SERVICE_RUNNING, NO_ERROR, 0, 0, 0);
						if (success != FALSE) 
						{
							if (ApplicationLogHandle != NULL) 
							{
								TCHAR UserNameBuff[256];
								DWORD UserNameSize = sizeof(UserNameBuff)/sizeof(TCHAR);
								GetUserName(UserNameBuff,&UserNameSize); // this is "wim" when I run it.
								CString csSubstitutionText;
								csSubstitutionText.Format(_T("Service %s has been started by %s"),theApp.m_pszAppName,UserNameBuff);
								LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
								//ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
							}
							WaitForSingleObject(terminateEvent,INFINITE);
						}
						if (terminateEvent)
							CloseHandle(terminateEvent);
						if (threadHandle)
						{
							delete threadHandle;
							threadHandle = NULL;
						}
						if (serviceStatusHandle)
							SendStatusToSCM(SERVICE_STOPPED,GetLastError(),0,0,500);
					}
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
volatile bool bRun = true;
void SignalHandler(int signal)
{
	bRun = false;
	cerr << "[" << getTimeISO8601() << "] SIGINT: Caught Ctrl-C, cleaning up and moving on." << endl;
	CStringA csTiVoPacket("tivoconnect=0\r\n");
	TiVoBeaconSend(csTiVoPacket);
}
extern bool TiVoBeaconListen(SOCKADDR_IN &saServer);
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
		//theApp.SetRegistryKey(_T("WimsWorld"));
		CString Parameters(theApp.m_lpCmdLine);
		if (argc > 1)
			Parameters = argv[1];
		TCHAR UserNameBuff[256];
		DWORD UserNameSize = sizeof(UserNameBuff)/sizeof(TCHAR);
		GetUserName(UserNameBuff,&UserNameSize); // this is "wim" when I run it.
		CString UserName(UserNameBuff,UserNameSize);

		CString csMyProgramGuid(theApp.GetProfileString(_T("Settings"), _T("GUID")));
		if (csMyProgramGuid.IsEmpty())
		{
			GUID MyProgramGuid;
			if (SUCCEEDED(CoCreateGuid(&MyProgramGuid)))
			{
				OLECHAR MyProgramGuidString[40] = {0};
				StringFromGUID2(MyProgramGuid, MyProgramGuidString, sizeof(MyProgramGuidString) / sizeof(OLECHAR));
				csMyProgramGuid = CString(MyProgramGuidString);
				theApp.WriteProfileString(_T("Settings"), _T("GUID"), csMyProgramGuid);
			}
		}

		CString csMyHostName;
		if (csMyHostName.IsEmpty())
		{
			char MyHostName[255]; // winsock hostname used for data recordkeeping
			gethostname(MyHostName,sizeof(MyHostName)); 
			csMyHostName = MyHostName;
		}

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

		av_register_all(); // FFMPEG initialization

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
				SC_HANDLE newService = CreateService(
					scm,
					theApp.m_pszAppName,
					_T("WimTiVoServer"),
					SERVICE_ALL_ACCESS,
					SERVICE_WIN32_OWN_PROCESS,
					SERVICE_AUTO_START,
					SERVICE_ERROR_NORMAL,
					tcModuleFileName,
					0,0,0,0,0);
				if (newService != NULL)
				{
					// successfully installed the service
					// Add a description to the entry.
					SERVICE_DESCRIPTION servdesc;
					servdesc.lpDescription = _TEXT("WimsWorld TiVo Server");
					ChangeServiceConfig2(newService,SERVICE_CONFIG_DESCRIPTION,&servdesc);

					// set up the event log stuff.
					// Add your source name as a subkey under the Application 
					// key in the EventLog registry key. 
					HKEY hk; 
					CString csRegKey("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
					csRegKey.Append(theApp.m_pszAppName);
					if (ERROR_SUCCESS == RegCreateKey(HKEY_LOCAL_MACHINE,csRegKey, &hk))
					{
						// Add the name to the EventMessageFile subkey. 
						RegSetValueEx(hk,		// subkey handle 
							_T("EventMessageFile"),	// value name 
							0,					// must be zero 
							REG_EXPAND_SZ,		// value type 
							(LPBYTE) tcModuleFileName,	// pointer to value data 
							ModuleFileNameBytes);// length of value data 
						// Set the supported event types in the TypesSupported subkey.
						DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
						RegSetValueEx(hk,		// subkey handle 
							_T("TypesSupported"),	// value name 
							0,					// must be zero 
							REG_DWORD,			// value type 
							(LPBYTE) &dwData,	// pointer to value data 
							sizeof(DWORD));		// length of value data 
						RegCloseKey(hk); 
						// Here I atempt to write a message
						_tprintf(_T("Sucessfully installed %s as a service\n"),tcModuleFileName);
						HANDLE h = RegisterEventSource(NULL,  // uses local computer 
									theApp.m_pszAppName);	// source name 
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
			_tprintf(_T("Removing Registry Entries\n"));
			CString csRegKey(_T("Software\\WimsWorld\\"));
			csRegKey.Append(theApp.m_pszAppName);
			theApp.DelRegTree(HKEY_CURRENT_USER, csRegKey);
		}
		// If I'm running as SYSTEM, I assume that I'm running as a service.
		else if (UserName.CompareNoCase(_T("SYSTEM")) == 0)
		{
			ApplicationLogHandle = RegisterEventSource(NULL,theApp.m_pszAppName);
			if (ApplicationLogHandle != NULL) 
			{
				CString csSubstitutionText;
				csSubstitutionText.Format(_T("Service %s has been started by %s"), theApp.m_pszAppName, UserNameBuff);
				LPCTSTR lpStrings[] = { csSubstitutionText.GetString(), NULL };
				ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
			}
			SERVICE_TABLE_ENTRY serviceTable[] =
			{
				//{ m_pszAppName, (LPSERVICE_MAIN_FUNCTION) ServiceMain },
				{ NULL, (LPSERVICE_MAIN_FUNCTION) ServiceMain },
				{ NULL, NULL }
			};
			serviceTable[0].lpServiceName = (LPWSTR) theApp.m_pszAppName;
			BOOL success = StartServiceCtrlDispatcher(serviceTable);
			if (ApplicationLogHandle != NULL) 
			{
				CString csSubstitutionText;
				csSubstitutionText.Format(_T("StartServiceCtrlDispatcher returned %d (TRUE = %d)"), success, TRUE);
				LPCTSTR lpStrings[] = { LPCTSTR(csSubstitutionText), NULL };
				ReportEvent(ApplicationLogHandle,EVENTLOG_INFORMATION_TYPE,0,WIMSWORLD_EVENT_GENERIC,NULL,1,0,lpStrings,NULL);
				DeregisterEventSource(ApplicationLogHandle);
			}
		}
		// Otherwise I assume I've been run in a console window
		else
		{
			bConsoleExists = true;
			std::cout << "[" << getTimeISO8601() << "] Running Application from the command line." << endl;
			std::cout << "[" << getTimeISO8601() << "] Built on " << __DATE__ << " at " <<  __TIME__ << endl;
			//std::wcout << L"[                   ] Snowman: ☃" << endl;
			std::cout << "[" << getTimeISO8601() << "] Use key combination Ctrl-C to end the program." << endl;
			std::locale mylocale("");   // get global locale
			std::locale OriginalLocale(std::cout.imbue(mylocale));  // imbue global locale
			std::cout << "[" << getTimeISO8601() << "] Test of digit grouping: " << 1024000 << endl;
			std::cout.imbue(OriginalLocale);
			std::cout << "[" << getTimeISO8601() << "] Test of digit grouping: " << 1024000 << endl;

			char szDescription[8][32] = {
				"NetBIOS", 
				"DNS hostname", 
				"DNS domain", 
				"DNS fully-qualified", 
				"Physical NetBIOS", 
				"Physical DNS hostname", 
				"Physical DNS domain", 
				"Physical DNS fully-qualified"};
			TCHAR buffer[256] = TEXT("");
			DWORD dwSize = sizeof(buffer);
			std::cout << "[" << getTimeISO8601() << "] Determining Machine name with various methods:" << endl;
			for (int cnf = 0; cnf < ComputerNameMax; cnf++)
			{
				if (!GetComputerNameEx((COMPUTER_NAME_FORMAT)cnf, buffer, &dwSize))
				{
					std::cout << "[" << getTimeISO8601() << "] GetComputerNameEx failed (" << GetLastError() << ")" << endl;
					break;
				}
				else
					std::cout << "[" << getTimeISO8601() << "] " << setw(28) << right << szDescription[cnf] << " : " << left << CStringA(buffer).GetString() << endl;
				dwSize = _countof(buffer);
				ZeroMemory(buffer, dwSize);
			}

			TiVoFileList.reserve(1000);
			if (!csMyHostName.CompareNoCase(_T("INSPIRON")))
			{
				#ifdef _WIM_TIVO_CONVERT_
				CFileFind finder;
				BOOL bWorking = finder.FindFile(_T("//Acid/TiVo/*.tivo"));
				while (bWorking)
				{
					bWorking = finder.FindNextFile();
					if (finder.IsDots())
						continue;
					if (finder.IsDirectory())
						continue;
					if (finder.IsHidden())
						continue;
					if (finder.IsSystem())
						continue;
					if (finder.IsTemporary())
						continue;
					if (finder.GetLength() > 0)
					{
						cTiVoFile myFile;
						myFile.SetPathName(finder);
						myFile.SetMAK(_T("1760168186"));
						TiVoFilesToConvert.push(myFile);
					}
				}
				finder.Close();
				AfxBeginThread(TiVoConvertFileThread, NULL);
				#endif
				//PopulateTiVoFileList(TiVoFileList, "//Acid/TiVo/*.TiVo");
				//PopulateTiVoFileList(TiVoFileList, "//Acid/TiVo/Evening*.TiVo");
				//PopulateTiVoFileList(TiVoFileList, "D:/Videos/Evening Magazine (Recorded Mar 26, 2010, KINGDT).TiVo");
				PopulateTiVoFileList(TiVoFileList, "D:/Recorded TV/*.wtv");
				//PopulateTiVoFileList(TiVoFileList, "//Acid/TiVo/*.TiVo");
				PopulateTiVoFileList(TiVoFileList, "//Acid/TiVo/*");
				//PopulateTiVoFileList(TiVoFileList, "//Acid/TiVo/Archer.*");
			}
			else
			{
				PopulateTiVoFileList(TiVoFileList, "//Acid/TiVo/Evening*.TiVo");
				PopulateTiVoFileList(TiVoFileList, "C:/Users/Wim/Videos/*.mp4");
			}
			std::sort(TiVoFileList.begin(),TiVoFileList.end(),cTiVoFileCompareDate);
			std::cout << "[" << getTimeISO8601() << "] TiVoFileList Size: " << TiVoFileList.size() << endl;

			#ifdef _Original_Download_Tests_
			if (SUCCEEDED(CoInitializeEx(0, COINIT_MULTITHREADED))) // COINIT_APARTMENTTHREADED
			{
				//XML_Test_FileReformat(_T("D:\\Videos\\chunk-01-0001.xml"), _T("D:/Videos/Evening Magazine (Recorded Mar 26, 2010, KINGDT).1.xml"));
				//XML_Test_FileReformat(_T("D:\\Videos\\chunk-02-0002.xml"), _T("D:/Videos/Evening Magazine (Recorded Mar 26, 2010, KINGDT).2.xml"));
				//XML_Test_Read_ElementsOnly();
				//XML_Test_Read();
				//XML_Test_Write();
				//XML_Test_Write_InMemory();
				std::vector<cTiVoFile> FilesToGetFromTiVo;
				std::vector<CTiVoContainer> TiVoContainers;
				//XML_Parse_TiVoNowPlaying(_T("WimTivoServer.3.xml"), FilesToGetFromTiVo);
				//std::sort(FilesToGetFromTiVo.begin(),FilesToGetFromTiVo.end(),cTiVoFileCompareDateReverse);
				FilesToGetFromTiVo.clear();
				CInternetSession serverSession0;
				XML_Parse_TiVoNowPlaying(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying&Recurse=Yes&SortOrder=!CaptureDate")), FilesToGetFromTiVo, TiVoContainers, serverSession0);
				std::sort(FilesToGetFromTiVo.begin(),FilesToGetFromTiVo.end(),cTiVoFileCompareDateReverse);				
				FilesToGetFromTiVo.clear();	// This line is temporary just to make sure that no files are downloaded or converted
				for (auto TiVoFileToGet = FilesToGetFromTiVo.begin(); TiVoFileToGet != FilesToGetFromTiVo.end(); TiVoFileToGet++)
				{
					CString csPathName(_T("//Acid/TiVo/"));
					csPathName.Append(TiVoFileToGet->GetPathName());
					CFileStatus status;
					if (TRUE != CFile::GetStatus(csPathName, status))
						while (false == GetTiVoFile(*TiVoFileToGet, serverSession0, _T("1760168186"), _T("//Acid/TiVo/")));
					if (TRUE == CFile::GetStatus(csPathName, status))
					{
						status.m_ctime = status.m_mtime = TiVoFileToGet->GetCaptureDate();
						CFile::SetStatus(csPathName, status);
						CString csMPEGPathName(csPathName);
						csMPEGPathName.Replace(_T(".TiVo"), _T(".mpeg"));
						CString csMP4PathName(csPathName);
						csMP4PathName.Replace(_T(".TiVo"), _T(".mp4"));
						if (TRUE != CFile::GetStatus(csMP4PathName, status)) // Only do a bunch of this stuff if the mp4 file doesn't already exist
						{
							// This is just demo code making sure I can create a temporary file properly for future use with tdcat or tivodecode
							TCHAR lpTempPathBuffer[MAX_PATH];
							DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
							if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
							{
								std::cout << "[" << getTimeISO8601() << "] GetTempPath failed" << endl;
								_tcscpy(lpTempPathBuffer,_T("."));
							}
							//  Generates a temporary file name. 
							TCHAR szTempFileName[MAX_PATH];  
							if (0 != GetTempFileName(lpTempPathBuffer, AfxGetAppName(), 0, szTempFileName))
							{
								wcout << L"[                   ]  GetTempFileName: " << szTempFileName << endl;
								if (-1 == _tspawnlp(_P_WAIT, _T("tdcat.exe"), _T("tdcat.exe"), _T("--mak"), _T("1760168186"), _T("--out"), szTempFileName, _T("--chunk-2"), QuoteFileName(csPathName).GetString(), NULL))
									std::cout << "[" << getTimeISO8601() << "] _tspawnlp failed: " << _sys_errlist[errno] << endl;

								if (TRUE != CFile::GetStatus(csMPEGPathName, status))
									if (-1 == _tspawnlp(_P_WAIT, _T("tivodecode.exe"), _T("tivodecode.exe"), _T("--mak"), _T("1760168186"), _T("--out"), QuoteFileName(csMPEGPathName).GetString(), QuoteFileName(csPathName).GetString(), NULL))
										std::cout << "[" << getTimeISO8601() << "] _tspawnlp failed: " << _sys_errlist[errno] << endl;
								if (TRUE == CFile::GetStatus(csMPEGPathName, status))
								{
									status.m_ctime = status.m_mtime = TiVoFileToGet->GetCaptureDate();
									CFile::SetStatus(csMPEGPathName, status);
									if (TRUE != CFile::GetStatus(csMP4PathName, status))
									{
										CString csTitle(TiVoFileToGet->GetTitle()); csTitle.Insert(0,_T("title=\""));csTitle.Append(_T("\""));
										CString csShow(TiVoFileToGet->GetTitle()); csShow.Insert(0,_T("show=\""));csShow.Append(_T("\""));
										CString csDescription(TiVoFileToGet->GetDescription()); csDescription.Insert(0,_T("description=\""));csDescription.Append(_T("\""));
										CString csEpisodeID(TiVoFileToGet->GetEpisodeTitle()); csEpisodeID.Insert(0,_T("episode_id=\""));csEpisodeID.Append(_T("\""));
										if (-1 == _tspawnlp(_P_WAIT, _T("ffmpeg.exe"), _T("ffmpeg.exe"), _T("-i"), QuoteFileName(csMPEGPathName).GetString(), 
											_T("-metadata"), csTitle.GetString(),
											_T("-metadata"), csShow.GetString(),
											_T("-metadata"), csDescription.GetString(),
											_T("-metadata"), csEpisodeID.GetString(),
											_T("-vcodec"), _T("copy"),
											_T("-acodec"), _T("copy"),
											_T("-y"), // Cause it to overwrite exiting output files
											QuoteFileName(csMP4PathName).GetString(), NULL))
											std::cout << "[" << getTimeISO8601() << "] _tspawnlp failed: " << _sys_errlist[errno] << endl;
									}
									if (TRUE == CFile::GetStatus(csMP4PathName, status))
									{
										status.m_ctime = status.m_mtime = TiVoFileToGet->GetCaptureDate();
										CFile::SetStatus(csMP4PathName, status);
										DeleteFile(csMPEGPathName);
									}
								}
								DeleteFile(szTempFileName);	// I must delete this file because the zero in the unique field up above causes a file to be created.
							}
						}
					}
				}

				std::vector<CString> myURLS;
				//myURLS.push_back(CString(_T("http://192.168.0.108/TiVoConnect?Command=ResetServer")));
				//myURLS.push_back(CString(_T("http://192.168.0.108/TiVoConnect?Command=QueryContainer&Container=/")));
				myURLS.push_back(CString(_T("http://192.168.0.108/TiVoConnect?Command=QueryFormats&SourceFormat=video%2Fx-tivo-mpeg")));
				//myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying")));
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying&Recurse=Yes&SortOrder=!CaptureDate")));
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying/9948")));
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoVideoDetails?id=984268")));	// It appears that after a "Details" call it uses a "Connection: keep-alive" call, so I should probably make another call to the now playing list to see if that makes the tivo more stable.
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying/9948")));
				//myURLS.push_back(CString(_T("http://192.168.1.11/TiVoConnect?Command=QueryContainer&Container=/")));
				//myURLS.push_back(CString(_T("https://tivo:9371539867@192.168.1.11:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying")));
				//myURLS.push_back(CString(_T("http://192.168.0.5:8080/TiVoConnect?Command=QueryContainer&Container=/")));
				//myURLS.push_back(CString(_T("http://192.168.0.5:8080/TiVoConnect?Command=QueryFormats&SourceFormat=video%2Fx-tivo-mpeg")));
				//myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.5:4430/TiVoConnect?Command=QueryContainer&Container=/TivoNowPlaying")));

				//myURLS.push_back(CString(_T("http://tivo:1760168186@192.168.0.108/download/Evening Magazine.TiVo?Container=/NowPlaying&id=984268")));
				//myURLS.push_back(CString(_T("http://tivo:1760168186@192.168.0.108/download/Evening Magazine.TiVo?Container=/NowPlaying&id=879493")));
				//myURLS.push_back(CString(_T("http://tivo:1760168186@192.168.0.108/download/Community.TiVo?Container=/NowPlaying&id=3002485")));
				//myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=ResetServer")));
				myURLS.push_back(CString(_T("http://tivo:1760168186@192.168.0.108/TiVoConnect?Command=ResetServer")));
				int FileIndex = 0;
				CInternetSession serverSession;
				for (auto csURL = myURLS.begin(); csURL != myURLS.end(); csURL++)
				{
					std::cout << "[" << getTimeISO8601() << "] Attempting: " << CStringA(*csURL).GetString() << endl;
					DWORD dwServiceType;
					CString strServer;
					CString strObject; 
					INTERNET_PORT nPort; 
					CString strUsername; 
					CString strPassword; 
					AfxParseURLEx(csURL->GetString(), dwServiceType, strServer, strObject, nPort, strUsername, strPassword);
					std::wcout << L"[                   ] strServer: " << strServer.GetString() << endl;
					std::wcout << L"[                   ] strObject: " << strObject.GetString() << endl;
					std::wcout << L"[                   ] nPort: " << nPort << endl;
					std::wcout << L"[                   ] strUsername: " << strUsername.GetString() << endl;
					std::wcout << L"[                   ] strPassword: " << strPassword.GetString() << endl;
					std::unique_ptr<CHttpConnection> serverConnection(serverSession.GetHttpConnection(strServer,nPort,strUsername,strPassword));
					if (NULL != serverConnection)
					{
						DWORD dwFlags = INTERNET_FLAG_TRANSFER_BINARY | SECURITY_IGNORE_ERROR_MASK;
						if (dwServiceType == AFX_INET_SERVICE_HTTPS)
							dwFlags |= INTERNET_FLAG_SECURE;
						std::unique_ptr<CHttpFile> serverFile(serverConnection->OpenRequest(1, strObject, NULL, 1, NULL, NULL, dwFlags));
						if (serverFile != NULL)
						{
							int BadCertErrorCount = 0;
							AGAIN:
							try 
							{
								//std::unique_ptr<CHttpFile> serverFile((CHttpFile*) serverSession.OpenURL(*csURL, 1, INTERNET_FLAG_TRANSFER_BINARY));
								serverFile->SendRequest();
								DWORD dwRet;
								serverFile->QueryInfoStatusCode(dwRet);
								#ifdef _DEBUG
								//std::wcout << L"[                   ] Server Status Code: " << dwRet << endl;
								CString headers;
								serverFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF,headers);
								headers.Trim();
								headers.Replace(_T("\r\n"),_T("\r\n[                   ] HTTP_QUERY_RAW_HEADERS_CRLF: "));
								std::wcout << L"[                   ] HTTP_QUERY_RAW_HEADERS_CRLF: " << headers.GetString() << endl;
								#endif
								if(dwRet == HTTP_STATUS_OK)
								{
									CString csCookie;
									serverFile->QueryInfo(HTTP_QUERY_SET_COOKIE, csCookie);
									if (!csCookie.IsEmpty())
									{
										std::wcout << L"[                   ] HTTP_QUERY_SET_COOKIE: " << csCookie.GetString() << endl;
										// sid=1BFA53E13BDF178B; path=/; expires="Saturday, 16-Feb-2013 00:00:00 GMT";
										CString csCookieName(csCookie.Left(csCookie.Find(_T("="))));
										CString csCookieData(csCookie.Left(csCookie.Find(_T(";"))));;
										csCookieData.Delete(0,csCookieData.Find(_T("="))+1);
										CString csCookiePath(csCookie);
										csCookiePath.Delete(0,csCookiePath.Find(_T("path="))+5);
										csCookiePath.Delete(csCookiePath.Find(_T(";")),csCookiePath.GetLength());
										CString csCookieURL;
										if (dwServiceType == AFX_INET_SERVICE_HTTPS)
											csCookieURL = _T("https://");
										else
											csCookieURL = _T("https://");
										csCookieURL.Append(strServer);
										csCookieURL.AppendFormat(_T(":%d"), nPort);
										csCookieURL.Append(csCookiePath);
										std::wcout << L"[                   ] csCookieURL: " << csCookieURL.GetString() << endl;
										std::wcout << L"[                   ] csCookieName: " << csCookieName.GetString() << endl;
										std::wcout << L"[                   ] csCookieData: " << csCookieData.GetString() << endl;
										serverSession.SetCookie(csCookieURL,csCookieName,csCookieData);
									}									
									CString csContentType;
									serverFile->QueryInfo(HTTP_QUERY_CONTENT_TYPE, csContentType);
									if (0 < csContentType.Find(_T(";"))) // Fix issue of text/xml; charset=UTF-8
										csContentType.Delete(csContentType.Find(_T(";")),csContentType.GetLength());									
									if (!csContentType.CompareNoCase(_T("text/xml")))
									{								
										CComPtr<IStream> spMemoryStreamOne(::SHCreateMemStream(NULL, 0));
										CComPtr<IStream> spMemoryStreamTwo(::SHCreateMemStream(NULL, 0));
										if ((spMemoryStreamOne != NULL) && (spMemoryStreamTwo != NULL))
										{
											char ittybittybuffer;
											ULONG cbWritten;
											while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
												spMemoryStreamOne->Write(&ittybittybuffer, 1, &cbWritten);

											// reposition back to beginning of stream
											LARGE_INTEGER position;
											position.QuadPart = 0;
											spMemoryStreamOne->Seek(position, STREAM_SEEK_SET, NULL);

											HRESULT hr = S_OK;
											CComPtr<IXmlReader> pReader; 
											if (SUCCEEDED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL))) 
											{
												if (SUCCEEDED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) 
												{
													if (SUCCEEDED(hr = pReader->SetInput(spMemoryStreamOne))) 
													{
														CComPtr<IXmlWriter> pWriter;
														if (SUCCEEDED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
														{
															pWriter->SetOutput(spMemoryStreamTwo);
															pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
															while (S_OK == (hr = pWriter->WriteNode(pReader, TRUE)));	// loops over entire xml file, writing it out with indenting
															//pWriter->WriteEndDocument();
															pWriter->Flush();

															// Allocates enough memeory for the xml content.
															STATSTG ssStreamData = {0};
															spMemoryStreamTwo->Stat(&ssStreamData, STATFLAG_NONAME);
															SIZE_T cbSize = ssStreamData.cbSize.LowPart;
															char *XMLDataBuff = new char[cbSize+1];

															// Copies the content from the stream to the buffer.
															LARGE_INTEGER position;
															position.QuadPart = 0;
															spMemoryStreamTwo->Seek(position, STREAM_SEEK_SET, NULL);

															ULONG cbRead;
															spMemoryStreamTwo->Read(XMLDataBuff, cbSize, &cbRead);
															XMLDataBuff[cbSize] = '\0';
															std::stringstream OutPutFileName;
															//std::stringstream ReturnedData;
															OutPutFileName << "WimTivoServer." << FileIndex++ << ".xml";
															std::cout << "[                   ] Writing File: " << OutPutFileName.str() << endl;
															std::ofstream OutputFile(OutPutFileName.str(), ios_base::binary);
															if (OutputFile.is_open())
																OutputFile.write(XMLDataBuff,cbSize);
															OutputFile.close();
															delete[] XMLDataBuff;
														}
													}
												}
											}
										}
									}
									else if (!csContentType.CompareNoCase(_T("video/x-tivo-mpeg")))
									{
										std::stringstream OutPutFileName;
										OutPutFileName << "WimTivoServer." << FileIndex++ << ".TiVo";
										std::cout << "[                   ] Writing File: " << OutPutFileName.str() << endl;
										std::ofstream OutputFile(OutPutFileName.str(), ios_base::binary);
										if (OutputFile.is_open())
										{
											static size_t ReadWriteBufferSize = 1024;
											char* ReadWriteBuffer = new char[ReadWriteBufferSize];
											long long TotalRead = 0;
											UINT uiRead;
											CTime ctCurrent(CTime::GetCurrentTime());
											CTime ctStart(ctCurrent);
											CTime ctLastOutput(ctStart);
											CTimeSpan ctsTotal = ctCurrent - ctStart;
											std::locale mylocale("");   // get global locale
											std::locale OriginalLocale(std::cout.imbue(mylocale));  // imbue global locale
											std::cout << "[                   ] ReadWriteBufferSize: " << ReadWriteBufferSize << endl;
											while (0 < (uiRead = serverFile->Read(ReadWriteBuffer, ReadWriteBufferSize)))
											{
												OutputFile.write(ReadWriteBuffer,uiRead);
												TotalRead += uiRead;
												ctCurrent = CTime::GetCurrentTime();
												ctsTotal = ctCurrent - ctStart;
												if ((ctCurrent - ctLastOutput) > CTimeSpan(0,0,0,1))
												{
													std::cout << "[" << getTimeISO8601() << "] Elapsed: " << CStringA(ctsTotal.Format(_T("%H:%M:%S"))).GetString() << " Bytes: " << TotalRead << " bytes/second: " <<  (TotalRead / ctsTotal.GetTotalSeconds()) << "\r";
													ctLastOutput = ctCurrent;
												}
											}
											OutputFile.close();
											delete[] ReadWriteBuffer;
											std::cout << "[" << getTimeISO8601() << "] Elapsed: " << CStringA(ctsTotal.Format(_T("%H:%M:%S"))).GetString() << " Bytes: " << TotalRead << " bytes/second: " <<  (TotalRead / ctsTotal.GetTotalSeconds()) << " ReadWriteBufferSize: " << ReadWriteBufferSize << endl;
											std::cout.imbue(OriginalLocale);
											ReadWriteBufferSize *= 10;
										}
									}
									else
									{
										std::cout << "[                   ] not text/xml or video/x-tivo-mpeg" << endl;
										char ittybittybuffer;
										while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
											std::cout << ittybittybuffer;
										std::cout << endl;
									}
								}
								else if (dwRet == HTTP_STATUS_SERVICE_UNAVAIL)
								{
									CString csRetry;
									if (0 < serverFile->QueryInfo(HTTP_QUERY_RETRY_AFTER, csRetry))
									{
										int iRetry = _ttoi(csRetry.GetString());
										std::cout << "[" << getTimeISO8601() << "] Sleeping for " << iRetry << " Seconds" << endl;
										Sleep(iRetry * 1000);
										csURL--;
									}
								}
								else if (serverFile->GetLength() > 0)
								{
									char ittybittybuffer;
									std::string ss;
									while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
										ss += ittybittybuffer;
									std::cout << "[                   ] Returned File: " << ss << endl;
								}
								serverFile->Close();
							}
							catch(CInternetException *e)
							{
								TCHAR   szCause[255];
								e->GetErrorMessage(szCause,sizeof(szCause)/sizeof(TCHAR));
								CStringA csErrorMessage(szCause);
								csErrorMessage.Trim();
								std::cout << "[                   ] InternetException: " <<  csErrorMessage.GetString() << " (" << e->m_dwError << ") " << endl;
								if ((e->m_dwError == ERROR_INTERNET_INVALID_CA) || 
									(e->m_dwError == ERROR_INTERNET_SEC_CERT_CN_INVALID) ||
									(e->m_dwError == ERROR_INTERNET_SEC_CERT_DATE_INVALID) ||
									(e->m_dwError == ERROR_INTERNET_SEC_INVALID_CERT) )
								{
									serverFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, SECURITY_SET_MASK);
									if (BadCertErrorCount++ < 2)
										goto AGAIN;
								}
							}
						}
					}
				}
			}
			CoUninitialize();

			if (csMyHostName.CompareNoCase(_T("INSPIRON")))
			{
				std::cout << "[" << getTimeISO8601() << "] Listening for TiVo Broadcast packets on UDP port 2190" << endl;
				// Set up CTR-C signal handler
				typedef void (*SignalHandlerPointer)(int);
				SignalHandlerPointer previousHandler = signal(SIGINT, SignalHandler);
				int loopCounter = 0;
				bRun = true;
				SOCKADDR_IN saServer;
				while (bRun && (TiVoBeaconListen(saServer) == true))
					std::cout << "[" << getTimeISO8601() << "] \r";
				// remove our special Ctrl-C signal handler and restore previous one
				signal(SIGINT, previousHandler);
				std::cout << "\n[" << getTimeISO8601() << "] No longer listening for TiVo Broadcast packets on UDP port 2190" << endl;
			}
			#endif

			terminateEvent = CreateEvent(0,TRUE,FALSE,0);
			if (terminateEvent != NULL) 
			{
				threadHandle = AfxBeginThread(HTTPMain, NULL);
				if (threadHandle != NULL)
				{
					DWORD dwVersion = GetVersion();
					int dwMajorVersion = (int)(LOBYTE(LOWORD(dwVersion)));
					int dwMinorVersion = (int)(HIBYTE(LOWORD(dwVersion)));
					int dwBuild = 0;
					if (dwVersion < 0x80000000)              
						dwBuild = (int)(HIWORD(dwVersion));

					cTiVoServer myServer;
					myServer.m_method = true?"broadcast":"connect";
					{
						std::stringstream ss;
						ss << "pc/WinNT:" << dwMajorVersion << "." << dwMinorVersion << "." << dwBuild;
						myServer.m_platform = ss.str();
					}
					myServer.m_machine = CStringA(csMyHostName).GetString();
					myServer.m_identity = CStringA(csMyProgramGuid).GetString();
					myServer.m_swversion = CStringA(csBuildDateTime).GetString();
					for (auto index = 8*60; index > 0; --index)
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
						}
						TiVoBeaconSend(CStringA(myServer.WriteTXT('\n').c_str()));
						Sleep(60 * 1000);
					}
					closesocket(ControlSocket);
					ControlSocket = INVALID_SOCKET;

					TRACE(__FUNCTION__ " Waiting for Thread to end\n");
					WaitForSingleObject(terminateEvent,INFINITE);

					if (terminateEvent)
						CloseHandle(terminateEvent);
				}
			}
		}
	}
	TRACE(__FUNCTION__ " Exiting\n");
	return nRetCode;
}
