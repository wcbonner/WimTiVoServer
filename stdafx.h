// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

//#ifndef VC_EXTRALEAN
//#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
//#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <afxsock.h>		// MFC socket extensions
#include <afxinet.h>
#include <afxmt.h>			// for CCriticalSection
#include <iostream>
#include <locale>
#include <iomanip>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <codecvt>
#include <locale>
#include <regex>
#include <signal.h>
#include <memory> // for smart pointers
#include <xmllite.h>
#pragma comment(lib, "xmllite")
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include <bcrypt.h>
#pragma comment(lib, "bcrypt")
#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")
#include <windns.h> // for mDNS functions related to Bonjour https://learn.microsoft.com/en-us/windows/win32/api/windns/nf-windns-dnsserviceregister
#pragma comment(lib, "dnsapi.lib")
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

// Helper: convert in_addr to std::string using InetNtop (reentrant)
static inline std::string InetAddrToString(const in_addr& addr)
{
	char buf[INET_ADDRSTRLEN] = {0};
	if (InetNtopA(AF_INET, (void*)&addr, buf, sizeof(buf)) == NULL)
		return std::string();
	return std::string(buf);
}

// Helper: convert dotted IP string to in_addr using InetPton
static inline in_addr InetAddrFromString(const char* s)
{
	in_addr addr = {0};
	if (s && InetPtonA(AF_INET, s, &addr) == 1)
		return addr;
	addr.S_un.S_addr = INADDR_NONE;
	return addr;
}

// FFMPEG Libraries
//extern "C" {
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libavutil/dict.h>
//}
//#pragma comment(lib, "avformat")
//#pragma comment(lib, "avcodec")
//#pragma comment(lib, "avutil")

//#pragma comment(lib, "avdevice")
//#pragma comment(lib, "avfilter")
//#pragma comment(lib, "postproc")
//#pragma comment(lib, "swresample")
//#pragma comment(lib, "swscale")

#include "WimISO8601.h"
#include "WimTiVoServerEventLog.h"
#include "Dtwinver.h"
#define _INTERNAL_TiVoDecode
#ifdef _INTERNAL_TiVoDecode
#include "libTiVoDecode/happyfile.h"
#include "libTiVoDecode/turing_stream.h"
#include "libTiVoDecode/tivodecoder.h"
#endif