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
// SHCreateMemStream function http://msdn.microsoft.com/en-us/library/windows/desktop/bb773831(v=vs.85).aspx
// SHCreateStreamOnFile function http://msdn.microsoft.com/en-us/library/windows/desktop/bb759864(v=vs.85).aspx

// Smart Pointer Reference: http://msdn.microsoft.com/en-us/library/hh279674.aspx
// WinINet vs. WinHTTP: http://msdn.microsoft.com/en-us/library/windows/desktop/hh227298(v=vs.85).aspx

#include "stdafx.h"
#include "WimTiVoServer.h"

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
	timer += _daylight*3600;
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
			cout << "[" << getTimeISO8601() << "] Error creating file writer, error is: " << hex << hr << endl;
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
int GetTiVoConnect(SOCKET DataSocket)
{
	TRACE(__FUNCTION__ "\n");
	int rval = 0;
	char MyHostName[255] = {0}; // winsock hostname used for data recordkeeping
	gethostname(MyHostName,sizeof(MyHostName)); 
	char XMLDataBuff[1024*11];

	CComPtr<IXmlWriter> pWriter;
	CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL);
	// from: http://stackoverflow.com/questions/3037946/how-can-i-store-xml-in-buffer-using-xmlite
	CComPtr<IStream> spMemoryStream;
	// Opens writeable output stream.
	spMemoryStream.Attach(::SHCreateMemStream(NULL, 0));
	if (spMemoryStream != NULL)
	{
		CComPtr<IXmlWriterOutput> pWriterOutput;
		CreateXmlWriterOutputWithEncodingName(spMemoryStream, NULL, L"UTF-8", &pWriterOutput);
		pWriter->SetOutput(pWriterOutput);
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
						pWriter->WriteString(CString(MyHostName).GetString());
						pWriter->WriteString(_T(" (WimTiVoServer)"));
					pWriter->WriteFullEndElement();
					pWriter->WriteStartElement(NULL,_T("TotalItems"),NULL);
						pWriter->WriteString(_T("1"));
					pWriter->WriteFullEndElement();
				pWriter->WriteFullEndElement();	// Details

				pWriter->WriteStartElement(NULL,_T("Item"),NULL);
					pWriter->WriteStartElement(NULL,_T("Details"),NULL);
						pWriter->WriteStartElement(NULL,_T("ContentType"),NULL);
							pWriter->WriteString(_T("x-tivo-container/tivo-videos"));
						pWriter->WriteFullEndElement();
						pWriter->WriteStartElement(NULL,_T("SourceFormat"),NULL);
							pWriter->WriteString(_T("x-tivo-container/tivo-dvr"));
						pWriter->WriteFullEndElement();
						pWriter->WriteStartElement(NULL,_T("Title"),NULL);
							pWriter->WriteString(CString(MyHostName).GetString());
							pWriter->WriteString(_T(" (WimTiVoServer)"));
						pWriter->WriteFullEndElement();
						pWriter->WriteStartElement(NULL,_T("UniqueId"),NULL);
							pWriter->WriteString(_T("TivoHD"));
						pWriter->WriteFullEndElement();
					pWriter->WriteFullEndElement();	// Details
					pWriter->WriteStartElement(NULL,_T("Links"),NULL);
						pWriter->WriteStartElement(NULL,_T("Content"),NULL);
							pWriter->WriteStartElement(NULL,_T("Url"),NULL);
								pWriter->WriteString(_T("/TiVoConnect?Command=QueryContainer&Container=%2FNowPlaying"));
								//pWriter->WriteString(_T("https://192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=%2FNowPlaying"));
							pWriter->WriteFullEndElement();
							pWriter->WriteStartElement(NULL,_T("ContentType"),NULL);
								pWriter->WriteString(_T("x-tivo-container/tivo-videos"));
							pWriter->WriteFullEndElement();
						pWriter->WriteFullEndElement();	// Content
					pWriter->WriteFullEndElement();	// Links
				pWriter->WriteFullEndElement();	// Item

				pWriter->WriteStartElement(NULL,_T("ItemStart"),NULL);
					pWriter->WriteString(_T("0"));
				pWriter->WriteFullEndElement();
				pWriter->WriteStartElement(NULL,_T("ItemCount"),NULL);
					pWriter->WriteString(_T("1"));
				pWriter->WriteFullEndElement();

			pWriter->WriteFullEndElement();	// TiVoContainer
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
	else
	{
		strcpy(XMLDataBuff,"<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n");
		strcat(XMLDataBuff,"<TiVoContainer>\n");
		strcat(XMLDataBuff,"  <Details>\n");
		strcat(XMLDataBuff,"    <Title>"); strcat(XMLDataBuff, MyHostName); strcat(XMLDataBuff," (WimTiVoServer)</Title>\n");
		strcat(XMLDataBuff,"    <ContentType>x-tivo-container/tivo-server</ContentType>\n");
		strcat(XMLDataBuff,"    <SourceFormat>x-tivo-container/folder</SourceFormat>\n");
		strcat(XMLDataBuff,"    <TotalItems>1</TotalItems>\n");
		strcat(XMLDataBuff,"  </Details>\n");
		strcat(XMLDataBuff,"  <Item>\n");
		strcat(XMLDataBuff,"    <Details>\n");
		strcat(XMLDataBuff,"      <Title>"); strcat(XMLDataBuff, MyHostName); strcat(XMLDataBuff," (WimTiVoServer)</Title>\n");
		strcat(XMLDataBuff,"      <ContentType>x-tivo-container/tivo-videos</ContentType>\n");
		strcat(XMLDataBuff,"      <SourceFormat>x-tivo-container/folder</SourceFormat>\n");
		strcat(XMLDataBuff,"    </Details>\n");
		strcat(XMLDataBuff,"    <Links>\n");
		strcat(XMLDataBuff,"      <Content>\n");
		strcat(XMLDataBuff,"        <Url>/TiVoConnect?Command=QueryContainer&amp;Container=%2FTivoNowPlaying</Url>\n");
		strcat(XMLDataBuff,"        <ContentType>x-tivo-container/tivo-videos</ContentType>\n");
		strcat(XMLDataBuff,"      </Content>\n");
		strcat(XMLDataBuff,"    </Links>\n");
		strcat(XMLDataBuff,"  </Item>\n");
		strcat(XMLDataBuff,"  <ItemStart>0</ItemStart>\n");
		strcat(XMLDataBuff,"  <ItemCount>1</ItemCount>\n");
		strcat(XMLDataBuff,"</TiVoContainer>\n");
		//strcat(XMLDataBuff,"<!-- Copyright © 2003-2010 TiVo Inc.-->\n");
	}
	/* Create HTTP Header */
	char tmpBuff[1024];
	char HttpResponse[1024];
	strcpy(HttpResponse,"HTTP/1.1 200 OK\r\n");
	strcat(HttpResponse,"Content-Type: text/html\r\n");
	strcat(HttpResponse,"Connection: close\r\n");
	time_t timer;
	/* Get Current Time */
	time(&timer);
	sprintf(tmpBuff,"Date: %s",asctime(gmtime(&timer)));
	tmpBuff[30] = '\0';
	strcat(tmpBuff," GMT\r\n");
	strcat(HttpResponse,tmpBuff);
	sprintf(tmpBuff,"Content-Length: %d\r\n",strlen(XMLDataBuff));
	strcat(HttpResponse,tmpBuff);
	strcat(HttpResponse,"\r\n");

	int nRet;
	nRet = send(DataSocket, HttpResponse, strlen(HttpResponse),0);
	nRet = send(DataSocket, XMLDataBuff, strlen(XMLDataBuff),0);
	return(0);
}
int GetTiVoQueryFormats(SOCKET DataSocket)
{
	TRACE(__FUNCTION__ "\n");
	int rval = 0;
	char MyHostName[255] = {0}; // winsock hostname used for data recordkeeping
	gethostname(MyHostName,sizeof(MyHostName)); 
	char XMLDataBuff[1024*11];
	strcpy(XMLDataBuff,"<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n");
	strcat(XMLDataBuff,"<TiVoFormats xmlns=\"http://www.tivo.com/developer/calypso-protocol-1.6/\">\n");
	strcat(XMLDataBuff,"  <Format>\n");
	strcat(XMLDataBuff,"    <ContentType>video/x-tivo-mpeg</ContentType>\n");
	strcat(XMLDataBuff,"    <Description/>\n");
	strcat(XMLDataBuff,"  </Format>\n");
	strcat(XMLDataBuff,"  <Format>\n");
	strcat(XMLDataBuff,"    <ContentType>video/x-tivo-raw-tts</ContentType>\n");
	strcat(XMLDataBuff,"    <Description/>\n");
	strcat(XMLDataBuff,"  </Format>\n");
	strcat(XMLDataBuff,"</TiVoFormats>\n");

	/* Create HTTP Header */
	char tmpBuff[1024];
	char HttpResponse[1024];
	time_t timer;
	/* Get Current Time */
	time(&timer);
	strcpy(HttpResponse,"HTTP/1.1 200 OK\r\n");
	strcat(HttpResponse,"Content-Type: text/html\r\n");
	strcat(HttpResponse,"Connection: close\r\n");
	sprintf(tmpBuff,"Date: %s",asctime(gmtime(&timer)));
	tmpBuff[30] = '\0';
	strcat(tmpBuff," GMT\r\n");
	strcat(HttpResponse,tmpBuff);
	sprintf(tmpBuff,"Content-Length: %d\r\n",strlen(XMLDataBuff));
	strcat(HttpResponse,tmpBuff);
	strcat(HttpResponse,"\r\n");

	int nRet;
	nRet = send(DataSocket, HttpResponse, strlen(HttpResponse),0);
	nRet = send(DataSocket, XMLDataBuff, strlen(XMLDataBuff),0);
	return(0);
}
string TiVoFileItemtoXML(void)
{
	ostringstream XMLItem;
	XMLItem << "<Item>\n";

	//time_t timer = TheTime;
	//struct tm * UTC = gmtime(&timer);
	//if (UTC != NULL)
	//{
	//	ISOTime.fill('0');
	//	ISOTime << UTC->tm_year+1900 << "-";
	//	ISOTime.width(2);
	//	ISOTime << UTC->tm_mon+1 << "-";
	//	ISOTime.width(2);
	//	ISOTime << UTC->tm_mday << " ";
	//	ISOTime.width(2);
	//	ISOTime << UTC->tm_hour << ":";
	//	ISOTime.width(2);
	//	ISOTime << UTC->tm_min << ":";
	//	ISOTime.width(2);
	//	ISOTime << UTC->tm_sec;
	//}
	XMLItem << "</Item>\n";
	return(XMLItem.str());
}
int GetTivoNowPlaying(SOCKET DataSocket)
{
	TRACE(__FUNCTION__ "\n");
	int rval = 0;
	char MyHostName[255] = {0}; // winsock hostname used for data recordkeeping
	gethostname(MyHostName,sizeof(MyHostName));
	char XMLDataBuff[1024*11];
	strcpy(XMLDataBuff,"<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n");
	strcat(XMLDataBuff,"<TiVoContainer>\n");
	strcat(XMLDataBuff,"  <ItemStart>0</ItemStart>\n");
	strcat(XMLDataBuff,"  <ItemCount>2</ItemCount>\n");
	strcat(XMLDataBuff,"  <Details>\n");
	strcat(XMLDataBuff,"    <Title>"); strcat(XMLDataBuff, MyHostName); strcat(XMLDataBuff," (WimTiVoServer)</Title>\n");
	strcat(XMLDataBuff,"    <ContentType>x-tivo-container/folder</ContentType>\n");
	strcat(XMLDataBuff,"    <SourceFormat>x-tivo-container/folder</SourceFormat>\n");
	strcat(XMLDataBuff,"    <TotalItems>2</TotalItems>\n");
	strcat(XMLDataBuff,"  </Details>\n");

	strcat(XMLDataBuff,"  <Item>\n");
	strcat(XMLDataBuff,"    <Details>\n");
	strcat(XMLDataBuff,"      <Title>Family.Guy.S11E03.HDTV.x264-LOL</Title>\n");
	strcat(XMLDataBuff,"      <ContentType>video/x-tivo-mpeg</ContentType>\n");
	strcat(XMLDataBuff,"      <SourceFormat>video/mp4</SourceFormat>\n");
	strcat(XMLDataBuff,"      <CopyProtected>No</CopyProtected>\n");
	strcat(XMLDataBuff,"      <Duration>1298171</Duration>\n");
	strcat(XMLDataBuff,"      <SourceSize>1285727232</SourceSize>\n");
	strcat(XMLDataBuff,"      <ContentSize>1285727232</ContentSize>\n");
	strcat(XMLDataBuff,"      <HighDefinition>Yes</HighDefinition>\n");
	strcat(XMLDataBuff,"      <CaptureDate>0x50976384</CaptureDate>\n");
	strcat(XMLDataBuff,"    </Details>\n");
	strcat(XMLDataBuff,"    <Links>\n");
	strcat(XMLDataBuff,"      <Content>\n");
	strcat(XMLDataBuff,"        <ContentType>video/x-tivo-mpeg</ContentType>\n");
	strcat(XMLDataBuff,"        <Url>/TiVoConnect/TivoNowPlaying/Family.Guy.S11E03.HDTV.x264-LOL.mp4</Url>\n");
//	strcat(XMLDataBuff,"        <Url>http://192.168.0.5:8080/TiVoConnect/TivoNowPlaying/IFamily.Guy.S11E03.HDTV.x264-LOL.mp4</Url>\n");
	strcat(XMLDataBuff,"      </Content>\n");
	strcat(XMLDataBuff,"      <CustomIcon>\n");
	strcat(XMLDataBuff,"        <ContentType>image/x-tivo-urn</ContentType>\n");
	strcat(XMLDataBuff,"        <AcceptsParams>No</AcceptsParams>\n");
	strcat(XMLDataBuff,"        <Url>urn:tivo:image:save-until-i-delete-recording</Url>\n");
	strcat(XMLDataBuff,"      </CustomIcon>\n");
	strcat(XMLDataBuff,"    </Links>\n");
	strcat(XMLDataBuff,"  </Item>\n");

	strcat(XMLDataBuff,"  <Item>\n");
	strcat(XMLDataBuff,"    <Details>\n");
	strcat(XMLDataBuff,"      <Title>NBC Nightly News</Title>\n");
	strcat(XMLDataBuff,"      <ContentType>video/x-tivo-mpeg</ContentType>\n");
	strcat(XMLDataBuff,"      <SourceFormat>video/x-tivo-mpeg</SourceFormat>\n");
	strcat(XMLDataBuff,"      <CaptureDate>0x4E2E1190</CaptureDate>\n");
	strcat(XMLDataBuff,"      <Description>The latest world and national news. Copyright Tribune Media Services, Inc.</Description>\n");
	strcat(XMLDataBuff,"      <Duration>1798000</Duration>\n");
	strcat(XMLDataBuff,"      <SourceSize>2805738488</SourceSize>\n");
	strcat(XMLDataBuff,"      <ContentSize>2805738488</ContentSize>\n");
	strcat(XMLDataBuff,"      <HighDefinition>Yes</HighDefinition>\n");
	strcat(XMLDataBuff,"    </Details>\n");
	strcat(XMLDataBuff,"    <Links>\n");
	strcat(XMLDataBuff,"      <Content>\n");
	strcat(XMLDataBuff,"        <ContentType>video/x-tivo-mpeg</ContentType>\n");
	strcat(XMLDataBuff,"        <Url>/TiVoConnect/TivoNowPlaying/NBC%20Nightly%20News%20(Recorded%20Jul%2025,%202011,%20KINGDT).TiVo</Url>\n");
	strcat(XMLDataBuff,"      </Content>\n");
	strcat(XMLDataBuff,"      <CustomIcon>\n");
	strcat(XMLDataBuff,"        <ContentType>image/x-tivo-urn</ContentType>\n");
	strcat(XMLDataBuff,"        <AcceptsParams>No</AcceptsParams>\n");
	strcat(XMLDataBuff,"        <Url>urn:tivo:image:save-until-i-delete-recording</Url>\n");
	strcat(XMLDataBuff,"      </CustomIcon>\n");
	strcat(XMLDataBuff,"      <TiVoVideoDetails>\n");
	strcat(XMLDataBuff,"        <ContentType>text/xml</ContentType>\n");
	strcat(XMLDataBuff,"        <AcceptsParams>No</AcceptsParams>\n");
	strcat(XMLDataBuff,"        <Url>/TiVoConnect/TivoNowPlaying/INBC%20Nightly%20News%20(Recorded%20Jul%2025,%202011,%20KINGDT).TiVo?Format=text%2Fxml</Url>\n");
	strcat(XMLDataBuff,"      </TiVoVideoDetails>\n");
	strcat(XMLDataBuff,"    </Links>\n");
	strcat(XMLDataBuff,"  </Item>\n");

	strcat(XMLDataBuff,"</TiVoContainer>\n");

	/* Create HTTP Header */
	char tmpBuff[1024];
	char HttpResponse[1024];
	time_t timer;
	/* Get Current Time */
	time(&timer);
	strcpy(HttpResponse,"HTTP/1.1 200 OK\r\n");
	strcat(HttpResponse,"Content-Type: text/html\r\n");
	strcat(HttpResponse,"Connection: close\r\n");
	sprintf(tmpBuff,"Date: %s",asctime(gmtime(&timer)));
	tmpBuff[30] = '\0';
	strcat(tmpBuff," GMT\r\n");
	strcat(HttpResponse,tmpBuff);
	sprintf(tmpBuff,"Content-Length: %d\r\n",strlen(XMLDataBuff));
	strcat(HttpResponse,tmpBuff);
	strcat(HttpResponse,"\r\n");

	int nRet;
	nRet = send(DataSocket, HttpResponse, strlen(HttpResponse),0);
	nRet = send(DataSocket, XMLDataBuff, strlen(XMLDataBuff),0);
	return(0);
}
int GetFile(SOCKET DataSocket)
{
	TRACE(__FUNCTION__ "\n");
	int rval = 0;

	/* Create HTTP Header */
	char tmpBuff[1024];
	char HttpResponse[1024];
	time_t timer;
	/* Get Current Time */
	time(&timer);
	strcpy(HttpResponse,"HTTP/1.1 200 OK\r\n");
	strcat(HttpResponse,"Content-Type: video/x-tivo-mpeg\r\n");
	strcat(HttpResponse,"Connection: close\r\n");
	sprintf(tmpBuff,"Date: %s",asctime(gmtime(&timer)));
	tmpBuff[30] = '\0';
	strcat(tmpBuff," GMT\r\n");
	strcat(HttpResponse,tmpBuff);
	sprintf(tmpBuff,"Content-Length: 2805738488\r\n");
	strcat(HttpResponse,tmpBuff);
	strcat(HttpResponse,"\r\n");

	int nRet;
	nRet = send(DataSocket, HttpResponse, strlen(HttpResponse),0);
	std::ifstream FileToTransfer;
//	FileToTransfer.open("//Acid/TiVo/Family.Guy.S11E03.HDTV.x264-LOL.mp4", ios_base::in | ios_base::binary);
	FileToTransfer.open("//Acid/TiVo/Evening Magazine (Recorded Mar 26, 2010, KINGDT).TiVo", ios_base::in | ios_base::binary);
	if (FileToTransfer.good())
	{
		cout << " Sending File " << endl;
		cout << HttpResponse << endl;
		char XMLDataBuff[1024];
		while (!FileToTransfer.eof())
		{
			FileToTransfer.read(XMLDataBuff,sizeof(XMLDataBuff));
			nRet = send(DataSocket, XMLDataBuff, FileToTransfer.gcount(), 0);
			//nRet = send(DataSocket, XMLDataBuff, strlen(XMLDataBuff),0);
		}
	}
	return(0);
}
int GetSlash(SOCKET DataSocket)
{
	TRACE(__FUNCTION__ "\n");
	int rval = -1;
	char tmpBuff[1024];
	char XMLDataBuff[1024*11];
	int  XMLDataSize = 0;
	int nRet;
	int index = 0;
	time_t timer;
	struct tm *newtime = NULL;
	char HttpResponse[1024];
	/* Get Current Time */
	time(&timer);

	/* Create XML Data */
	strcpy(XMLDataBuff,"<html>\r\n");
	strcat(XMLDataBuff,"<head><title>RFSAW Reader</title></head>\r\n");
	strcat(XMLDataBuff,"<body ms_positioning=\"GridLayout\">\r\n");
	strcat(XMLDataBuff,"<IMG style=\"Z-INDEX: 101; LEFT: 337px; POSITION: absolute; TOP: 16px\" src=\"http://www.rfsaw.com/imgs/rfsaw_home_logo.gif\">\r\n");
	strcat(XMLDataBuff,"<TEXTAREA id=\"Textarea1\" style=\"Z-INDEX: 102; LEFT: 337px; WIDTH: 319px; POSITION: absolute; TOP: 112px; HEIGHT: 82px\" name=\"Textarea1\" rows=\"5\" cols=\"37\">");
	strcat(XMLDataBuff,"RFSAW EPC Reader\r\nVersion 1.0\r\nCopyright 2003-2004\r\n");
	strcat(XMLDataBuff,"</TEXTAREA>\r\n");
	strcat(XMLDataBuff,"<p><a href=\"/Setup/Net/\">Setup Network</a></p>\r\n");
	strcat(XMLDataBuff,"<p><a href=\"/GroomedData/\">Get Groomed Data</a></p>\r\n");
	strcat(XMLDataBuff,"<p><a href=\"/Calibrate/\">Calibrate</a></p>\r\n");

	strcat(XMLDataBuff,"<form action=\"/GroomedPoster/\" method=\"post\">\r\n");
	sockaddr_in HostName;
	int HostNameSize = sizeof(HostName);
	if (0 == getpeername(DataSocket, (sockaddr *)&HostName, &HostNameSize))
	{
		strcat(XMLDataBuff,"Host<INPUT type=\"text\" name=\"host\" value=\"");
		strcat(XMLDataBuff,inet_ntoa(HostName.sin_addr));
		strcat(XMLDataBuff,"\"><br>\r\n");
	}
	else
	{
		strcat(XMLDataBuff,"Host<INPUT type=\"text\" name=\"host\" value=\"datacolector.rfsaw.com\"><br>\r\n");
	}
	strcat(XMLDataBuff,"Port<INPUT type=\"text\" name=\"port\" value=\"80\"><br>\r\n");
	strcat(XMLDataBuff,"<INPUT type=\"submit\" value=\"Start Data Poster\">\r\n");
	strcat(XMLDataBuff,"</form>\r\n");

	strcat(XMLDataBuff,"</body>\r\n");
	strcat(XMLDataBuff,"</html>\r\n");

	XMLDataSize = strlen(XMLDataBuff);

	/* Create HTTP Header */
	strcpy(HttpResponse,"HTTP/1.1 200 OK\r\n");
	strcat(HttpResponse,"Content-Type: text/html\r\n");
	strcat(HttpResponse,"Connection: close\r\n");
	sprintf(tmpBuff,"Date: %s",asctime(gmtime(&timer)));
	tmpBuff[30] = '\0';
	strcat(tmpBuff," GMT\r\n");
	strcat(HttpResponse,tmpBuff);
	sprintf(tmpBuff,"Content-Length: %d\r\n",XMLDataSize);
	strcat(HttpResponse,tmpBuff);
	strcat(HttpResponse,"\r\n");

	nRet = send(DataSocket,HttpResponse,strlen(HttpResponse),0);
	nRet = send(DataSocket,XMLDataBuff,strlen(XMLDataBuff),0);
	return(0);
}
//int GetNetSetup(SOCKET DataSocket)
//{
//	int rval = -1;
//	char tmpBuff[1024];
//	char XMLDataBuff[1024*11];
//	int  XMLDataSize = 0;
//	int nRet;
//	int index = 0;
//	time_t timer;
//	struct tm *newtime = NULL;
//	char HttpResponse[1024];
//	/* Get Current Time */
//	time(&timer);
//	/* Create XML Data */
//	strcpy(XMLDataBuff,"<html>\r\n");
//	strcat(XMLDataBuff,"<head><title>RFSAW Reader</title></head>\r\n");
//	strcat(XMLDataBuff,"<body ms_positioning=\"GridLayout\">\r\n");
//	strcat(XMLDataBuff,"<IMG style=\"Z-INDEX: 101; LEFT: 337px; POSITION: absolute; TOP: 16px\" src=\"http://www.rfsaw.com/imgs/rfsaw_home_logo.gif\">\r\n");
//	strcat(XMLDataBuff,"<P>\n");
//	strcat(XMLDataBuff,"<INPUT id=\"Radio1\" type=\"radio\" value=\"Radio1\" name=\"RadioGroup\" CHECKED>Set up Network Automatically Using DHCP<BR>\n");
//	strcat(XMLDataBuff,"<INPUT id=\"Radio2\" type=\"radio\" value=\"Radio2\" name=\"RadioGroup\">Set up Network Manually\n");
//	strcat(XMLDataBuff,"</P>\n<P>");
//	if (0 == gethostname(tmpBuff,sizeof(tmpBuff)))
//	{
//		strcat(XMLDataBuff,"<INPUT id=\"IPADDR\" type=\"text\" name=\"IPADDR\" VALUE=\"");
//		strcat(XMLDataBuff,tmpBuff);
//		strcat(XMLDataBuff,"\">IP Address<BR>\n");
//		strcat(XMLDataBuff,"<INPUT id=\"IPMASK\" type=\"text\" name=\"IPMASK\" VALUE=\"");
//		strcat(XMLDataBuff,tmpBuff);
//		strcat(XMLDataBuff,"\">IP Netmask<BR>\n");
//		//FIXED_INFO mf;
//		//if (ERROR_SUCCESS == GetNetworkParams(&mf,sizeof(mf)))
//		MIB_IPADDRTABLE IpAddrTable;
//		ULONG dwSize = sizeof(IpAddrTable);
//		if (NO_ERROR == GetIpAddrTable(&IpAddrTable,&dwSize,TRUE))
//		{
//		}
//		DWORD ifcount;
//		GetNumberOfInterfaces(&ifcount);
//		while (ifcount>0)
//		{
//			MIB_IFROW ifrowmib;
//			ifrowmib.dwIndex = ifcount--;
//			if (NO_ERROR == GetIfEntry(&ifrowmib))
//			{
//				if ((MIB_IF_TYPE_ETHERNET == ifrowmib.dwType) &&
//					(MIB_IF_OPER_STATUS_OPERATIONAL == ifrowmib.dwOperStatus))
//				{
//					strcat(XMLDataBuff,"");
//				}
//			}
//		}
//		//MIB_IPNETTABLE nettable;
//		//DWORD dwSize = sizeof(nettable);
//		//if (NO_ERROR == GetIpNetTable(&nettable,&dwSize,TRUE))
//		//{
//		//}
//	}
//	else
//	{
//		strcat(XMLDataBuff,"<INPUT id=\"IPADDR\" type=\"text\" name=\"IPADDR\" VALUE=\"127.0.0.1\">IP Address<BR>\n");
//		strcat(XMLDataBuff,"<INPUT id=\"IPMASK\" type=\"text\" name=\"IPMASK\" VALUE=\"255.255.255.0\">IP Netmask<BR>\n");
//	}
//	strcat(XMLDataBuff,"<INPUT id=\"IPGATE\" type=\"text\" name=\"IPGATE\">Default Gateway\n");
//	strcat(XMLDataBuff,"</P>\n");
//	strcat(XMLDataBuff,"<P><INPUT id=\"Checkbox1\" type=\"checkbox\" name=\"Checkbox1\">SNMP Enabled<BR>\n");
//	strcat(XMLDataBuff,"<INPUT id=\"Checkbox2\" type=\"checkbox\" name=\"Checkbox2\" CHECKED>Web Server Enabled\n");
//	strcat(XMLDataBuff,"</P>\n");
//	strcat(XMLDataBuff,"<P><INPUT id=\"Submit1\" type=\"submit\" value=\"Submit\" name=\"Submit1\"></P>\n");
//	strcat(XMLDataBuff,"</body>\n");
//	XMLDataSize = strlen(XMLDataBuff);
//
//	/* Create HTTP Header */
//	strcpy(HttpResponse,"HTTP/1.1 200 OK\r\n");
//	strcat(HttpResponse,"Content-Type: text/html\r\n");
//	strcat(HttpResponse,"Connection: close\r\n");
//	sprintf(tmpBuff,"Date: %s",asctime(gmtime(&timer)));
//	tmpBuff[30] = '\0';
//	strcat(tmpBuff," GMT\r\n");
//	strcat(HttpResponse,tmpBuff);
//	sprintf(tmpBuff,"Content-Length: %d\r\n",XMLDataSize);
//	strcat(HttpResponse,tmpBuff);
//	strcat(HttpResponse,"\r\n");
//
//	nRet = send(DataSocket,HttpResponse,strlen(HttpResponse),0);
//	nRet = send(DataSocket,XMLDataBuff,strlen(XMLDataBuff),0);
//	return(0);
//}
UINT HTTPMain(LPVOID lvp)
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
//			saServer.sin_port = htons(2190);		/* Use port from command line */
			saServer.sin_port = htons(0);			/* Use unique port */
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
						if (remoteSocket != INVALID_SOCKET)
						{
							char InBuff[1024];
							int count = recv(remoteSocket,InBuff,sizeof(InBuff),0);
							InBuff[count] = '\0';
							//if (strncmp(InBuff,"GET /GroomedData/",17) == 0)
							//{
							//	GetGroomedData(remoteSocket);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							//else if (strncmp(InBuff,"POST /GroomedPoster/",20) == 0)
							//{
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//	CString * csRequest = new CString(InBuff,count);
							//	AfxBeginThread(PostGroomedDataThread,csRequest);
							//	GetSlash(remoteSocket);
							//	//CString csResponse("HTTP/1.1 200 OK\r\nConnection: close\r\n");
							//	//csResponse.AppendFormat("\r\n");
							//	//send(remoteSocket,LPCSTR(csResponse),csResponse.GetLength(),0);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							//else if (strncmp(InBuff,"GET /Calibrate/",15) == 0)
							//{
							//	GetCalibrate(remoteSocket);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							//else if (strncmp(InBuff,"GET /Setup/Net/",15) == 0)
							//{
							//	GetNetSetup(remoteSocket);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							//else 
							if (strncmp(InBuff,"GET /TiVoConnect?Command=QueryContainer&Container=%2FTivoNowPlaying",67) == 0)
							{
								GetTivoNowPlaying(remoteSocket);
								if(bConsoleExists)
									cout << "GetTivoNowPlaying\t" << InBuff;
							}
							else if (strncmp(InBuff,"GET /TiVoConnect?Command=QueryContainer&Container=%2F",53) == 0)
							{
								GetTiVoConnect(remoteSocket);
								if(bConsoleExists)
									cout << "GetTiVoConnect\t" << InBuff;
							}
							else if (strncmp(InBuff,"GET /TiVoConnect/TivoNowPlaying/",32) == 0)
							{
								GetFile(remoteSocket);
								if(bConsoleExists)
									cout << "GetFile\t" << InBuff;
							}
							else if (strncmp(InBuff,"GET /TiVoConnect?Command=QueryFormats&SourceFormat=video%2Fx-tivo-mpeg",70) == 0)
							{
								GetTiVoQueryFormats(remoteSocket);
								if(bConsoleExists)
									cout << "GetTiVoQueryFormats\t" << InBuff;
							}						
							//else if (strncmp(InBuff,"GET /",5) == 0)
							//{
							//	GetSlash(remoteSocket);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							else
							{
								int nRet = send(remoteSocket,"HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n",41,0);
								if(bConsoleExists)
									cout << "HTTP/1.1 404 Not Found\t" << InBuff;
							}
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
							//if (strncmp(InBuff,"GET /GroomedData/",17) == 0)
							//{
							//	GetGroomedData(remoteSocket);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							//else if (strncmp(InBuff,"POST /GroomedPoster/",20) == 0)
							//{
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//	CString * csRequest = new CString(InBuff,count);
							//	AfxBeginThread(PostGroomedDataThread,csRequest);
							//	GetSlash(remoteSocket);
							//	//CString csResponse("HTTP/1.1 200 OK\r\nConnection: close\r\n");
							//	//csResponse.AppendFormat("\r\n");
							//	//send(remoteSocket,LPCSTR(csResponse),csResponse.GetLength(),0);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							//else if (strncmp(InBuff,"GET /Calibrate/",15) == 0)
							//{
							//	GetCalibrate(remoteSocket);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							//else if (strncmp(InBuff,"GET /Setup/Net/",15) == 0)
							//{
							//	GetNetSetup(remoteSocket);
							//	if(bConsoleExists)
							//		printf("%s",InBuff);
							//}
							//else 
							if (strncmp(InBuff,"GET /",5) == 0)
							{
								GetSlash(remoteSocket);
								if(bConsoleExists)
									printf("%s",InBuff);
							}
							else
							{
								int nRet = send(remoteSocket,"HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n",41,0);
								if(bConsoleExists)
									printf("%s",InBuff);
							}
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
				cout << "[" << getTimeISO8601() << "] " << inet_ntoa(saServer.sin_addr) << " " << csServerBroadcast.GetString() << endl;
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
			if (S_OK == CoCreateGuid(&MyProgramGuid))
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
			std::cout << "[" << getTimeISO8601() << "] Use key combination Ctrl-C to end the program." << endl;

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

			if (S_OK == CoInitializeEx(0, COINIT_MULTITHREADED)) // COINIT_APARTMENTTHREADED
			{
				XML_Test_Read_ElementsOnly();
				XML_Test_Read();
				XML_Test_Write();
				XML_Test_Write_InMemory();

				std::vector<CString> myURLS;
				myURLS.push_back(CString(_T("http://192.168.0.108/TiVoConnect?Command=QueryContainer&Container=/")));
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying")));
				myURLS.push_back(CString(_T("http://192.168.0.5:8080/TiVoConnect?Command=QueryContainer&Container=/")));
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.5:4430/TiVoConnect?Command=QueryContainer&Container=/TivoNowPlaying")));
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
								if(dwRet == HTTP_STATUS_OK)
								{
									std::stringstream OutPutFileName;
									OutPutFileName << "WimTivoServer." << FileIndex++ << ".xml";
									std::ofstream OutputFile(OutPutFileName.str());
									std::cout << "[" << getTimeISO8601() << "] file contents: **--**" << endl;
									char ittybittybuffer;
									while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
									{
										std::cout << ittybittybuffer;
										if (OutputFile.is_open())
											OutputFile << ittybittybuffer;
									}
									std::cout << "**--**" << endl;
									OutputFile.close();
								}
								serverFile->Close();
							}
							catch(CInternetException *e)
							{
								TCHAR   szCause[255];
								e->GetErrorMessage(szCause,sizeof(szCause)/sizeof(TCHAR));
								CStringA csErrorMessage(szCause);
								csErrorMessage.Trim();
								std::cout << "[" << getTimeISO8601() << "] InternetException: " <<  csErrorMessage.GetString() << " (" << e->m_dwError << ") " << endl;
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
				cout << "[" << getTimeISO8601() << "] Listening for TiVo Broadcast packets on UDP port 2190" << endl;
				// Set up CTR-C signal handler
				typedef void (*SignalHandlerPointer)(int);
				SignalHandlerPointer previousHandler = signal(SIGINT, SignalHandler);
				int loopCounter = 0;
				bRun = true;
				SOCKADDR_IN saServer;
				while (bRun && (TiVoBeaconListen(saServer) == true))
					cout << "[" << getTimeISO8601() << "] \r";
				// remove our special Ctrl-C signal handler and restore previous one
				signal(SIGINT, previousHandler);
				cout << "\n[" << getTimeISO8601() << "] No longer listening for TiVo Broadcast packets on UDP port 2190" << endl;
			}
			terminateEvent = CreateEvent(0,TRUE,FALSE,0);
			if (terminateEvent != NULL) 
			{
				threadHandle = AfxBeginThread(HTTPMain, NULL, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
				if (threadHandle != NULL)
				{
					threadHandle->m_bAutoDelete = false;
					threadHandle->ResumeThread();

					DWORD dwVersion = GetVersion();
					int dwMajorVersion = (int)(LOBYTE(LOWORD(dwVersion)));
					int dwMinorVersion = (int)(HIBYTE(LOWORD(dwVersion)));
					int dwBuild = 0;
					if (dwVersion < 0x80000000)              
						dwBuild = (int)(HIWORD(dwVersion));

					CStringA csTiVoPacket("tivoconnect=1\n");
					csTiVoPacket.AppendFormat("method=%s\n",true?"broadcast":"connect");
					csTiVoPacket.AppendFormat("platform=pc/WinNT:%d.%d.%d\n", dwMajorVersion, dwMinorVersion, dwBuild);
					csTiVoPacket.Append("machine="); csTiVoPacket.Append(CStringA(csMyHostName).GetString());csTiVoPacket.Append("\n");
					csTiVoPacket.Append("identity="); csTiVoPacket.Append(CStringA(csMyProgramGuid).GetString());csTiVoPacket.Append("\n");
					struct sockaddr addr;
					socklen_t addr_len = sizeof(addr);
					getsockname(ControlSocket, &addr, &addr_len);
					if (addr.sa_family == AF_INET)
					{
						struct sockaddr_in * saServer = (sockaddr_in *)&addr;
						csTiVoPacket.AppendFormat("services=TiVoMediaServer:%hu/http\n",ntohs(saServer->sin_port));
					}
					//csTiVoPacket.AppendFormat(_T("services=<name>[:<port>][/<protocol>], ...\r\n"));
					csTiVoPacket.Append("swversion=");csTiVoPacket.Append(CStringA(csBuildDateTime));csTiVoPacket.Append("\n");
					cout << csTiVoPacket.GetString();
					for (auto index = 300; index > 0; --index)
					{
						CStringA csTiVoPacket("tivoconnect=1\n");
						csTiVoPacket.AppendFormat("method=%s\n",true?"broadcast":"connect");
						csTiVoPacket.AppendFormat("platform=pc/WinNT:%d.%d.%d\n", dwMajorVersion, dwMinorVersion, dwBuild);
						csTiVoPacket.Append("machine="); csTiVoPacket.Append(CStringA(csMyHostName).GetString());csTiVoPacket.Append("\n");
						csTiVoPacket.Append("identity="); csTiVoPacket.Append(CStringA(csMyProgramGuid).GetString());csTiVoPacket.Append("\n");
						struct sockaddr addr;
						addr.sa_family = AF_UNSPEC;
						socklen_t addr_len = sizeof(addr);
						if (ControlSocket != INVALID_SOCKET)
							getsockname(ControlSocket, &addr, &addr_len);
						if (addr.sa_family == AF_INET)
						{
							struct sockaddr_in * saServer = (sockaddr_in *)&addr;
							csTiVoPacket.AppendFormat("services=TiVoMediaServer:%hu/http\n",ntohs(saServer->sin_port));
						}
						//csTiVoPacket.AppendFormat(_T("services=<name>[:<port>][/<protocol>], ...\r\n"));
						csTiVoPacket.Append("swversion="); csTiVoPacket.Append(CStringA(csBuildDateTime));csTiVoPacket.Append("\n");
						TiVoBeaconSend(csTiVoPacket);
						csTiVoPacket.Replace("\n", " ");
						csTiVoPacket.Trim();
						cout << "[" << getTimeISO8601() << "] (" << index << ") " << csTiVoPacket.GetString() << endl;
						Sleep(1000);
					}
					closesocket(ControlSocket);
					ControlSocket = INVALID_SOCKET;

					WaitForSingleObject(terminateEvent,INFINITE);

					if (terminateEvent)
						CloseHandle(terminateEvent);
					if (threadHandle)
					{
						delete threadHandle;
						threadHandle = NULL;
					}
				}
			}
		}
	}
	return nRetCode;
}
