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
#include <sstream>
#include <vector>
#include <queue>
//#include <deque>
//#include <map>
#include <algorithm>
#include <chrono>
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

#include "WimTiVoServerEventLog.h"
#include "Dtwinver.h"
#include "libTiVoDecode/happyfile.h"
#include "libTiVoDecode/turing_stream.h"
#include "libTiVoDecode/tivodecoder.h"