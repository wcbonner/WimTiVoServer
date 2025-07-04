// WimTivoBeaconListener.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WimTivoBeaconListener.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;
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
		delete[] pVersionInfo;
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
class cTiVoServer
{
	//192.168.1.11 tivoconnect=1 swversion=11.0k-01-2-652 method=broadcast identity=6520201806EEAAE machine=Living Room platform=tcd/Series3 services=TiVoMediaServer:80/http
public:
	std::string m_address;
	std::string m_swversion;
	std::string m_method;
	std::string m_identity;
	std::string m_machine;
	std::string m_platform;
	std::string m_services;
	std::string m_MAK;
	bool operator==(const cTiVoServer & other) const;
	std::string WriteTXT(const char seperator = '\t') const;
	bool ReadTXT(const std::string & text, const char seperator = '\t');
};
bool cTiVoServer::operator==(const cTiVoServer & other) const
{
	return(
		//(m_address == other.m_address) && 
		//(m_swversion == other.m_swversion) && 
		//(m_method == other.m_method) && 
		m_identity == other.m_identity
		//(m_machine == other.m_machine) && 
		//(m_platform == other.m_platform) && 
		//(m_services == other.m_services)
		//m_machine == other.m_machine
		);
}
std::string cTiVoServer::WriteTXT(const char seperator) const
{
	std::stringstream ssValue;
	ssValue << "machine=" << m_machine;
	ssValue << seperator << "address=" << m_address;
	if (!m_identity.empty()) ssValue << seperator << "identity=" << m_identity;
	if (!m_method.empty()) ssValue << seperator << "method=" << m_method;
	if (!m_platform.empty()) ssValue << seperator << "platform=" << m_platform;
	if (!m_services.empty()) ssValue << seperator << "services=" << m_services;
	if (!m_swversion.empty()) ssValue << seperator << "swversion=" << m_swversion;
	ssValue << seperator << "tivoconnect=1";
	if (!m_MAK.empty()) ssValue << seperator << "MAK=" << m_MAK;
	return(ssValue.str());
}
bool cTiVoServer::ReadTXT(const std::string & text, const char seperator)
{
	bool rval = true;
	CString csText(text.c_str());
	int SectPos = 0;
	CString csSect(csText.Tokenize(CString(seperator), SectPos));
	while (csSect != _T(""))
	{
		int KeyPos = 0;
		CString csKey(csSect.Tokenize(_T("="), KeyPos));
		CString csValue(csSect.Tokenize(_T("="), KeyPos));
		if (!csKey.CompareNoCase(_T("address")))
			m_address = CStringA(csValue);
		else if (!csKey.CompareNoCase(_T("identity")))
			m_identity = CStringA(csValue);
		else if (!csKey.CompareNoCase(_T("machine")))
			m_machine = CStringA(csValue);
		else if (!csKey.CompareNoCase(_T("method")))
			m_method = CStringA(csValue);
		else if (!csKey.CompareNoCase(_T("platform")))
			m_platform = CStringA(csValue);
		else if (!csKey.CompareNoCase(_T("services")))
			m_services = CStringA(csValue);
		else if (!csKey.CompareNoCase(_T("swversion")))
			m_swversion = CStringA(csValue);
		else if (!csKey.CompareNoCase(_T("MAK")))
			m_MAK = CStringA(csValue);
		csSect = csText.Tokenize(CString(seperator), SectPos);
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
// Attempt to browse mDNS for tivo reports via Bonjour (2023-12-20)
#ifdef AppleExample1
// this code came from: https://flylib.com/books/en/2.94.1.54/1/
//#include <dns_sd.h> 
#include <stdio.h>     // For stdout, stderr 
#include <string.h>     // For strlen(), strcpy(  ), bzero(  ) 
extern void HandleEvents(DNSServiceRef); 
static void MyBrowseCallBack(DNSServiceRef service,
	DNSServiceFlags flags,                 
	uint32_t interfaceIndex,                 
	DNSServiceErrorType errorCode,                 
	const char * name,                 
	const char * type,                 
	const char * domain,                 
	void * context)     
{     
#pragma unused(context)     
	if (errorCode != kDNSServiceErr_NoError)         
		fprintf(stderr, "MyBrowseCallBack returned %d\n", errorCode);     
	else         
	{         
		char *addString  = (flags & kDNSServiceFlagsAdd) ? "ADD" : "REMOVE";
		char *moreString = (flags & kDNSServiceFlagsMoreComing) ? "MORE" : "    ";
		printf("%-7s%-5s %d%s.%s%s\n",             addString, moreString, interfaceIndex, name, type, domain);
	}
	if (!(flags & kDNSServiceFlagsMoreComing)) fflush(stdout);
} 
static DNSServiceErrorType MyDNSServiceBrowse(  )
{     
	DNSServiceErrorType error;
	DNSServiceRef  serviceRef;
	error = DNSServiceBrowse(&serviceRef,
		0,                // no flags
		0,                // all network interfaces
		"_http._tcp",     // service type
		"",               // default domains
		MyBrowseCallBack, // call back function
		NULL);            // no context
	if (error == kDNSServiceErr_NoError)
	{
		HandleEvents(serviceRef); // Add service to runloop to get callbacks
		DNSServiceRefDeallocate(serviceRef);
	}
	return error;
}
int main (int argc, const char * argv[])
{
	DNSServiceErrorType error = MyDNSServiceBrowse(  );
	if (error)
	fprintf(stderr, "DNSServiceDiscovery returned %d\n", error);
	return 0;
} 
#endif // AppleExample1
#ifdef AooleExample2
#include <dns_sd.h> 
#include <stdio.h>            // For stdout, stderr 
#include <string.h>            // For strlen(  ), strcpy(  ), bzero(  ) 
extern void HandleEvents(DNSServiceRef); 
static void MyRegisterCallBack(
	DNSServiceRef service,                    
	DNSServiceFlags flags,                    
	DNSServiceErrorType errorCode,                    
	const char * name,                    
	const char * type,                    
	const char * domain,                    
	void * context)
{
#pragma unused(flags)
#pragma unused(context)
	if (errorCode != kDNSServiceErr_NoError)
		fprintf(stderr, "MyRegisterCallBack returned %d\n", errorCode);
	else
		printf("%-15s %s.%s%s\n","REGISTER", name, type, domain);
}
static DNSServiceErrorType MyDNSServiceRegister(  )
{
	DNSServiceErrorType error;
	DNSServiceRef serviceRef;
	error = DNSServiceRegister(
		&serviceRef,
		0,                  // no flags
		0,                  // all network interfaces
		"Not a real page",  // name
		"_http._tcp",       // service type
		"",                 // register in default domain(s)
		NULL,               // use default host name
		htons(9092),        // port number
		0,                  // length of TXT record
		NULL,               // no TXT record
		MyRegisterCallBack, // call back function
		NULL);              // no context
	if (error == kDNSServiceErr_NoError)
	{
		HandleEvents(serviceRef);
		DNSServiceRefDeallocate(serviceRef);
	}
	return error;
}
int main (int argc, const char * argv[])
{
	DNSServiceErrorType error = MyDNSServiceRegister(  );
	fprintf(stderr, "DNSServiceDiscovery returned %d\n", error);
	return 0;
} 
#endif // AooleExample2
// https://stackoverflow.com/questions/66474722/use-multicast-dns-when-network-cable-is-unplugged
// https://git.walbeck.it/archive/mumble-voip_mumble/src/branch/master/src/mumble/Zeroconf.cpp
DNS_SERVICE_CANCEL cancelBrowse{ 0 };	// A pointer to a DNS_SERVICE_CANCEL structure that can be used to cancel a pending asynchronous browsing operation. This handle must remain valid until the query is canceled.
VOID WINAPI Browse_mDNS_Callback(DWORD Status, PVOID pQueryContext, PDNS_RECORD pDnsRecord)
{
	if (Status != ERROR_SUCCESS)
	{
		if (pDnsRecord)
			DnsRecordListFree(pDnsRecord, DnsFreeRecordList);
		if (Status == ERROR_CANCELLED)
			std::wcout << "[                   ] DnsServiceBrowse() reports status code ERROR_CANCELLED" << std::endl;
		else
			std::wcout << "[                   ] DnsServiceBrowse() reports status code " << Status << ", ignoring results" << std::endl;
	}
	if (!pDnsRecord)
		return;
	std::cout << "[" << getTimeISO8601() << "] Browse_mDNS_Callback" << std::endl;
	for (auto cur = pDnsRecord; cur; cur = cur->pNext)
	{
		std::wcout << "[                   ]                     pName: " << std::wstring(cur->pName) << std::endl;
		switch (cur->wType)
		{
		case DNS_TYPE_PTR:
			std::wcout << "[                   ]        Data.PTR.pNameHost: " << std::wstring(cur->Data.PTR.pNameHost) << std::endl;
			break;
		case DNS_TYPE_A:	//  RFC 1034/1035
			{
			wchar_t StringBuf[17];
			std::wcout << "[                   ]          Data.A.IpAddress: " << InetNtopW(AF_INET, &(cur->Data.A.IpAddress), StringBuf, sizeof(StringBuf)) << std::endl;
			}
			break;
		case DNS_TYPE_AAAA:
		{
			wchar_t StringBuf[47];
			std::wcout << "[                   ]          Data.A.IpAddress: " << InetNtopW(AF_INET6, &(cur->Data.A.IpAddress), StringBuf, sizeof(StringBuf)) << std::endl;
		}
		break;
		case DNS_TYPE_TEXT:	//  RFC 1034/1035
			{
			auto index = cur->Data.TXT.dwStringCount;
			while (index > 0)
			{
				index--;
				std::wcout << "[                   ]  Data.TXT.pStringArray[" << index <<"]: " << std::wstring(cur->Data.TXT.pStringArray[index]) << std::endl;
			}
			}
			break;
		case DNS_TYPE_SRV:	//  RFC 2052    (Service location)
			std::wcout << "[                   ]      Data.SRV.pNameTarget: " << std::wstring(cur->Data.SRV.pNameTarget) << std::endl;
			std::wcout << "[                   ]            Data.SRV.wPort: " << cur->Data.SRV.wPort << std::endl;
			break;
		default:
			std::wcout << "[                   ]                     wType: " << cur->wType << std::endl;
		}
	}
	DnsRecordListFree(pDnsRecord, DnsFreeRecordList);
}
void Browse_mDNS(void)
{
	DNS_SERVICE_BROWSE_REQUEST browseRequest{0};
	browseRequest.Version = DNS_QUERY_REQUEST_VERSION1;
	browseRequest.InterfaceIndex = 0;
	browseRequest.QueryName = L"_tivo-videos._tcp.local";
	browseRequest.pBrowseCallback = Browse_mDNS_Callback;
	browseRequest.pQueryContext = (PVOID)43;
	const DNS_STATUS result = DnsServiceBrowse(&browseRequest, &cancelBrowse);
	if (result == DNS_REQUEST_PENDING)
		std::cout << "[" << getTimeISO8601() << "] DnsServiceBrowse(DNS_REQUEST_PENDING)" << std::endl;
	else
		std::cout << "[" << getTimeISO8601() << "] DnsServiceBrowse(" << result << ") DNS_REQUEST_PENDING=" << DNS_REQUEST_PENDING << std::endl;
}
void Browse_mDNS_Cancel(void)
{
	auto result = DnsServiceBrowseCancel(&cancelBrowse);
	if (result != ERROR_SUCCESS)
		std::wcout << "[                   ] DnsServiceBrowseCancel(" << result << ")" << std::endl;
}
/////////////////////////////////////////////////////////////////////////////
bool m_TiVoBeaconListenThreadStopRequested = false;
/////////////////////////////////////////////////////////////////////////////
int main(int argc,      // Number of strings in array argv  
	char *argv[],   // Array of command-line argument strings  
	char *envp[])  // Array of environment variable strings  
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else if (!AfxSocketInit())
		{
			std::cout << "[" << getTimeISO8601() << "] IDP_SOCKETS_INIT_FAILED." << endl;
			nRetCode = 1;
		}
		else
        {
			TCHAR filename[1024];
			unsigned long buffersize = sizeof(filename) / sizeof(TCHAR);
			// Get the file name that we are running from.
			GetModuleFileName(AfxGetResourceHandle(), filename, buffersize);
			std::cout << "[" << getTimeISO8601() << "] Program: " << CStringA(filename).GetString() << std::endl;
			std::cout << "[                   ] Version: " << CStringA(GetFileVersion(filename)).GetString() << " Built on: " __DATE__ " at " __TIME__ << std::endl;
			std::cout << "[                   ] Command Line Format:" << std::endl;
			std::cout << "[                   ] \t" << argv[0] << " [+Raw]" << std::endl;
			std::cout << "[                   ] Command: "; for (auto index = 0; index < argc; index++) std::cout << argv[index] << " "; std::cout << std::endl;
#ifdef DEBUG
			std::cout << "[" << getTimeISO8601() << "] Current locale setting is \"" << std::cout.getloc().name().c_str() << "\"" << endl;
			std::cout << "[                   ] 1000.010 == " << 1000.010 << endl;
			std::cout.imbue(std::locale(""));  // imbue global locale
			std::cout << "[" << getTimeISO8601() << "] Current locale setting is \"" << std::cout.getloc().name().c_str() << "\"" << endl;
			std::cout << "[                   ] 1000.010 == " << 1000.010 << endl;
			#endif // DEBUG

			// Adapted from example code at http://msdn2.microsoft.com/en-us/library/aa365917.aspx
			// Now get Windows' IPv4 addresses table.  Once again, we gotta call GetIpAddrTable()
			// multiple times in order to deal with potential race conditions properly.
			MIB_IPADDRTABLE * ipTable = NULL;
			{
				ULONG bufLen = 0;
				for (auto i = 0; i<5; i++)
				{
					DWORD ipRet = GetIpAddrTable(ipTable, &bufLen, false);
					if (ipRet == ERROR_INSUFFICIENT_BUFFER)
					{
						free(ipTable);  // in case we had previously allocated it
						ipTable = (MIB_IPADDRTABLE *)malloc(bufLen);
					}
					else if (ipRet == NO_ERROR) break;
					else
					{
						free(ipTable);
						ipTable = NULL;
						break;
					}
				}
			}
			if (ipTable)
			{
				// Try to get the Adapters-info table, so we can given useful names to the IP
				// addresses we are returning.  Gotta call GetAdaptersInfo() up to 5 times to handle
				// the potential race condition between the size-query call and the get-data call.
				// I love a well-designed API :^P
				IP_ADAPTER_INFO * pAdapterInfo = NULL;
				{
					ULONG bufLen = 0;
					for (auto i = 0; i<5; i++)
					{
						DWORD apRet = GetAdaptersInfo(pAdapterInfo, &bufLen);
						if (apRet == ERROR_BUFFER_OVERFLOW)
						{
							free(pAdapterInfo);  // in case we had previously allocated it
							pAdapterInfo = (IP_ADAPTER_INFO *)malloc(bufLen);
						}
						else if (apRet == ERROR_SUCCESS) break;
						else
						{
							free(pAdapterInfo);
							pAdapterInfo = NULL;
							break;
						}
					}
				}
				for (auto i = 0; i<ipTable->dwNumEntries; i++)
				{
					std::cout << "[                   ] Interface:";
					const MIB_IPADDRROW & row = ipTable->table[i];

					// Now lookup the appropriate adaptor-name in the pAdaptorInfos, if we can find it
					std::string name;
					std::string desc;
					if (pAdapterInfo)
					{
						IP_ADAPTER_INFO * next = pAdapterInfo;
						while ((next) && (name.empty()))
						{
							IP_ADDR_STRING * ipAddr = &next->IpAddressList;
							while (ipAddr)
							{
								auto check1 = ntohl(inet_addr(ipAddr->IpAddress.String));
								auto check2 = ntohl(row.dwAddr);
								if (ntohl(inet_addr(ipAddr->IpAddress.String)) == ntohl(row.dwAddr))
								{
									name = next->AdapterName;
									desc = next->Description;
									break;
								}
								ipAddr = ipAddr->Next;
							}
							next = next->Next;
						}
					}
					if (name.empty())
					{
						std::stringstream out;
						out << "unnamed-" << i;
						name = out.str();
					}
					std::cout << " name=[" << name << "]";
					std::cout << " desc=[" << (desc.empty() ? "unavailable" : desc) << "]";
					in_addr ipAddr, netmask, baddr;
					ipAddr.S_un.S_addr = row.dwAddr;
					netmask.S_un.S_addr = row.dwMask;
					baddr.S_un.S_addr = ipAddr.S_un.S_addr & netmask.S_un.S_addr;
					if (row.dwBCastAddr) 
						baddr.S_un.S_addr |= ~netmask.S_un.S_addr;
					std::cout << " address=[" << inet_ntoa(ipAddr) << "]";
					std::cout << " netmask=[" << inet_ntoa(netmask) << "]";
					std::cout << " broadcastAddr=[" << inet_ntoa(baddr) << "]";
					std::cout << std::endl;
				}
				free(pAdapterInfo);
				free(ipTable);
			}

			Browse_mDNS();

			std::cout << "[" << getTimeISO8601() << "] Listening for UDP TiVo Beacons..." << std::endl;

			bool bDisplayRawBeacon = false;
			for (auto index = 0; index < argc; index++)
				if (CString(argv[index]).CompareNoCase(_T("+Raw")) == 0)
					bDisplayRawBeacon = true;

			do {
				SOCKET theSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
				if (theSocket == INVALID_SOCKET)
					m_TiVoBeaconListenThreadStopRequested = true;
				else
				{
					SOCKADDR_IN saClient;
					saClient.sin_family = AF_INET;
					saClient.sin_addr.s_addr = INADDR_ANY;
					saClient.sin_port = htons(2190);
					int nRet = bind(theSocket, (LPSOCKADDR)&saClient, sizeof(SOCKADDR_IN));
					if (nRet == SOCKET_ERROR)
					{
						auto TheError = WSAGetLastError();
						if (WSAEADDRINUSE == TheError)
							m_TiVoBeaconListenThreadStopRequested = true;
					}
					else
					{
						char szBuf[2048];
						SOCKADDR_IN saServer;
						int nLen = sizeof(SOCKADDR_IN);
						nRet = recvfrom(theSocket, szBuf, sizeof(szBuf), 0, (LPSOCKADDR)&saServer, &nLen);
						if (nRet != INVALID_SOCKET)
						{
							// here's where I look at what I recieve.
							CStringA csServerBroadcast(szBuf, nRet);
							cTiVoServer myServer;
							myServer.ReadTXT(csServerBroadcast.GetString(), '\n');
							myServer.m_address = inet_ntoa(saServer.sin_addr);
							std::cout << "[" << getTimeISO8601() << "] " << inet_ntoa(saServer.sin_addr) << " " << myServer.WriteTXT(' ') << std::endl;
							if (bDisplayRawBeacon)
							{
								csServerBroadcast.Trim();
								csServerBroadcast.Replace("\n", "\n[                   ] ");
								csServerBroadcast.Insert(0, "[                   ] ");
								std::cout << csServerBroadcast.GetString() << std::endl;
							}
						}
					}
					closesocket(theSocket);
				}
			} while (m_TiVoBeaconListenThreadStopRequested == false);
			Browse_mDNS_Cancel();
		}
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
