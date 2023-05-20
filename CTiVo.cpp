#include "stdafx.h"
#include "CTiVo.h"
#pragma comment(lib, "wininet") // For some reason InternetCanonicalizeUrl() wasn't linking without this.
using namespace std;

static std::string timeToISO8601(const time_t & TheTime)
{
	std::ostringstream ISOTime;
	struct tm UTC;
	gmtime_s(&UTC, &TheTime);
	ISOTime.fill('0');
	ISOTime << UTC.tm_year+1900 << "-";
	ISOTime.width(2);
	ISOTime << UTC.tm_mon+1 << "-";
	ISOTime.width(2);
	ISOTime << UTC.tm_mday << "T";
	ISOTime.width(2);
	ISOTime << UTC.tm_hour << ":";
	ISOTime.width(2);
	ISOTime << UTC.tm_min << ":";
	ISOTime.width(2);
	ISOTime << UTC.tm_sec;
	return(ISOTime.str());
}
//static std::string timeToISO8601(const CTime & TheTime)
//{
//	time_t TheOtherTime;
//	//mktime(
//	//TheTime.
//	return(timeToISO8601(TheOtherTime));
//}
static std::string timeToISO8601(const CTimeSpan & TheTimeSpan)
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
static std::string timeToExcelDate(const time_t & TheTime)
{
	std::ostringstream ISOTime;
	struct tm UTC;
	gmtime_s(&UTC, &TheTime);
	ISOTime.fill('0');
	ISOTime << UTC.tm_year+1900 << "-";
	ISOTime.width(2);
	ISOTime << UTC.tm_mon+1 << "-";
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
static std::string getTimeISO8601(void)
{
	time_t timer;
	time(&timer);
	return(timeToISO8601(timer));
}
static time_t ISO8601totime(const std::string & ISOTime)
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
static const CString QuoteFileName(const CString & Original)
{
	CString csQuotedString(Original);
	// csQuotedString.Replace(_T("\\"), _T("/"));
	if (csQuotedString.Find(_T(" ")) >= 0)
	{
		csQuotedString.Insert(0,_T('"'));
		csQuotedString.AppendChar(_T('"'));
	}
	return(csQuotedString);
}
/////////////////////////////////////////////////////////////////////////////
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
		CString csKey(csSect.Tokenize(_T("="),KeyPos));
		CString csValue(csSect.Tokenize(_T("="),KeyPos));
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
bool CTiVoContainer::operator==(const CTiVoContainer & other) const
{
	return(
		m_title == other.m_title
		);
}
std::string CTiVoContainer::WriteTXT(const char seperator) const
{
	std::stringstream ssValue;
	ssValue << "title=" << m_title;
	if (!m_url.empty()) ssValue << seperator << "url=" << m_url;
	//if (!m_MAK.empty()) ssValue << seperator << "MAK=" << m_MAK;
	//if (!m_ContentType.empty()) ssValue << seperator << "ContentType=" << m_ContentType;
	//if (!m_SourceFormat.empty()) ssValue << seperator << "SourceFormat=" << m_SourceFormat;
	return(ssValue.str());
}
bool CTiVoContainer::ReadTXT(const std::string & text, const char seperator)
{
	bool rval = true;
	CString csText(text.c_str());
	int SectPos = 0;
	CString csSect(csText.Tokenize(CString(seperator), SectPos));
	while (csSect != _T(""))
	{
		int KeyPos = csSect.Find(_T("="));
		if (KeyPos > 0)
		{
			CString csKey(csSect.Left(KeyPos));
			CString csValue(csSect.Right(csSect.GetLength() - (KeyPos+1)));
			if (!csKey.CompareNoCase(_T("title")))
				m_title = CStringA(csValue);
			else if (!csKey.CompareNoCase(_T("url")))
				m_url = CStringA(csValue);
			else if (!csKey.CompareNoCase(_T("MAK")))
				m_MAK = CStringA(csValue);
			else if (!csKey.CompareNoCase(_T("ContentType")))
				m_ContentType = CStringA(csValue);
			else if (!csKey.CompareNoCase(_T("SourceFormat")))
				m_SourceFormat = CStringA(csValue);
		}
		csSect = csText.Tokenize(CString(seperator), SectPos);
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
bool cTiVoFile::operator==(const cTiVoFile & other) const
{
	return(
		m_csPathName == other.m_csPathName
		);
}
const CString csUrlPrefix(_T("/TiVoConnect/TivoNowPlaying/"));
void cTiVoFile::SetPathName(const CString csNewPath)
{
	m_csPathName = csNewPath;
	std::string isotime(getTimeISO8601());
	std::wstring wistotime(isotime.begin(),isotime.end());
	std::wcout << L"[" << wistotime << L"] " << m_csPathName.GetString() << endl;
	struct _stat buf;
	if (0 == _tstat( m_csPathName.GetString(), &buf ))
	{
		m_SourceSize = buf.st_size;
		//char timebuf[26];
		//ctime_s(timebuf, 26, &buf.st_mtime);
		//timebuf[24] = 0; // Get rid of \n that ctime_s appends to end of string
		//cout << "[                   ] " << setw(20) << right << "File size" << " : " << m_SourceSize << endl;
		//cout << "[                   ] " << setw(20) << right << "Drive" << " : " << char(buf.st_dev + 'A') << ":" << endl;
		//cout << "[                   ] " << setw(20) << right << "Time modified" << " : " << timebuf << endl;

		m_CaptureDate = CTime(buf.st_mtime);
		#ifdef AVCODEC_AVCODEC_H
		PopulateFromFFMPEG();
		#endif
		PopulateFromFFProbe();

		if (m_Title.IsEmpty())
		{
			TCHAR path_buffer[_MAX_PATH];
			TCHAR drive[_MAX_DRIVE];
			TCHAR dir[_MAX_DIR];
			TCHAR fname[_MAX_FNAME];
			TCHAR ext[_MAX_EXT];
			_tsplitpath_s(m_csPathName.GetString(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
			m_Title = fname;
			m_Title.Replace(_T("."), _T(" ")); // replace periods with spaces as on 2019-12-09
			m_Title.Replace(_T("_"), _T(" ")); // replace underscore with spaces as on 2019-12-09
		}
	}
	m_ContentType = _T("video/x-tivo-mpeg");
	//m_ContentType = _T("video/x-tivo-mpeg-ts");
	// Final Output of object values
	wcout << L"[                   ] " << setw(20) << right << L"m_Title" << L" : " << m_Title.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_EpisodeTitle" << L" : " << m_EpisodeTitle.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_Description" << L" : " << m_Description.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_ContentType" << L" : " << m_ContentType.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_SourceFormat" << L" : " << m_SourceFormat.GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"LastChangeDate" << L" : " << LastChangeDate.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_SourceSize" << L" : " << m_SourceSize << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_Duration" << L" : " << m_Duration << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_CaptureDate" << L" : " << m_CaptureDate.Format(_T("%c")).GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"URL" << L" : " << GetURL().GetString() << endl;
}	
void cTiVoFile::SetPathName(const CFileFind & csNewPath)
{
	m_csPathName = csNewPath.GetFilePath();
	m_SourceSize = csNewPath.GetLength();
	m_Title = csNewPath.GetFileTitle();
	m_Title.Replace(_T("."), _T(" ")); // replace periods with spaces as on 2019-12-09
	m_Title.Replace(_T("_"), _T(" ")); // replace underscore with spaces as on 2019-12-09
	m_ContentType = _T("video/x-tivo-mpeg");
	//m_ContentType = _T("video/x-tivo-mpeg-ts");
	csNewPath.GetLastWriteTime(m_LastWriteTime);
	csNewPath.GetLastWriteTime(m_CaptureDate);
	CFile XMLFile;
	CString XMLFilePath(m_csPathName);
	XMLFilePath.Truncate(XMLFilePath.ReverseFind(_T('.')));
	XMLFilePath.Append(_T(".xml"));
	if (XMLFile.Open(XMLFilePath, CFile::modeRead | CFile::shareDenyWrite))
	{
		ULONGLONG dwLength = XMLFile.GetLength();
		char * RawBuffer = new char[dwLength];
		int NumRead = XMLFile.Read(RawBuffer, dwLength);
		m_TvBusEnvelope = CString(RawBuffer, NumRead);
		delete[] RawBuffer;
		XMLFile.Close();
	}

	if (!csNewPath.GetFileName().Right(5).CompareNoCase(_T(".tivo")))
		m_SourceFormat = _T("video/x-tivo-mpeg"); 	// this block is run if I'm transferring a file with a .tivo extension
	else
	{
		#ifdef AVCODEC_AVCODEC_H
		PopulateFromFFMPEG();
		#endif
		PopulateFromFFProbe();
	}
	if (m_SourceSize == 0)
		m_SourceSize = csNewPath.GetLength();
	// Final Output of object values
	TRACE("%s %s m_csPathName %s %s\n", CStringA(CTime::GetCurrentTime().Format(_T("[%Y-%m-%dT%H:%M:%S]"))).GetString(), __FUNCTION__, CStringA(m_csPathName).GetString(), CStringA(m_SourceFormat).GetString());
	//wcout << L"[                   ] " << setw(20) << right << L"m_csPathName" << L" : " << m_csPathName.GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"m_Title" << L" : " << m_Title.GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"m_EpisodeTitle" << L" : " << m_EpisodeTitle.GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"m_Description" << L" : " << m_Description.GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"m_ContentType" << L" : " << m_ContentType.GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"m_SourceFormat" << L" : " << m_SourceFormat.GetString() << endl;
	////wcout << L"[                   ] " << setw(20) << right << L"LastChangeDate" << L" : " << LastChangeDate.GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"m_SourceSize" << L" : " << m_SourceSize << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"m_Duration" << L" : " << m_Duration << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"m_CaptureDate" << L" : " << m_CaptureDate.Format(_T("%c")).GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"URL" << L" : " << m_csURL.GetString() << endl;
}
void cTiVoFile::SetFromTiVoItem(const CString &csTitle, const CString &csEpisodeTitle, const CString &csDescription, const CString &csSourceStation, const CString &csContentURL, const CTime &ctCaptureDate, const CTimeSpan &ctsDuration, const CString & csMAK, const unsigned long long llSourceSize)
{
	m_Title = csTitle;
	m_EpisodeTitle = csEpisodeTitle;
	m_Description = csDescription;
	m_Description.Replace(_T("Copyright Tribune Media Services, Inc."), _T("")); // Hack to get rid of copyright notice in the descriptive text.
	m_Description.Replace(_T("Copyright Rovi, Inc."), _T("")); // Hack to get rid of copyright notice in the descriptive text.
	m_Description.Trim();
	m_csURL = csContentURL;
	//for (auto TiVoFile = pDoc->m_TiVoFiles.begin(); TiVoFile != pDoc->m_TiVoFiles.end(); TiVoFile++)
	//	TiVoFile->SetURL(CString(DereferenceURL(CStringA(TiVoFile->GetURL()).GetString(), CStringA(csURL).GetString()).c_str()));
	m_CaptureDate = ctCaptureDate;
	m_Duration = 1000 * ctsDuration.GetTotalSeconds();
	m_csMAK = csMAK;
	m_SourceFormat = _T("video/x-tivo-mpeg");
	m_SourceSize = llSourceSize;
	m_SourceStation = csSourceStation;
	std::wstringstream ssFileName(std::stringstream::in | std::stringstream::out);
	ssFileName << csTitle.GetString();
	if (!csEpisodeTitle.IsEmpty())
		ssFileName << L" - ''" << csEpisodeTitle.GetString() << L"''";
	ssFileName << L" (Recorded " << m_CaptureDate.Format(_T("%b %d, %Y")).GetString();
	if (!m_SourceStation.IsEmpty())
		ssFileName << ", " << m_SourceStation.GetString();
	ssFileName << L").TiVo";
	m_csPathName = ssFileName.str().c_str();
	m_csPathName.Replace(_T("\""),_T("''")); // http://msdn.microsoft.com/en-us/library/system.io.path.getinvalidfilenamechars.aspx should be further examined
	m_csPathName.Replace(_T(":"),_T("_")); // http://msdn.microsoft.com/en-us/library/system.io.path.getinvalidfilenamechars.aspx should be further examined
	m_csPathName.Replace(_T("?"),_T("")); // http://msdn.microsoft.com/en-us/library/system.io.path.getinvalidfilenamechars.aspx should be further examined
	m_csPathName.Replace(_T("*"),_T("")); // http://msdn.microsoft.com/en-us/library/system.io.path.getinvalidfilenamechars.aspx should be further examined
	m_csPathName.Replace(_T("<"),_T("")); // http://msdn.microsoft.com/en-us/library/system.io.path.getinvalidfilenamechars.aspx should be further examined
	m_csPathName.Replace(_T(">"),_T("")); // http://msdn.microsoft.com/en-us/library/system.io.path.getinvalidfilenamechars.aspx should be further examined
	m_csPathName.Replace(_T("|"),_T("")); // http://msdn.microsoft.com/en-us/library/system.io.path.getinvalidfilenamechars.aspx should be further examined
}
#ifdef AVCODEC_AVCODEC_H
void cTiVoFile::PopulateFromFFMPEG(void)
{
	// Change so that I only get the details from inside the file if details are requested
	int Oldavlog = av_log_get_level();
	av_log_set_level(AV_LOG_FATAL);
	AVFormatContext *fmt_ctx = NULL;
	if (0 == avformat_open_input(&fmt_ctx, CStringA(m_csPathName).GetString(), NULL, NULL))
	{
		if(0 <= avformat_find_stream_info(fmt_ctx, NULL))					
		{
			if (fmt_ctx->duration != AV_NOPTS_VALUE) 
				m_Duration = (fmt_ctx->duration + 5000) / (AV_TIME_BASE / 1000); // this makes at least my first example match the tivo desktop software

			//// Find the first video stream
			int videoStream=-1;
			for(int i = 0; i < fmt_ctx->nb_streams; i++)
				if(fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
				{
					videoStream=i;
					break;
				}
			if(videoStream != -1)
			{
				const char *codec_type = av_get_media_type_string(fmt_ctx->streams[videoStream]->codec->codec_type);
				const char *codec_name = avcodec_get_name(fmt_ctx->streams[videoStream]->codec->codec_id);
				if (!CStringA(codec_name).Compare("mpeg2video"))
					m_VideoCompatible = true;
				m_SourceFormat = CStringA(codec_type);
				m_SourceFormat.Append(_T("/"));
				m_SourceFormat.Append(CString(CStringA(codec_name)));
				if (fmt_ctx->streams[videoStream]->codec->width >= 1280)
					m_VideoHighDefinition = true;
				//if (fmt_ctx->streams[videoStream]->codec->codec_tag) 
				//{
				//	char tag_buf[32];
				//	av_get_codec_tag_string(tag_buf, sizeof(tag_buf), fmt_ctx->streams[videoStream]->codec->codec_tag);
				//	m_SourceFormat = CStringA(codec_type);
				//	m_SourceFormat.Append(_T("/"));
				//	m_SourceFormat.Append(CString(CStringA(tag_buf)));
				//}
				//char buf[256];
				//avcodec_string(buf, sizeof(buf), fmt_ctx->streams[videoStream]->codec, 1);
				//m_Duration = fmt_ctx->streams[videoStream]->duration;
				//cout << "fmt_ctx->streams[videoStream]->duration " << fmt_ctx->streams[videoStream]->duration << endl;
				//cout << "fmt_ctx->streams[videoStream]->time_base " << fmt_ctx->streams[videoStream]->time_base << endl;
				// Get a pointer to the codec context for the video stream
				//AVCodecContext *pCodecCtx=fmt_ctx->streams[videoStream]->codec;

				//// Find the decoder for the video stream
				//AVCodec *pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
				//if(pCodec==NULL) 
				//	cout << "Unsupported codec!" << endl;
				//else
				//{
				//	// Open codec
				//	if(avcodec_open(pCodecCtx, pCodec)<0)
				//		cout << "Could not open codec!" << endl;
				//	else
				//		avcodec_close(pCodecCtx);
				//}
			}
			//// Find the first audio stream
			int audioStream=-1;
			for(int i = 0; i < fmt_ctx->nb_streams; i++)
				if(fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) 
				{
					audioStream=i;
					break;
				}
			if(audioStream != -1)
			{
				const char *codec_type = av_get_media_type_string(fmt_ctx->streams[audioStream]->codec->codec_type);
				const char *codec_name = avcodec_get_name(fmt_ctx->streams[audioStream]->codec->codec_id);
				if (!CStringA(codec_name).Compare("ac3"))
					m_AudioCompatible = true;
			}
			// This next section looks at metadata
			AVDictionaryEntry *tag = NULL;
			while (tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))
			{
				//#ifdef _DEBUG
				//cout << "[                   ] " << setw(20) << right << tag->key << " : " << tag->value << endl;
				//#endif
				if (_stricmp("title", tag->key) == 0)
					m_Title = CString(tag->value);
				if (_stricmp("episode_id", tag->key) == 0)
					m_EpisodeTitle = CString(tag->value);
				if (_stricmp("description", tag->key) == 0)
					m_Description = CString(tag->value);
				if (_stricmp("WM/SubTitle", tag->key) == 0)
					m_EpisodeTitle = CString(tag->value);
				if (_stricmp("WM/SubTitleDescription", tag->key) == 0)
					m_Description = CString(tag->value);
				if (_stricmp("genre", tag->key) == 0)
					m_vProgramGenre = CString(tag->value);
				if (_stricmp("service_provider", tag->key) == 0)
					m_SourceStation = CString(tag->value);
				if (_stricmp("WM/MediaOriginalChannel", tag->key) == 0)
					m_SourceChannel = CString(tag->value);
				if (_stricmp("WM/MediaCredits", tag->key) == 0)
				{
					CString Value(tag->value);
					while (0 < Value.Replace(_T(";;"),_T(";")));
					while (0 < Value.Replace(_T("//"),_T("/")));
					m_vActor = Value;
				}
				if (_stricmp("WM/WMRVEncodeTime", tag->key) == 0)
				{
					CTime OriginalBroadcastDate = ISO8601totime(std::string(tag->value));
					if (OriginalBroadcastDate > 0)
						m_CaptureDate = OriginalBroadcastDate;
				}
				if (_stricmp("WM/MediaOriginalBroadcastDateTime", tag->key) == 0)
				{
					CTime OriginalBroadcastDate = ISO8601totime(std::string(tag->value));
					if (OriginalBroadcastDate > 0)
						m_CaptureDate = OriginalBroadcastDate;
				}
				//if (_stricmp("m_Duration", tag->key) == 0)
				//{
				//	std::stringstream ss(std::string(tag->value), std::stringstream::in | std::stringstream::out);
				//	ss >> m_Duration;
				//	// m_Duration should now be in 100-nanosecond units. The TiVo wants it in 1/1000 of a second units. http://msdn.microsoft.com/en-us/library/windows/desktop/ff384862(v=vs.85).aspx
				//	m_Duration /= 1000;
				//}
			}
			m_Description.Replace(_T("Copyright Tribune Media Services, Inc."), _T("")); // Hack to get rid of copyright notice in the descriptive text.
			m_Description.Replace(_T("Copyright Rovi, Inc."), _T("")); // Hack to get rid of copyright notice in the descriptive text.
			m_Description.Trim();
		}
		avformat_close_input(&fmt_ctx);
	}
	av_log_set_level(Oldavlog);
}
#endif
extern CString FindEXEFromPath(const CString & csEXE);
void cTiVoFile::PopulateFromFFProbe(void)
{
	static const CString csFFProbePath(FindEXEFromPath(_T("ffprobe.exe")));
	if (!csFFProbePath.IsEmpty())
	{
		//csFFProbePath -show_streams -show_format -print_format xml "\\Acid\TiVo\NOVA - ''Ancient Computer'' (Recorded Apr 05, 2013, KCTSDT).mp4"
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
 
				//static const CString csFFMPEGPath(FindEXEFromPath(_T("ffmpeg.exe")));
				CString csCommandLine(QuoteFileName(csFFProbePath));
				csCommandLine.Append(_T(" -hide_banner -show_streams -show_format -print_format xml "));
				csCommandLine.Append(QuoteFileName(m_csPathName));

				std::cout << "[" << getTimeISO8601() << "] CreateProcess: ";
				std::wcout << csCommandLine.GetString() << std::endl;
				TRACE(_T("CreateProcess: %s\n"), csCommandLine.GetString());
				// Create the child process.
				if (CreateProcess(NULL, 
					(LPTSTR) csCommandLine.GetString(),     // command line 
					NULL,          // process security attributes 
					NULL,          // primary thread security attributes 
					TRUE,          // handles are inherited 
					0,             // creation flags 
					NULL,          // use parent's environment 
					NULL,          // use parent's current directory 
					&siStartInfo,  // STARTUPINFO pointer 
					&piProcInfo))  // receives PROCESS_INFORMATION 
				{
					CloseHandle(g_hChildStd_OUT_Wr);	// If I don't do this, then the parent will never exit!
					CComPtr<IStream> spMemoryStreamOne(::SHCreateMemStream(NULL, 0));
					if (spMemoryStreamOne != NULL)
					{
						const int RAWDataBuffSize = 0x1000;	// 0x1000 is 4k
						char * RAWDataBuff = new char[RAWDataBuffSize];
						for (;;)
						{
							DWORD dwRead = 0;
							BOOL bSuccess = ReadFile(g_hChildStd_OUT_Rd, RAWDataBuff, RAWDataBuffSize, &dwRead, NULL);
							if( (!bSuccess) || (dwRead == 0)) break;
							ULONG cbWritten;
							spMemoryStreamOne->Write(RAWDataBuff, dwRead, &cbWritten);
						} 
						delete[] RAWDataBuff;
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
									int indentlevel = 0;
									XmlNodeType nodeType; 
									const WCHAR* pwszLocalName;
									const WCHAR* pwszValue;
									CString csLocalName;
									bool bIsFormat = false;
									bool bVideoStreamInfoNeeded = true;
									bool bAudioStreamInfoNeeded = true;

									//read until there are no more nodes 
									while (S_OK == (hr = pReader->Read(&nodeType))) 
									{
										if (nodeType == XmlNodeType_Element)
										{
											if (SUCCEEDED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
											{
												csLocalName = CString(pwszLocalName);
												if ((bVideoStreamInfoNeeded || bAudioStreamInfoNeeded) && !csLocalName.Compare(_T("stream")))
												{
													CString cs_codec_name;
													CString cs_codec_type;
													CString cs_codec_time_base;
													CString cs_width;
													CString cs_height;
													CString cs_duration;
													while (S_OK == pReader->MoveToNextAttribute())
													{
														if (SUCCEEDED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
															if (SUCCEEDED(hr = pReader->GetValue(&pwszValue, NULL)))
														{
															csLocalName = CString(pwszLocalName);
															if (!csLocalName.Compare(_T("codec_name")))
																cs_codec_name = CString(pwszValue);
															else if (!csLocalName.Compare(_T("codec_type")))
																cs_codec_type = CString(pwszValue);
															else if (!csLocalName.Compare(_T("codec_time_base")))
																cs_codec_time_base = CString(pwszValue);
															else if (!csLocalName.Compare(_T("width")))
																cs_width = CString(pwszValue);
															else if (!csLocalName.Compare(_T("height")))
																cs_height = CString(pwszValue);
															else if (!csLocalName.Compare(_T("duration")))
																cs_duration = CString(pwszValue);
														}
													}
													if (!cs_codec_type.Compare(_T("video")))
													{
														bVideoStreamInfoNeeded = false;
														if (!cs_codec_name.Compare(_T("mpeg2video")))
															m_VideoCompatible = true;
														m_SourceFormat = cs_codec_type + CString(_T("/")) + cs_codec_name;
														int width = 0;
														std::wstringstream ss;
														ss << cs_width.GetString();
														ss >> width;
														if (width > m_VideoWidth)
															m_VideoWidth = width;
														if (width >= 1280)
															m_VideoHighDefinition = true;
														int hieght = 0;
														ss = std::wstringstream();
														ss << cs_height.GetString();
														ss >> hieght;
														if (hieght > m_VideoHeight)
															m_VideoHeight = hieght;
														double duration = 0;
														ss = std::wstringstream();
														ss << cs_duration.GetString();
														ss >> duration;
														//m_Duration = (duration + 5000) / (AV_TIME_BASE / 1000); // this makes at least my first example match the tivo desktop software
														m_Duration = duration * 1000 + 5; // this makes at least my first example match the tivo desktop software
													}
													else if (!cs_codec_type.Compare(_T("audio")))
													{
														bAudioStreamInfoNeeded = false;
														if (!cs_codec_name.Compare(_T("ac3")))
															m_AudioCompatible = true;
													}	
												}
												else if (!csLocalName.Compare(_T("format")))
												{
													bIsFormat = true;
													const CString ccs_duration(_T("duration"));
													while (S_OK == pReader->MoveToNextAttribute())
													{
														if (SUCCEEDED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
															if (SUCCEEDED(hr = pReader->GetValue(&pwszValue, NULL)))
														{
															if (!ccs_duration.Compare(pwszLocalName))
															{
																double duration = 0;
																std::wstringstream ss;
																ss << pwszValue;
																ss >> duration;
																m_Duration = duration * 1000 + 5;
															}
														}
													}
												}
												// Here's where I need to dig deeper.
												else if (bIsFormat && (!csLocalName.Compare(_T("tag"))))
												{
													CString csAttributeKey;
													CString csAttributeValue;
													while (S_OK == pReader->MoveToNextAttribute())
													{
														if (SUCCEEDED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
															if (SUCCEEDED(hr = pReader->GetValue(&pwszValue, NULL)))
														{
															if (!CString(_T("key")).Compare(pwszLocalName))
																csAttributeKey = CString(pwszValue);
															else if (!CString(_T("value")).Compare(pwszLocalName))
																csAttributeValue = CString(pwszValue);
														}
													}
													if (!csAttributeKey.CompareNoCase(_T("title")))
													{
														m_Title = csAttributeValue;
														m_Title.Replace(_T("."), _T(" ")); // replace periods with spaces as on 2019-12-09
														m_Title.Replace(_T("_"), _T(" ")); // replace underscore with spaces as on 2019-12-09
													}
													else if (!csAttributeKey.CompareNoCase(_T("episode_id")))
														m_EpisodeTitle = csAttributeValue;
													else if (!csAttributeKey.CompareNoCase(_T("description")))
														m_Description = csAttributeValue;
													else if (!csAttributeKey.CompareNoCase(_T("WM/SubTitle")))
														m_EpisodeTitle = csAttributeValue;
													else if (!csAttributeKey.CompareNoCase(_T("WM/SubTitleDescription")))
														m_Description = csAttributeValue;
													else if (!csAttributeKey.CompareNoCase(_T("genre")))
														m_vProgramGenre = csAttributeValue;
													else if (!csAttributeKey.CompareNoCase(_T("service_provider")))
														m_SourceStation = csAttributeValue;
													else if (!csAttributeKey.CompareNoCase(_T("WM/MediaOriginalChannel")))
														m_SourceChannel = csAttributeValue;
													else if (!csAttributeKey.CompareNoCase(_T("WM/MediaCredits")))
													{
														m_vActor = csAttributeValue;
														while (0 < m_vActor.Replace(_T(";;"),_T(";")));
														while (0 < m_vActor.Replace(_T("//"),_T("/")));
													}
													else if (!csAttributeKey.CompareNoCase(_T("WM/WMRVEncodeTime")))
													{
														CTime OriginalBroadcastDate = ISO8601totime(std::string(CStringA(csAttributeValue).GetString()));
														if (OriginalBroadcastDate > 0)
															m_CaptureDate = OriginalBroadcastDate;
													}
													else if (!csAttributeKey.CompareNoCase(_T("WM/MediaOriginalBroadcastDateTime")))
													{
														CTime OriginalBroadcastDate = ISO8601totime(std::string(CStringA(csAttributeValue).GetString()));
														if (OriginalBroadcastDate > 0)
															m_CaptureDate = OriginalBroadcastDate;
													}
													m_Description.Replace(_T("Copyright Tribune Media Services, Inc."), _T("")); // Hack to get rid of copyright notice in the descriptive text.
													m_Description.Replace(_T("Copyright Rovi, Inc."), _T("")); // Hack to get rid of copyright notice in the descriptive text.
													m_Description.Trim();
												}
											}
										}
										else if (nodeType == XmlNodeType_EndElement)
										{
											if (SUCCEEDED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
												if (!CString(pwszLocalName).Compare(_T("format")))
													bIsFormat = false;
										}
									}
								}
							}
						}
					}
					// Close handles to the child process and its primary thread.
					// Some applications might keep these handles to monitor the status
					// of the child process, for example. 
					CloseHandle(piProcInfo.hProcess);
					CloseHandle(piProcInfo.hThread);
				}
			}
			CloseHandle(g_hChildStd_OUT_Rd);
		}
	}
}
#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)
static const BYTE rgbMsg[] = { 0x61, 0x62, 0x63 }; 
const CString cTiVoFile::GetURL(void) const
{
	CString csrVal(m_csURL);
	if (csrVal.IsEmpty())
	{
		// https://docs.microsoft.com/en-us/windows/desktop/SecCNG/creating-a-hash-with-cng
		//open an algorithm handle
		BCRYPT_ALG_HANDLE hAlg = NULL;
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		if (NT_SUCCESS(status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0)))
		{
			//calculate the size of the buffer to hold the hash object
			DWORD cbData = 0;
			DWORD cbHashObject = 0;
			if (NT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0)))
			{
				//allocate the hash object on the heap
				PBYTE pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
				if (NULL != pbHashObject)
				{
					//calculate the length of the hash
					DWORD cbHash = 0;
					if (NT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0)))
					{
						//allocate the hash buffer on the heap
						PBYTE pbHash = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHash);
						if (NULL != pbHash)
						{
							//create a hash
							BCRYPT_HASH_HANDLE hHash = NULL;
							if (NT_SUCCESS(status = BCryptCreateHash(hAlg, &hHash, pbHashObject, cbHashObject, NULL, 0, 0)))
							{
								//hash some data
								//if (NT_SUCCESS(status = BCryptHashData(hHash, (PBYTE)rgbMsg, sizeof(rgbMsg), 0)))
								if (NT_SUCCESS(status = BCryptHashData(hHash, (PBYTE)m_csPathName.GetString(), m_csPathName.GetLength() * sizeof(TCHAR), 0)))
								{
									//close the hash
									if (NT_SUCCESS(status = BCryptFinishHash(hHash, pbHash, cbHash, 0)))
									{
										TCHAR lpszBuffer[_MAX_PATH];
										DWORD dwBufferLength = sizeof(lpszBuffer) / sizeof(TCHAR);
										if (CryptBinaryToStringW(pbHash, cbHash, CRYPT_STRING_HEX | CRYPT_STRING_NOCRLF, lpszBuffer, &dwBufferLength))
										{
											csrVal = csUrlPrefix;
											csrVal.Append(CString(lpszBuffer, dwBufferLength));
											csrVal.Replace(_T(" "), _T(""));
										}
									}
								}
							}
							if (hHash)
								BCryptDestroyHash(hHash);
						}
						if (pbHash)
							HeapFree(GetProcessHeap(), 0, pbHash);
					}
				}
				if (pbHashObject)
					HeapFree(GetProcessHeap(), 0, pbHashObject);
			}
		}
		if (hAlg)
			BCryptCloseAlgorithmProvider(hAlg, 0);
	}
	return(csrVal);
}
const CString & cTiVoFile::SetMAK(const CString & csMAK)
{
	CString csrVal(m_csMAK);
	m_csMAK = csMAK;
	return(csrVal);
}
void cTiVoFile::GetTiVoItem(CComPtr<IXmlWriter> & pWriter) const
{
	pWriter->WriteStartElement(NULL, L"Item", NULL);
		pWriter->WriteStartElement(NULL, L"Details", NULL);
			pWriter->WriteElementString(NULL, L"Title", NULL, m_Title.GetString());
			if (!m_EpisodeTitle.IsEmpty()) pWriter->WriteElementString(NULL, L"EpisodeTitle", NULL, m_EpisodeTitle.GetString());
			if (!m_Description.IsEmpty()) pWriter->WriteElementString(NULL, L"Description", NULL, m_Description.GetString());
			if (!m_SourceStation.IsEmpty()) pWriter->WriteElementString(NULL, L"SourceStation", NULL, m_SourceStation.GetString());
			if (!m_SourceChannel.IsEmpty()) pWriter->WriteElementString(NULL, L"SourceChannel", NULL, m_SourceChannel.GetString());
			if (!m_ContentType.IsEmpty()) pWriter->WriteElementString(NULL, L"ContentType", NULL, m_ContentType.GetString());
			//if (!m_ContentType.IsEmpty()) pWriter->WriteElementString(NULL, L"SourceFormat", NULL, m_ContentType.GetString());
			if (!m_SourceFormat.IsEmpty()) pWriter->WriteElementString(NULL, L"SourceFormat", NULL, m_SourceFormat.GetString());
			if (m_SourceSize > 0)
			{
				std::wstringstream ss;
				if (m_VideoHighDefinition)
					ss << max(m_SourceSize, m_Duration * 10000);
				else
					ss << max(m_SourceSize, m_Duration * 1400);
				pWriter->WriteElementString(NULL, L"SourceSize", NULL, ss.str().c_str());
			}
			if (m_Duration > 0)
			{
				std::wstringstream ss(std::stringstream::in | std::stringstream::out);
				ss << m_Duration;
				pWriter->WriteElementString(NULL, L"Duration", NULL, ss.str().c_str());
			}
			//if (m_CaptureDate.IsValidFILETIME())
			{
				CTime tempTime(m_CaptureDate);
				const CTime UnixEpochTime(1970,1,1,0,0,0);
				_tzset();
				long SecondsFromUTC;
				_get_timezone(&SecondsFromUTC);
				tempTime += CTimeSpan(SecondsFromUTC);	// the time difference was ~8 hours, which makes sense as the diff between UTC and local time.
				CTimeSpan TimeDiff = tempTime - UnixEpochTime;
				std::wstringstream ss(std::stringstream::in | std::stringstream::out);
				ss << showbase << hex << TimeDiff.GetTotalSeconds();
				pWriter->WriteElementString(NULL, L"CaptureDate", NULL, ss.str().c_str());
			}
		pWriter->WriteEndElement();	// Details
		if (!m_ContentType.IsEmpty() && !GetURL().IsEmpty())
		{
			pWriter->WriteStartElement(NULL, L"Links", NULL);
				pWriter->WriteStartElement(NULL, L"Content", NULL);
					pWriter->WriteElementString(NULL, L"ContentType", NULL, m_ContentType.GetString());
					pWriter->WriteElementString(NULL, L"Url", NULL, GetURL().GetString());
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"CustomIcon", NULL);
					pWriter->WriteElementString(NULL, L"ContentType", NULL, L"image/*");
					pWriter->WriteElementString(NULL, L"AcceptsParams", NULL, L"No");
					pWriter->WriteElementString(NULL, L"Url", NULL, L"urn:tivo:image:save-until-i-delete-recording");
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"TiVoVideoDetails", NULL);
					pWriter->WriteElementString(NULL, L"ContentType", NULL, L"text/xml");
					pWriter->WriteElementString(NULL, L"AcceptsParams", NULL, L"No");
					CString DetailsURL(_T("/TiVoConnect?Command=TVBusQuery&Url="));
					DetailsURL.Append(GetURL());
					pWriter->WriteElementString(NULL, L"Url", NULL, DetailsURL.GetString());
				pWriter->WriteEndElement();
			pWriter->WriteEndElement();
		}
	pWriter->WriteEndElement();	// Item
}
void cTiVoFile::GetTvBusEnvelope(CComPtr<IXmlWriter> & pWriter) const
{
	if (!m_TvBusEnvelope.IsEmpty())
		pWriter->WriteRaw(m_TvBusEnvelope.GetString());
	else
	{
		pWriter->WriteRaw(L"<TvBusMarshalledStruct:TvBusEnvelope xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:TvBusMarshalledStruct=\"http://tivo.com/developer/xml/idl/TvBusMarshalledStruct\" xmlns:TvPgdRecording=\"http://tivo.com/developer/xml/idl/TvPgdRecording\" xmlns:TvBusDuration=\"http://tivo.com/developer/xml/idl/TvBusDuration\" xmlns:TvPgdShowing=\"http://tivo.com/developer/xml/idl/TvPgdShowing\" xmlns:TvDbShowingBit=\"http://tivo.com/developer/xml/idl/TvDbShowingBit\" xmlns:TvBusDateTime=\"http://tivo.com/developer/xml/idl/TvBusDateTime\" xmlns:TvPgdProgram=\"http://tivo.com/developer/xml/idl/TvPgdProgram\" xmlns:TvDbColorCode=\"http://tivo.com/developer/xml/idl/TvDbColorCode\" xmlns:TvPgdSeries=\"http://tivo.com/developer/xml/idl/TvPgdSeries\" xmlns:TvDbShowType=\"http://tivo.com/developer/xml/idl/TvDbShowType\" xmlns:TvPgdBookmark=\"http://tivo.com/developer/xml/idl/TvPgdBookmark\" xmlns:TvPgdChannel=\"http://tivo.com/developer/xml/idl/TvPgdChannel\" xmlns:TvDbBitstreamFormat=\"http://tivo.com/developer/xml/idl/TvDbBitstreamFormat\" xs:schemaLocation=\"http://tivo.com/developer/xml/idl/TvBusMarshalledStruct TvBusMarshalledStruct.xsd http://tivo.com/developer/xml/idl/TvPgdRecording TvPgdRecording.xsd http://tivo.com/developer/xml/idl/TvBusDuration TvBusDuration.xsd http://tivo.com/developer/xml/idl/TvPgdShowing TvPgdShowing.xsd http://tivo.com/developer/xml/idl/TvDbShowingBit TvDbShowingBit.xsd http://tivo.com/developer/xml/idl/TvBusDateTime TvBusDateTime.xsd http://tivo.com/developer/xml/idl/TvPgdProgram TvPgdProgram.xsd http://tivo.com/developer/xml/idl/TvDbColorCode TvDbColorCode.xsd http://tivo.com/developer/xml/idl/TvPgdSeries TvPgdSeries.xsd http://tivo.com/developer/xml/idl/TvDbShowType TvDbShowType.xsd http://tivo.com/developer/xml/idl/TvPgdBookmark TvPgdBookmark.xsd http://tivo.com/developer/xml/idl/TvPgdChannel TvPgdChannel.xsd http://tivo.com/developer/xml/idl/TvDbBitstreamFormat TvDbBitstreamFormat.xsd\" xs:type=\"TvPgdRecording:TvPgdRecording\">");
		pWriter->WriteElementString(NULL, L"recordedDuration", NULL, CString(timeToISO8601(CTimeSpan(m_Duration/1000)).c_str()).GetString()); // This is ISO8601 Format for Duration
		pWriter->WriteStartElement(NULL, L"vActualShowing", NULL);pWriter->WriteEndElement();
		pWriter->WriteStartElement(NULL, L"vBookmark", NULL);pWriter->WriteEndElement();
		pWriter->WriteStartElement(NULL, L"recordingQuality", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"75");pWriter->WriteString(L"HIGH");pWriter->WriteEndElement();
		pWriter->WriteStartElement(NULL, L"showing", NULL);
			pWriter->WriteStartElement(NULL, L"showingBits", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"0");pWriter->WriteEndElement();
			pWriter->WriteElementString(NULL, L"time", NULL, m_CaptureDate.FormatGmt(_T("%Y-%m-%d:%H:%M:%SZ")));
			pWriter->WriteElementString(NULL, L"duration", NULL, CString(timeToISO8601(CTimeSpan(m_Duration/1000)).c_str()).GetString());
			pWriter->WriteStartElement(NULL, L"program", NULL);
				pWriter->WriteStartElement(NULL, L"vActor", NULL);
				if (!m_vActor.IsEmpty())
				{
					int iStart = 0;
					CString csToken(m_vActor.Tokenize(_T("/;"), iStart));
					while (!csToken.IsEmpty())
					{
						pWriter->WriteElementString(NULL, L"element", NULL, csToken.GetString());
						csToken = m_vActor.Tokenize(_T("/;"), iStart);
					}
				}
				pWriter->WriteEndElement();				
				pWriter->WriteStartElement(NULL, L"vAdvisory", NULL);
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"vChoreographer", NULL);
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"colorCode", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"4");pWriter->WriteString(L"COLOR");pWriter->WriteEndElement();
				pWriter->WriteElementString(NULL, L"description", NULL, m_Description.GetString());
				pWriter->WriteStartElement(NULL, L"vDirector", NULL);
				pWriter->WriteEndElement();
				pWriter->WriteElementString(NULL, L"episodeTitle", NULL, m_EpisodeTitle.GetString());
				pWriter->WriteStartElement(NULL, L"vExecProducer", NULL);
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"vProgramGenre", NULL);
				if (!m_vProgramGenre.IsEmpty())
				{
					int iStart = 0;
					CString csToken(m_vProgramGenre.Tokenize(_T(";"), iStart));
					while (!csToken.IsEmpty())
					{
						pWriter->WriteElementString(NULL, L"element", NULL, csToken.GetString());
						csToken = m_vProgramGenre.Tokenize(_T(";"), iStart);
					}
				}
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"vGuestStar", NULL);
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"vHost", NULL);
				pWriter->WriteEndElement();
				pWriter->WriteElementString(NULL, L"isEpisode", NULL, L"false");
				pWriter->WriteElementString(NULL, L"originalAirDate", NULL, m_CaptureDate.FormatGmt(_T("%Y-%m-%d:%H:%M:%SZ")));
				pWriter->WriteStartElement(NULL, L"vProducer", NULL);
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"series", NULL);
					pWriter->WriteElementString(NULL, L"isEpisodic", NULL, L"false");
					pWriter->WriteStartElement(NULL, L"vSeriesGenre", NULL);
					pWriter->WriteEndElement();
					pWriter->WriteElementString(NULL, L"seriesTitle", NULL, m_Title.GetString());
				pWriter->WriteEndElement();
				pWriter->WriteStartElement(NULL, L"showType", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"5");pWriter->WriteString(L"SERIES");pWriter->WriteEndElement();
				pWriter->WriteElementString(NULL, L"title", NULL, m_Title.GetString());
				pWriter->WriteStartElement(NULL, L"vWriter", NULL);
				pWriter->WriteEndElement();
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"channel", NULL);
				pWriter->WriteElementString(NULL, L"displayMajorNumber", NULL, NULL);
				pWriter->WriteElementString(NULL, L"displayMinorNumber", NULL, NULL);
				pWriter->WriteElementString(NULL, L"callsign", NULL, NULL);
			pWriter->WriteEndElement();
		pWriter->WriteEndElement();
		pWriter->WriteElementString(NULL, L"startTime", NULL, m_CaptureDate.FormatGmt(_T("%Y-%m-%d:%H:%M:%SZ")));
		pWriter->WriteElementString(NULL, L"stopTime", NULL, CTime(m_CaptureDate + CTimeSpan(m_Duration/1000)).FormatGmt(_T("%Y-%m-%d:%H:%M:%SZ")));
		pWriter->WriteRaw(L"</TvBusMarshalledStruct:TvBusEnvelope>");
	}
}
const CString cTiVoFile::GetFFMPEGCommandLine(const CString & csFFMPEGPath) const
{
	CString rval(QuoteFileName(csFFMPEGPath));
	rval.Append(_T(" -hide_banner -i "));
	rval.Append(QuoteFileName(m_csPathName));
	#ifdef _DEBUG
	rval.Append(_T(" -report"));
	#endif
	if (0 == GetContentType().Compare(_T("video/x-tivo-mpeg")))
	{
		//rval.Append(_T(" -map 0:v -map 0:a?")); // copy all audio streams Added 2020-04-04
		rval.Append(_T(" -map_metadata -1"));
		if (m_VideoCompatible)
			rval.Append(_T(" -vcodec copy"));
		else
		{
			// quick and dirty addition to support subtitles
			//rval.Append(_T(" -vf subtitles="));
			//rval.Append(QuoteFileName(m_csPathName));
			rval.Append(_T(" -vcodec mpeg2video"));

			if ((m_VideoWidth > 1920) || (m_VideoHeight > 1080))
				rval.Append(_T(" -s 1920x1080"));
		}
		rval.Append(_T(" -b:v 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000"));
		if (m_AudioCompatible)
			rval.Append(_T(" -acodec copy"));
		else
			rval.Append(_T(" -acodec ac3"));

		rval.Append(_T(" -f vob -"));
	}
	else
	{
		//rval.Append(_T(" -map 0:v -map 0:a?")); // copy all audio streams Added 2020-04-04
		rval.Append(_T(" -map_metadata -1"));
		if (m_VideoCompatible)
			rval.Append(_T(" -vcodec copy"));
		else
		{
			// quick and dirty addition to support subtitles
			//rval.Append(_T(" -vf subtitles="));
			//rval.Append(QuoteFileName(m_csPathName));
			//rval.Append(_T(" -vcodec mpeg2video"));
			//rval.Append(_T(" -vcodec libx264 -coder 0 -level 41 -g 250 -subq 6 -me_range 16 -qmin 10 -qmax 50 -bufsize 14000k -b:v 2500k -maxrate 10000k -trellis 2 -mbd 1")); // 2023-05-20
			rval.Append(_T(" -vcodec libx264")); // 2023-05-20

			if ((m_VideoWidth > 1920) || (m_VideoHeight > 1080))
				rval.Append(_T(" -s 1920x1080"));
		}
		rval.Append(_T(" -b:v 16384k -maxrate 30000k -bufsize 4096k -ab 448k -ar 48000"));
		if (m_AudioCompatible)
			rval.Append(_T(" -acodec copy"));
		else
			rval.Append(_T(" -acodec ac3"));

		//rval.Append(_T(" -map 0")); // attempt to copy all streams 2023-05-20

		rval.Append(_T(" -f mpegts -")); // 2023-05-20
	}

	//rval.Append(_T(" -f mpegts -")); // use mpegts as output Added 2020-04-04
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
// Simple Sorting Routines
bool cTiVoFileCompareDate(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_CaptureDate < b.m_CaptureDate); }
bool cTiVoFileCompareDateReverse(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_CaptureDate > b.m_CaptureDate); }
bool cTiVoFileComparePath(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_csPathName.CompareNoCase(b.m_csPathName) > 0); }
bool cTiVoFileComparePathReverse(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_csPathName.CompareNoCase(b.m_csPathName) < 0); }
bool cTiVoFileCompareSize(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_SourceSize > b.m_SourceSize); }
bool cTiVoFileCompareSizeReverse(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_SourceSize < b.m_SourceSize); }
bool cTiVoFileCompareTitle(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_Title.CompareNoCase(b.m_Title) < 0); }
bool cTiVoFileCompareTitleReverse(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_Title.CompareNoCase(b.m_Title) > 0); }
/////////////////////////////////////////////////////////////////////////////
bool XML_Parse_TiVoNowPlaying(CComPtr<IStream> &spStream, const CString & csMAK, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers)
{
	bool rval = false;
	HRESULT hr = S_OK;
	CComPtr<IXmlReader> pReader; 
	if (SUCCEEDED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL))) 
	{
		if (SUCCEEDED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) 
		{
			if (SUCCEEDED(hr = pReader->SetInput(spStream))) 
			{
				int indentlevel = 0;
				XmlNodeType nodeType; 
				const WCHAR* pwszLocalName; 
				const WCHAR* pwszValue; 
				bool bIsItem = false;
				bool bIsItemDetails = false;
				bool bIsItemLinks = false;
				bool bIsItemLinksContent = false;
				bool bIsItemVideo = false;
				bool bCopyProtected = false;
				bool bIsItemVideoContainer = false;
				const CString ccsItem(_T("Item"));
				const CString ccsTitle(_T("Title"));
				const CString ccsDuration(_T("Duration"));
				const CString ccsSourceSize(_T("SourceSize"));
				const CString ccsCaptureDate(_T("CaptureDate"));
				const CString ccsEpisodeTitle(_T("EpisodeTitle"));
				const CString ccsDescription(_T("Description"));
				const CString ccsSourceStation(_T("SourceStation"));
				const CString ccsCopyProtected(_T("CopyProtected"));
				const CString ccsUrl(_T("Url"));
				const CString ccsContentType(_T("ContentType"));
				const CString ccsContentTypeVideo(_T("video/x-tivo-raw-tts"));
				const CString ccsContentTypeVideoAlternate(_T("video/x-tivo-mpeg"));
				const CString ccsContentTypeContainer(_T("x-tivo-container/tivo-videos"));
				const CString ccsContentTypeContainerAlternate(_T("x-container/tivo-videos"));
				const CString ccsDetails(_T("Details"));
				const CString ccsLinks(_T("Links"));
				const CString ccsContent(_T("Content"));
				CString csTitle;
				CString csEpisodeTitle;
				CString csDescription;
				CString csSourceStation;
				CString csContentURL;
				CString csContentType;
				CTime ctCaptureDate;
				CTimeSpan ctsDuration;
				unsigned long long llSourceSize = 0;
				//read until there are no more nodes 
				while (S_OK == (hr = pReader->Read(&nodeType))) 
				{ 
					if (nodeType == XmlNodeType_Element)
					{
						if (SUCCEEDED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
						{
							if (!ccsItem.Compare(pwszLocalName))
							{
								// re-initialize stuff when starting the Item
								bIsItem = true;
								bIsItemDetails = false;
								bIsItemLinks = false;
								bIsItemLinksContent = false;
								bIsItemVideo = false;
								bCopyProtected = false;
								bIsItemVideoContainer = false;
								csTitle.Empty();
								csEpisodeTitle.Empty();
								csDescription.Empty();
								csSourceStation.Empty();
								csContentURL.Empty();
								csContentType.Empty();
							}
							else if (bIsItem && !ccsDetails.Compare(pwszLocalName))
								bIsItemDetails = true;
							else if (bIsItem && !ccsLinks.Compare(pwszLocalName))
								bIsItemLinks = true;
							else if (bIsItem && bIsItemLinks && !ccsContent.Compare(pwszLocalName))
								bIsItemLinksContent = true;
							// Here's where I need to dig deeper.
							else if (bIsItemDetails)
							{
								if (SUCCEEDED(hr = pReader->Read(&nodeType)))
								{
									if (nodeType == XmlNodeType_Text)
									{
										if (SUCCEEDED(hr = pReader->GetValue(&pwszValue, NULL)))
										{
											if (!ccsContentType.Compare(pwszLocalName))
											{
												csContentType = pwszValue;
												if (!ccsContentTypeVideo.CompareNoCase(csContentType))
													bIsItemVideo = true;
												else if (!ccsContentTypeVideoAlternate.CompareNoCase(csContentType))
													bIsItemVideo = true;
												else if (!ccsContentTypeContainer.Compare(csContentType))
													bIsItemVideoContainer = true;
												else if (!ccsContentTypeContainerAlternate.Compare(csContentType))
													bIsItemVideoContainer = true;
											}
											else if (!ccsTitle.Compare(pwszLocalName))
												csTitle = pwszValue;
											else if (!ccsEpisodeTitle.Compare(pwszLocalName))
												csEpisodeTitle = pwszValue;
											else if (!ccsDescription.Compare(pwszLocalName))
												csDescription = pwszValue;
											else if (!ccsSourceStation.Compare(pwszLocalName))
												csSourceStation = pwszValue;
											else if (!ccsCopyProtected.Compare(pwszLocalName))
												bCopyProtected = CString(pwszValue).CompareNoCase(_T("yes")) == 0;
											else if (!ccsDuration.Compare(pwszLocalName))
											{
												auto tempDuration = _wtoi(pwszValue);
												ctsDuration = CTimeSpan(0,0,0,tempDuration/1000);
											}
											else if (!ccsSourceSize.Compare(pwszLocalName))
											{
												std::wstringstream ss;
												ss << pwszValue;
												ss >> llSourceSize;
											}
											else if (!ccsCaptureDate.Compare(pwszLocalName))
											{
												std::wstringstream ss;
												ss << pwszValue;
												ss << showbase << hex;
												long long SecondsSinceEpoch;
												ss >> SecondsSinceEpoch;
												long SecondsFromUTC;
												_get_timezone(&SecondsFromUTC);
												ctCaptureDate = CTime(1970,1,1,0,0,0) + CTimeSpan(0,0,0,SecondsSinceEpoch) - CTimeSpan(SecondsFromUTC);
											}
										}
									}
								}
							}
							else if (bIsItemLinksContent)
							{
								if (SUCCEEDED(hr = pReader->Read(&nodeType)))
									if (nodeType == XmlNodeType_Text)
										if (SUCCEEDED(hr = pReader->GetValue(&pwszValue, NULL)))
											if (!ccsUrl.Compare(pwszLocalName))
												csContentURL = pwszValue;
											else if (!ccsContentType.Compare(pwszLocalName))
												if (!ccsContentTypeContainer.Compare(pwszValue) || !ccsContentTypeContainerAlternate.Compare(pwszValue))
													bIsItemVideoContainer = true;
							}
						}
					}
					else if (nodeType == XmlNodeType_EndElement)
					{
						if (SUCCEEDED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
						{
							if (!ccsItem.Compare(pwszLocalName))
							{
								bIsItem = false;
								if (bIsItemVideo && !bCopyProtected)
								{
									#ifdef _DEBUG
									// Output stuff when ending the item
									std::wcout << L"[                   ]";
									std::wcout << L" " << csTitle.GetString();
									std::wcout << L" " << csEpisodeTitle.GetString();
									std::wcout << L" " << csDescription.GetString();
									std::wcout << L" " << csSourceStation.GetString();
									std::wcout << L" " << csContentURL.GetString();
									std::wcout << L" " << ctCaptureDate.Format(_T("%c")).GetString();
									std::wcout << L" " << ctsDuration.Format(_T("%H:%M:%S")).GetString();
									std::wcout << endl;
									#endif
									cTiVoFile MyFile;
									MyFile.SetFromTiVoItem(csTitle, csEpisodeTitle, csDescription, csSourceStation, csContentURL, ctCaptureDate, ctsDuration, csMAK, llSourceSize);
									// Quickly scan list to make sure I'm not adding duplicates into list
									bool bNotInList = true;
									for (auto TiVoFile = TiVoFileList.begin(); TiVoFile != TiVoFileList.end(); TiVoFile++)
										if (MyFile == *TiVoFile)
										{
											bNotInList = false;
											break;
										}
									if (bNotInList)
										TiVoFileList.push_back(MyFile);
									rval = true;
								}
								else if (bIsItemVideoContainer)
								{
									CTiVoContainer myContainer;
									myContainer.m_title = CStringA(csTitle).GetString();
									myContainer.m_url = CStringA(csContentURL).GetString();
									myContainer.m_MAK = CStringA(csMAK).GetString();
									myContainer.m_ContentType = CStringA(csContentType).GetString();
									//myContainer.m_SourceFormat;
									// Only add the container if it doesn't already exist
									auto pContainer = std::find(TiVoTiVoContainers.begin(), TiVoTiVoContainers.end(), myContainer);
									if (pContainer == TiVoTiVoContainers.end())
										TiVoTiVoContainers.push_back(myContainer);
									rval = true;
								}
							}
							else if (bIsItemDetails && !ccsDetails.Compare(pwszLocalName))
								bIsItemDetails = false;
							else if (bIsItemLinks && !ccsLinks.Compare(pwszLocalName))
								bIsItemLinks = false;
							else if (bIsItemLinksContent && !ccsContent.Compare(pwszLocalName))
								bIsItemLinksContent = false;
						}
					}
				}
			} 
		}
	}
	return(rval);
}
bool XML_Parse_TiVoNowPlaying(const CString & Source, const CString & csMAK, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers)
{
	bool rval = false;
	CComPtr<IStream> spFileStream;
	if (SUCCEEDED(SHCreateStreamOnFile(Source.GetString(), STGM_READ, &spFileStream)))
		rval = XML_Parse_TiVoNowPlaying(spFileStream, csMAK, TiVoFileList, TiVoTiVoContainers);
	return(rval);
}
bool XML_Parse_TiVoNowPlaying(const CString & Source, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers, CInternetSession & serverSession)
{
	bool rval = false;
	std::cout << "[" << getTimeISO8601() << "] Attempting: " << CStringA(Source).GetString() << endl;
	DWORD dwServiceType;
	CString strServer;
	CString strObject;
	INTERNET_PORT nPort;
	CString strUsername;
	CString strPassword;
	AfxParseURLEx(Source.GetString(), dwServiceType, strServer, strObject, nPort, strUsername, strPassword);
	std::unique_ptr<CHttpConnection> serverConnection(serverSession.GetHttpConnection(strServer, nPort, strUsername, strPassword));
	if (NULL != serverConnection)
	{
		DWORD dwFlags = INTERNET_FLAG_TRANSFER_BINARY | SECURITY_IGNORE_ERROR_MASK;
		if (dwServiceType == AFX_INET_SERVICE_HTTPS)
			dwFlags |= INTERNET_FLAG_SECURE;
		auto FileListSizeBefore = TiVoFileList.size();
		do {
			FileListSizeBefore = TiVoFileList.size();
			std::unique_ptr<CHttpFile> serverFile(serverConnection->OpenRequest(1, strObject, NULL, 1, NULL, NULL, dwFlags));
			if (serverFile != NULL)
			{
				int BadCertErrorCount = 0;
			TiVoNowPlayingAGAIN:
				try
				{
					//std::unique_ptr<CHttpFile> serverFile((CHttpFile*) serverSession.OpenURL(*Source, 1, INTERNET_FLAG_TRANSFER_BINARY));
					serverFile->SendRequest();
					DWORD dwRet;
					serverFile->QueryInfoStatusCode(dwRet);
#ifdef _DEBUG
					//std::wcout << L"[                   ] Server Status Code: " << dwRet << endl;
					CString headers;
					serverFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, headers);
					headers.Trim();
					headers.Replace(_T("\r\n"), _T("\r\n[                   ] HTTP_QUERY_RAW_HEADERS_CRLF: "));
					std::wcout << L"[                   ] HTTP_QUERY_RAW_HEADERS_CRLF: " << headers.GetString() << endl;
#endif
					if (dwRet == HTTP_STATUS_OK)
					{
						CString csCookie;
						serverFile->QueryInfo(HTTP_QUERY_SET_COOKIE, csCookie);
						if (!csCookie.IsEmpty())
						{
							std::wcout << L"[                   ] HTTP_QUERY_SET_COOKIE: " << csCookie.GetString() << endl;
							// sid=1BFA53E13BDF178B; path=/; expires="Saturday, 16-Feb-2013 00:00:00 GMT";
							CString csCookieName(csCookie.Left(csCookie.Find(_T("="))));
							CString csCookieData(csCookie.Left(csCookie.Find(_T(";"))));;
							csCookieData.Delete(0, csCookieData.Find(_T("=")) + 1);
							CString csCookiePath(csCookie);
							csCookiePath.Delete(0, csCookiePath.Find(_T("path=")) + 5);
							csCookiePath.Delete(csCookiePath.Find(_T(";")), csCookiePath.GetLength());
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
							serverSession.SetCookie(csCookieURL, csCookieName, csCookieData);
						}
						CString csContentType;
						serverFile->QueryInfo(HTTP_QUERY_CONTENT_TYPE, csContentType);
						if (0 < csContentType.Find(_T(";"))) // Fix issue of text/xml; charset=UTF-8
							csContentType.Delete(csContentType.Find(_T(";")), csContentType.GetLength());
						if (!csContentType.CompareNoCase(_T("text/xml")))
						{
							CComPtr<IStream> spMemoryStreamOne(::SHCreateMemStream(NULL, 0));
							if (spMemoryStreamOne != NULL)
							{
								char ittybittybuffer;
								ULONG cbWritten;
								while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
									spMemoryStreamOne->Write(&ittybittybuffer, 1, &cbWritten);

								// reposition back to beginning of stream
								LARGE_INTEGER position;
								position.QuadPart = 0;
								spMemoryStreamOne->Seek(position, STREAM_SEEK_SET, NULL);
								rval = XML_Parse_TiVoNowPlaying(spMemoryStreamOne, strPassword, TiVoFileList, TiVoTiVoContainers);
								#ifdef _DEBUG
								CComPtr<IStream> spMemoryStreamTwo;
								static int iFileCount = 0;
								TCHAR szLogFilePath[MAX_PATH] = _T("");
								SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szLogFilePath);
								PathAppend(szLogFilePath, AfxGetAppName());
								CString csFileName(szLogFilePath);
								csFileName.AppendFormat(_T(".%d.xml"), iFileCount++);
								if (SUCCEEDED(SHCreateStreamOnFile(csFileName.GetString(), STGM_CREATE | STGM_WRITE, &spMemoryStreamTwo))) 
								{
									STATSTG statstg;
									spMemoryStreamOne->Stat(&statstg, STATFLAG_NONAME);
									spMemoryStreamOne->Seek(position, STREAM_SEEK_SET, NULL);
									spMemoryStreamOne->CopyTo(spMemoryStreamTwo, statstg.cbSize, NULL, NULL);
								}
								#endif
							}
						}
						else
						{
							std::cout << "[                   ] not text/xml" << endl;
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
							rval = false;
						}
					}
					else if (serverFile->GetLength() > 0)
					{
						char ittybittybuffer;
						std::string ss;
						while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
							ss += ittybittybuffer;
						std::cout << "[                   ] Returned File: " << ss << endl;
						TRACE("[                   ] Returned File: %s\n", ss.c_str());
					}
				}
				catch (CInternetException *e)
				{
					TCHAR   szCause[255];
					e->GetErrorMessage(szCause, sizeof(szCause) / sizeof(TCHAR));
					CStringA csErrorMessage(szCause);
					csErrorMessage.Trim();
					std::stringstream ss;
					ss << "[                   ] InternetException: " << csErrorMessage.GetString() << " (" << e->m_dwError << ") " << std::endl;
					std::cout << ss.str();
					TRACE(ss.str().c_str());
					if ((e->m_dwError == ERROR_INTERNET_INVALID_CA) ||
						(e->m_dwError == ERROR_INTERNET_SEC_CERT_CN_INVALID) ||
						(e->m_dwError == ERROR_INTERNET_SEC_CERT_DATE_INVALID) ||
						(e->m_dwError == ERROR_INTERNET_SEC_INVALID_CERT))
					{
						serverFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS, SECURITY_SET_MASK);
						if (BadCertErrorCount++ < 2)
							goto TiVoNowPlayingAGAIN;
					}
				}
				serverFile->Close();
				//TODO: This is where I need to update strObject with the AnchorItem= of the last element in our current file list.
				if (strObject.Find(_T("&AnchorItem=")) > 0)
					strObject.Truncate(strObject.Find(_T("&AnchorItem=")));
				if (!TiVoFileList.empty())
				{
					TCHAR lpszBuffer[_MAX_PATH];
					DWORD dwBufferLength = sizeof(lpszBuffer) / sizeof(TCHAR);
					InternetCanonicalizeUrl(TiVoFileList.back().GetURL().GetString(), lpszBuffer, &dwBufferLength, ICU_ENCODE_PERCENT);
					CString csAnchorItem(lpszBuffer, dwBufferLength);
					csAnchorItem.Replace(_T(":"), _T("%3A"));
					csAnchorItem.Replace(_T("/"), _T("%2F"));
					csAnchorItem.Replace(_T("&"), _T("%26"));
					csAnchorItem.Replace(_T("?"), _T("%3F"));
					strObject.Append(_T("&AnchorItem="));
					strObject.Append(csAnchorItem);
					strObject.Append(_T("&AnchorOffset=-1"));
				}
			}
		} while (FileListSizeBefore != TiVoFileList.size());
		serverConnection->Close();
	}
	return(rval);
}
bool GetTiVoFile(const cTiVoFile & TiVoFile, CInternetSession & serverSession, const CString & csTiVoMAK, const CString & csFileLocation)
{
	bool rval = true;
	std::cout << "[" << getTimeISO8601() << "] Attempting: " << CStringA(TiVoFile.GetURL()).GetString() << endl;
	DWORD dwServiceType;
	CString strServer;
	CString strObject; 
	INTERNET_PORT nPort; 
	CString strUsername; 
	CString strPassword; 
	AfxParseURLEx(TiVoFile.GetURL().GetString(), dwServiceType, strServer, strObject, nPort, strUsername, strPassword);
	strUsername = _T("tivo");
	strPassword = csTiVoMAK;
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
					if (!csContentType.CompareNoCase(_T("video/x-tivo-mpeg")))
					{
						//std::wcout << L"[                   ] Duration: " << TiVoFile
						std::wcout << L"[                   ] Writing File: " << TiVoFile.GetPathName().GetString() << endl;
						CString csTransferFileName(csFileLocation);
						csTransferFileName.Append(TiVoFile.GetPathName());
						std::ofstream OutputFile(csTransferFileName, ios_base::binary);
						if (OutputFile.is_open())
						{
							const size_t ReadWriteBufferSize = 1024*10;
							char ReadWriteBuffer[ReadWriteBufferSize];
							long long TotalRead = 0;
							UINT uiRead;
							CTime ctCurrent(CTime::GetCurrentTime());
							CTime ctStart(ctCurrent);
							CTime ctLastOutput(ctStart);
							CTimeSpan ctsTotal = ctCurrent - ctStart;
							std::locale mylocale("");   // get global locale
							std::locale OriginalLocale(std::cout.imbue(mylocale));  // imbue global locale
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
							std::cout << "[" << getTimeISO8601() << "] Elapsed: " << CStringA(ctsTotal.Format(_T("%H:%M:%S"))).GetString() << " Bytes: " << TotalRead << " bytes/second: " <<  (TotalRead / ctsTotal.GetTotalSeconds()) << " ReadWriteBufferSize: " << ReadWriteBufferSize << endl;
							std::cout.imbue(OriginalLocale);
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
						rval = false;
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
					rval = false;
				}
			}
		}
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
const std::string DereferenceURL(const std::string & URL, const std::string & URLParent)
{
	std::stringstream ss;
	if (URL.substr(0,4) == "http")
		ss << URL;
	else
	{
		TCHAR szScheme[_MAX_PATH] = _T("");
		DWORD dwSchemeLength = sizeof(szScheme) / sizeof(TCHAR);
		TCHAR szHostName[_MAX_PATH] = _T("");
		DWORD dwHostNameLength = sizeof(szHostName) / sizeof(TCHAR);
		TCHAR szUserName[_MAX_PATH] = _T("");
		DWORD dwUserNameLength = sizeof(szUserName) / sizeof(TCHAR);
		TCHAR szPassword[_MAX_PATH] = _T("");
		DWORD dwPasswordLength = sizeof(szPassword) / sizeof(TCHAR);
		TCHAR szUrlPath[_MAX_PATH] = _T("");
		DWORD dwUrlPathLength = sizeof(szUrlPath) / sizeof(TCHAR);
		TCHAR szExtraInfo[_MAX_PATH] = _T("");
		DWORD dwExtraInfoLength = sizeof(szExtraInfo) / sizeof(TCHAR);		
		URL_COMPONENTS crackedURL;
		crackedURL.dwStructSize = sizeof(URL_COMPONENTS);
		crackedURL.lpszScheme = szScheme;					// pointer to scheme name
		crackedURL.dwSchemeLength = dwSchemeLength;			// length of scheme name
		crackedURL.nScheme;									// enumerated scheme type (if known)
		crackedURL.lpszHostName = szHostName;				// pointer to host name
		crackedURL.dwHostNameLength = dwHostNameLength;		// length of host name
		crackedURL.nPort;									// converted port number
		crackedURL.lpszUserName = szUserName;				// pointer to user name
		crackedURL.dwUserNameLength = dwUserNameLength;		// length of user name
		crackedURL.lpszPassword = szPassword;				// pointer to password
		crackedURL.dwPasswordLength = dwPasswordLength;		// length of password
		crackedURL.lpszUrlPath = szUrlPath;					// pointer to URL-path
		crackedURL.dwUrlPathLength = dwUrlPathLength;		// length of URL-path
		crackedURL.lpszExtraInfo = szExtraInfo;				// pointer to extra information (e.g. ?foo or #foo)
		crackedURL.dwExtraInfoLength = dwExtraInfoLength;	// length of extra information
		if (TRUE == InternetCrackUrl(CString(URLParent.c_str()).GetString(), CString(URLParent.c_str()).GetLength(), ICU_DECODE, &crackedURL))
		{
			ss << CStringA(crackedURL.lpszScheme).GetString() << "://";
			if (!CStringA(crackedURL.lpszUserName).IsEmpty()) ss << CStringA(crackedURL.lpszUserName).GetString() << ":" << CStringA(crackedURL.lpszPassword).GetString() << "@";
			ss << CStringA(crackedURL.lpszHostName).GetString() << ":" << crackedURL.nPort;
			ss << URL;
			//szUrlPath[0] = _T('');
			//szExtraInfo[0] = _T('');
			//BOOL Crack2 = InternetCrackUrl(CString(URL.c_str()).GetString(), CString(URL.c_str()).GetLength(), ICU_DECODE, &crackedURL);
			//TCHAR rVal[512] = _T("");
			//DWORD rValSize = sizeof(rVal) / sizeof(TCHAR);
			//if (FALSE == Crack2)
			//	Crack2 = InternetCombineUrl(CString(URLParent.c_str()).GetString(), CString(URL.c_str()).GetString(), rVal, &rValSize, 0);
			//else
			//	Crack2 = InternetCreateUrl(&crackedURL, ICU_ESCAPE, rVal, &rValSize);
			//return(std::string(CStringA(rVal).GetString()));
		}
	}
	return(ss.str());
}
/////////////////////////////////////////////////////////////////////////////
#ifdef CACHE_FILE
void cTiVoFile::WriteToCache(CComPtr<IXmlWriter>& pWriter)
{
	std::wstringstream ss;

	pWriter->WriteStartElement(NULL, L"cTiVoFile", NULL);

	ss << m_AudioCompatible; pWriter->WriteElementString(NULL, L"m_AudioCompatible", NULL, ss.str().c_str()); ss = std::wstringstream();
	pWriter->WriteElementString(NULL, L"m_CaptureDate", NULL, m_CaptureDate.FormatGmt(_T("%Y%m%dT%H%M%SZ")).GetString());
	pWriter->WriteElementString(NULL, L"m_ContentType", NULL, m_ContentType.GetString());
	pWriter->WriteElementString(NULL, L"m_csMAK", NULL, m_csMAK.GetString());
	pWriter->WriteElementString(NULL, L"m_csPathName", NULL, m_csPathName.GetString());
	pWriter->WriteElementString(NULL, L"m_Description", NULL, m_Description.GetString());
	ss << m_Duration; pWriter->WriteElementString(NULL, L"m_Duration", NULL, ss.str().c_str()); ss = std::wstringstream();
	pWriter->WriteElementString(NULL, L"m_EpisodeTitle", NULL, m_EpisodeTitle.GetString());
	pWriter->WriteElementString(NULL, L"m_LastWriteTime", NULL, m_LastWriteTime.FormatGmt(_T("%Y%m%dT%H%M%SZ")).GetString());
	pWriter->WriteElementString(NULL, L"m_SourceChannel", NULL, m_SourceChannel.GetString());
	pWriter->WriteElementString(NULL, L"m_SourceFormat", NULL, m_SourceFormat.GetString());
	ss << m_SourceSize; pWriter->WriteElementString(NULL, L"m_SourceSize", NULL, ss.str().c_str()); ss = std::wstringstream();
	pWriter->WriteElementString(NULL, L"m_SourceStation", NULL, m_SourceStation.GetString());
	pWriter->WriteElementString(NULL, L"m_Title", NULL, m_Title.GetString());
	pWriter->WriteElementString(NULL, L"m_TvBusEnvelope", NULL, m_TvBusEnvelope.GetString());
	pWriter->WriteElementString(NULL, L"m_vActor", NULL, m_vActor.GetString());
	ss << m_VideoCompatible; pWriter->WriteElementString(NULL, L"m_VideoCompatible", NULL, ss.str().c_str()); ss = std::wstringstream();
	ss << m_VideoHeight; pWriter->WriteElementString(NULL, L"m_VideoHeight", NULL, ss.str().c_str()); ss = std::wstringstream();
	ss << m_VideoHighDefinition; pWriter->WriteElementString(NULL, L"m_VideoHighDefinition", NULL, ss.str().c_str()); ss = std::wstringstream();
	ss << m_VideoWidth; pWriter->WriteElementString(NULL, L"m_VideoWidth", NULL, ss.str().c_str()); ss = std::wstringstream();
	pWriter->WriteElementString(NULL, L"m_vProgramGenre", NULL, m_vProgramGenre.GetString());

	pWriter->WriteEndElement();	// cTiVoFile
}

bool cTiVoFile::PopulateFromXML(CComPtr<IXmlReader> &pReader, CComPtr<IStream>& spStream)
{
	HRESULT hr = S_OK;
	XmlNodeType nodeType;
	//read until there are no more nodes 
	while (S_OK == (hr = pReader->Read(&nodeType)))
	{
		if (nodeType == XmlNodeType_Element)
		{
			const WCHAR* pwszLocalName;
			const WCHAR* pwszValue;
			if (SUCCEEDED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
			{
				if (!ccsItem.Compare(pwszLocalName))
				{
					// re-initialize stuff when starting the Item
					bIsItem = true;
					bIsItemDetails = false;
					bIsItemLinks = false;
					bIsItemLinksContent = false;
					bIsItemVideo = false;
					bCopyProtected = false;
					bIsItemVideoContainer = false;
					csTitle.Empty();
					csEpisodeTitle.Empty();
					csDescription.Empty();
					csSourceStation.Empty();
					csContentURL.Empty();
					csContentType.Empty();
				}
				else if (bIsItem && !ccsDetails.Compare(pwszLocalName))
					bIsItemDetails = true;
				else if (bIsItem && !ccsLinks.Compare(pwszLocalName))
					bIsItemLinks = true;
				else if (bIsItem && bIsItemLinks && !ccsContent.Compare(pwszLocalName))
					bIsItemLinksContent = true;
				// Here's where I need to dig deeper.
				else if (bIsItemDetails)
				{
					if (SUCCEEDED(hr = pReader->Read(&nodeType)))
					{
						if (nodeType == XmlNodeType_Text)
						{
							if (SUCCEEDED(hr = pReader->GetValue(&pwszValue, NULL)))
							{
								if (!ccsContentType.Compare(pwszLocalName))
								{
									csContentType = pwszValue;
									if (!ccsContentTypeVideo.CompareNoCase(csContentType))
										bIsItemVideo = true;
									else if (!ccsContentTypeVideoAlternate.CompareNoCase(csContentType))
										bIsItemVideo = true;
									else if (!ccsContentTypeContainer.Compare(csContentType))
										bIsItemVideoContainer = true;
									else if (!ccsContentTypeContainerAlternate.Compare(csContentType))
										bIsItemVideoContainer = true;
								}
								else if (!ccsTitle.Compare(pwszLocalName))
									csTitle = pwszValue;
								else if (!ccsEpisodeTitle.Compare(pwszLocalName))
									csEpisodeTitle = pwszValue;
								else if (!ccsDescription.Compare(pwszLocalName))
									csDescription = pwszValue;
								else if (!ccsSourceStation.Compare(pwszLocalName))
									csSourceStation = pwszValue;
								else if (!ccsCopyProtected.Compare(pwszLocalName))
									bCopyProtected = CString(pwszValue).CompareNoCase(_T("yes")) == 0;
								else if (!ccsDuration.Compare(pwszLocalName))
								{
									auto tempDuration = _wtoi(pwszValue);
									ctsDuration = CTimeSpan(0, 0, 0, tempDuration / 1000);
								}
								else if (!ccsSourceSize.Compare(pwszLocalName))
								{
									std::wstringstream ss;
									ss << pwszValue;
									ss >> llSourceSize;
								}
								else if (!ccsCaptureDate.Compare(pwszLocalName))
								{
									std::wstringstream ss;
									ss << pwszValue;
									ss << showbase << hex;
									long long SecondsSinceEpoch;
									ss >> SecondsSinceEpoch;
									long SecondsFromUTC;
									_get_timezone(&SecondsFromUTC);
									ctCaptureDate = CTime(1970, 1, 1, 0, 0, 0) + CTimeSpan(0, 0, 0, SecondsSinceEpoch) - CTimeSpan(SecondsFromUTC);
								}
							}
						}
					}
				}
				else if (bIsItemLinksContent)
				{
					if (SUCCEEDED(hr = pReader->Read(&nodeType)))
						if (nodeType == XmlNodeType_Text)
							if (SUCCEEDED(hr = pReader->GetValue(&pwszValue, NULL)))
								if (!ccsUrl.Compare(pwszLocalName))
									csContentURL = pwszValue;
								else if (!ccsContentType.Compare(pwszLocalName))
									if (!ccsContentTypeContainer.Compare(pwszValue) || !ccsContentTypeContainerAlternate.Compare(pwszValue))
										bIsItemVideoContainer = true;
				}
			}
		}
		else if (nodeType == XmlNodeType_EndElement)
		{
			break;
		}
	}
	return false;
}

bool XML_Parse_CacheFile(CComPtr<IStream>& spStream, std::vector<cTiVoFile>& TiVoFileList)
{

}
#endif // CACHE_FILE
