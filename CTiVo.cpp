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
bool cTiVoServer::operator==(const cTiVoServer & other) const
{
	return(
		//(m_address == other.m_address) && 
		//(m_swversion == other.m_swversion) && 
		//(m_method == other.m_method) && 
		//(m_identity == other.m_identity) && 
		//(m_machine == other.m_machine) && 
		//(m_platform == other.m_platform) && 
		//(m_services == other.m_services)
		m_machine == other.m_machine
		);
}
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
		TCHAR path_buffer[_MAX_PATH];
		TCHAR drive[_MAX_DRIVE];
		TCHAR dir[_MAX_DIR];
		TCHAR fname[_MAX_FNAME];
		TCHAR ext[_MAX_EXT];
		_tsplitpath_s(m_csPathName.GetString(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		//wcout << L"[                   ] " << setw(20) << right << L"Drive" << L" : " << drive << endl;
		//wcout << L"[                   ] " << setw(20) << right << L"dir" << L" : " << dir << endl;
		//wcout << L"[                   ] " << setw(20) << right << L"fname" << L" : " << fname << endl;
		//wcout << L"[                   ] " << setw(20) << right << L"ext" << L" : " << ext << endl;
		m_csURL = csUrlPrefix;
		_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fname, ext);
		m_csURL.Append(path_buffer);
		m_csURL.Replace(_T("\\"), _T("/"));
		// see WinHttpCreateUrl for useful info http://msdn.microsoft.com/en-us/library/windows/desktop/aa384093(v=vs.85).aspx
		TCHAR lpszBuffer[_MAX_PATH];
		DWORD dwBufferLength = sizeof(lpszBuffer) / sizeof(TCHAR);
		InternetCanonicalizeUrl(m_csURL.GetString(), lpszBuffer, &dwBufferLength, 0);
		m_csURL = CString(lpszBuffer, dwBufferLength);
		if (m_Title.IsEmpty())
			m_Title = fname;
	}
	m_ContentType = _T("video/x-tivo-mpeg");
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
	wcout << L"[                   ] " << setw(20) << right << L"URL" << L" : " << m_csURL.GetString() << endl;
}	
void cTiVoFile::SetPathName(const CFileFind & csNewPath)
{
	m_csPathName = csNewPath.GetFilePath();
	m_SourceSize = csNewPath.GetLength();
	m_Title = csNewPath.GetFileTitle();
	m_ContentType = _T("video/x-tivo-mpeg");		
	csNewPath.GetLastWriteTime(m_CaptureDate);

	//m_csURL = csNewPath.GetFileURL();
	m_csURL = csUrlPrefix;
	m_csURL.Append(m_csPathName);
	m_csURL.Replace(_T("\\"), _T("/"));
	TCHAR lpszBuffer[_MAX_PATH];
	DWORD dwBufferLength = sizeof(lpszBuffer) / sizeof(TCHAR);
	InternetCanonicalizeUrl(m_csURL.GetString(), lpszBuffer, &dwBufferLength, 0);
	m_csURL = CString(lpszBuffer, dwBufferLength);
	#ifdef AVCODEC_AVCODEC_H
	PopulateFromFFMPEG();
	#endif
	// Final Output of object values
	wcout << L"[                   ] " << setw(20) << right << L"m_csPathName" << L" : " << m_csPathName.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_Title" << L" : " << m_Title.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_EpisodeTitle" << L" : " << m_EpisodeTitle.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_Description" << L" : " << m_Description.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_ContentType" << L" : " << m_ContentType.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_SourceFormat" << L" : " << m_SourceFormat.GetString() << endl;
	//wcout << L"[                   ] " << setw(20) << right << L"LastChangeDate" << L" : " << LastChangeDate.GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_SourceSize" << L" : " << m_SourceSize << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_Duration" << L" : " << m_Duration << endl;
	wcout << L"[                   ] " << setw(20) << right << L"m_CaptureDate" << L" : " << m_CaptureDate.Format(_T("%c")).GetString() << endl;
	wcout << L"[                   ] " << setw(20) << right << L"URL" << L" : " << m_csURL.GetString() << endl;
}
void cTiVoFile::SetFromTiVoItem(const CString &csTitle, const CString &csEpisodeTitle, const CString &csDescription, const CString &csSourceStation, const CString &csContentURL, const CTime &ctCaptureDate, const CTimeSpan &ctsDuration, const unsigned long long llSourceSize)
{
	m_Title = csTitle;
	m_EpisodeTitle = csEpisodeTitle;
	m_Description = csDescription;
	m_csURL = csContentURL;
	m_CaptureDate = ctCaptureDate;
	m_Duration = 1000 * ctsDuration.GetTotalSeconds();
	m_SourceSize = llSourceSize;
	m_SourceStation = csSourceStation;
	std::wstringstream ssFileName(std::stringstream::in | std::stringstream::out);
	ssFileName << csTitle.GetString();
	if (!csEpisodeTitle.IsEmpty())
		ssFileName << L" - ''" << csEpisodeTitle.GetString() << L"''";
	ssFileName << L" (Recorded " << m_CaptureDate.Format(_T("%b %d, %Y, ")).GetString() << m_SourceStation.GetString() << L").TiVo";
	m_csPathName = ssFileName.str().c_str();
	m_csPathName.Replace(_T(":"),_T("_")); // http://msdn.microsoft.com/en-us/library/system.io.path.getinvalidfilenamechars.aspx should be further examined
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
			{
				m_Duration = fmt_ctx->duration + 5000;
				int secs = m_Duration / AV_TIME_BASE;
				int us = m_Duration % AV_TIME_BASE;
				int mins = secs / 60;
				secs %= 60;
				int hours = mins / 60;
				mins %= 60;
				std::cout << "[                   ] " << setw(20) << right << "m_Duration" << " : ";
				char oldfill = std::cout.fill('0');
				streamsize oldwidth = std::cout.width(2);
				std::cout << hours << ":" << mins << ":" << secs << "." << ((100 * us) / AV_TIME_BASE) << endl;
				std::cout.width(oldwidth);
				std::cout.fill(oldfill);
				m_Duration /= 1000; // this makes at least my first example match the tivo desktop software
			}
			m_SourceFormat.Append(_T("video/"));
			m_SourceFormat.Append(CString(CStringA(fmt_ctx->iformat->name)));
			//av_dump_format(fmt_ctx, 0, CStringA(m_csPathName).GetString(), 0);

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
				m_SourceFormat = CStringA(codec_type);
				m_SourceFormat.Append(_T("/"));
				m_SourceFormat.Append(CString(CStringA(codec_name)));
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
			// This next section looks at metadata
			AVDictionaryEntry *tag = NULL;
			while (tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))
			{
				//#ifdef _DEBUG
				//cout << "[                   ] " << setw(20) << right << tag->key << " : " << tag->value << endl;
				//#endif
				if (_stricmp("title", tag->key) == 0)
					m_Title = CString(tag->value);
				if (_stricmp("WM/SubTitle", tag->key) == 0)
					m_EpisodeTitle = CString(tag->value);
				if (_stricmp("WM/SubTitleDescription", tag->key) == 0)
					m_Description = CString(tag->value);
				//if (_stricmp("m_Duration", tag->key) == 0)
				//{
				//	std::stringstream ss(std::string(tag->value), std::stringstream::in | std::stringstream::out);
				//	ss >> m_Duration;
				//	// m_Duration should now be in 100-nanosecond units. The TiVo wants it in 1/1000 of a second units. http://msdn.microsoft.com/en-us/library/windows/desktop/ff384862(v=vs.85).aspx
				//	m_Duration /= 1000;
				//}
			}
		}
		avformat_close_input(&fmt_ctx);
	}
	av_log_set_level(Oldavlog);
}
#endif
void cTiVoFile::GetXML(CComPtr<IXmlWriter> & pWriter) const
{
	pWriter->WriteStartElement(NULL, L"Item", NULL);
		pWriter->WriteStartElement(NULL, L"Details", NULL);
			pWriter->WriteElementString(NULL, L"Title", NULL, m_Title.GetString());
			if (!m_EpisodeTitle.IsEmpty()) pWriter->WriteElementString(NULL, L"EpisodeTitle", NULL, m_EpisodeTitle.GetString());
			if (!m_Description.IsEmpty()) pWriter->WriteElementString(NULL, L"Description", NULL, m_Description.GetString());
			if (!m_ContentType.IsEmpty()) pWriter->WriteElementString(NULL, L"ContentType", NULL, m_ContentType.GetString());
			//if (!m_ContentType.IsEmpty()) pWriter->WriteElementString(NULL, L"SourceFormat", NULL, m_ContentType.GetString());
			if (!m_SourceFormat.IsEmpty()) pWriter->WriteElementString(NULL, L"SourceFormat", NULL, m_SourceFormat.GetString());
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
		if (!m_ContentType.IsEmpty() && !m_csURL.IsEmpty())
		{
			pWriter->WriteStartElement(NULL, L"Links", NULL);
				pWriter->WriteStartElement(NULL, L"Content", NULL);
					pWriter->WriteElementString(NULL, L"ContentType", NULL, m_ContentType.GetString());
					pWriter->WriteElementString(NULL, L"Url", NULL, m_csURL.GetString());
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
					DetailsURL.Append(m_csURL);
					pWriter->WriteElementString(NULL, L"Url", NULL, DetailsURL.GetString());
				pWriter->WriteEndElement();
			pWriter->WriteEndElement();
		}
	pWriter->WriteEndElement();	// Item
}
void cTiVoFile::GetTvBusEnvelope(CComPtr<IXmlWriter> & pWriter) const
{
	pWriter->WriteRaw(L"<TvBusMarshalledStruct:TvBusEnvelope xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:TvBusMarshalledStruct=\"http://tivo.com/developer/xml/idl/TvBusMarshalledStruct\" xmlns:TvPgdRecording=\"http://tivo.com/developer/xml/idl/TvPgdRecording\" xmlns:TvBusDuration=\"http://tivo.com/developer/xml/idl/TvBusDuration\" xmlns:TvPgdShowing=\"http://tivo.com/developer/xml/idl/TvPgdShowing\" xmlns:TvDbShowingBit=\"http://tivo.com/developer/xml/idl/TvDbShowingBit\" xmlns:TvBusDateTime=\"http://tivo.com/developer/xml/idl/TvBusDateTime\" xmlns:TvPgdProgram=\"http://tivo.com/developer/xml/idl/TvPgdProgram\" xmlns:TvDbColorCode=\"http://tivo.com/developer/xml/idl/TvDbColorCode\" xmlns:TvPgdSeries=\"http://tivo.com/developer/xml/idl/TvPgdSeries\" xmlns:TvDbShowType=\"http://tivo.com/developer/xml/idl/TvDbShowType\" xmlns:TvPgdBookmark=\"http://tivo.com/developer/xml/idl/TvPgdBookmark\" xmlns:TvPgdChannel=\"http://tivo.com/developer/xml/idl/TvPgdChannel\" xmlns:TvDbBitstreamFormat=\"http://tivo.com/developer/xml/idl/TvDbBitstreamFormat\" xs:schemaLocation=\"http://tivo.com/developer/xml/idl/TvBusMarshalledStruct TvBusMarshalledStruct.xsd http://tivo.com/developer/xml/idl/TvPgdRecording TvPgdRecording.xsd http://tivo.com/developer/xml/idl/TvBusDuration TvBusDuration.xsd http://tivo.com/developer/xml/idl/TvPgdShowing TvPgdShowing.xsd http://tivo.com/developer/xml/idl/TvDbShowingBit TvDbShowingBit.xsd http://tivo.com/developer/xml/idl/TvBusDateTime TvBusDateTime.xsd http://tivo.com/developer/xml/idl/TvPgdProgram TvPgdProgram.xsd http://tivo.com/developer/xml/idl/TvDbColorCode TvDbColorCode.xsd http://tivo.com/developer/xml/idl/TvPgdSeries TvPgdSeries.xsd http://tivo.com/developer/xml/idl/TvDbShowType TvDbShowType.xsd http://tivo.com/developer/xml/idl/TvPgdBookmark TvPgdBookmark.xsd http://tivo.com/developer/xml/idl/TvPgdChannel TvPgdChannel.xsd http://tivo.com/developer/xml/idl/TvDbBitstreamFormat TvDbBitstreamFormat.xsd\" xs:type=\"TvPgdRecording:TvPgdRecording\">");
	pWriter->WriteElementString(NULL, L"recordedDuration", NULL, CString(timeToISO8601(CTimeSpan(m_Duration/1000)).c_str()).GetString()); // This is ISO8601 Format for Duration
	pWriter->WriteStartElement(NULL, L"vActualShowing", NULL);pWriter->WriteEndElement();
	pWriter->WriteStartElement(NULL, L"vBookmark", NULL);pWriter->WriteEndElement();
	pWriter->WriteStartElement(NULL, L"recordingQuality", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"75");pWriter->WriteString(L"HIGH");pWriter->WriteEndElement();
	pWriter->WriteStartElement(NULL, L"showing", NULL);
		pWriter->WriteStartElement(NULL, L"showingBits", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"4609");pWriter->WriteEndElement();
		pWriter->WriteElementString(NULL, L"time", NULL, m_CaptureDate.FormatGmt(_T("%Y-%m-%d:%H:%M:%SZ")));
		pWriter->WriteElementString(NULL, L"duration", NULL, CString(timeToISO8601(CTimeSpan(m_Duration/1000)).c_str()).GetString());
		pWriter->WriteStartElement(NULL, L"program", NULL);
			pWriter->WriteStartElement(NULL, L"vActor", NULL);
			pWriter->WriteEndElement();				
			pWriter->WriteStartElement(NULL, L"vAdvisory", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"vChoreographer", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"colorCode", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"4");pWriter->WriteEndElement();
			pWriter->WriteElementString(NULL, L"description", NULL, m_Description.GetString());
			pWriter->WriteStartElement(NULL, L"vDirector", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteElementString(NULL, L"episodeTitle", NULL, m_EpisodeTitle.GetString());
			pWriter->WriteStartElement(NULL, L"vExecProducer", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"vProgramGenre", NULL);
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
					//pWriter->WriteElementString(NULL, L"element", NULL, L"Talk Show");
					//pWriter->WriteElementString(NULL, L"element", NULL, L"News Magazine");
					//pWriter->WriteElementString(NULL, L"element", NULL, L"News and Business");
					//pWriter->WriteElementString(NULL, L"element", NULL, L"Talk Shows");
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
/////////////////////////////////////////////////////////////////////////////
// Simple Sorting Routines
bool cTiVoFileCompareDate(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_CaptureDate > b.m_CaptureDate); }
bool cTiVoFileCompareDateReverse(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_CaptureDate < b.m_CaptureDate); }
bool cTiVoFileComparePath(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_csPathName >b.m_csPathName); }
bool cTiVoFileComparePathReverse(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_csPathName < b.m_csPathName); }
bool cTiVoFileCompareSize(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_SourceSize > b.m_SourceSize); }
bool cTiVoFileCompareSizeReverse(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_SourceSize < b.m_SourceSize); }
/////////////////////////////////////////////////////////////////////////////
void XML_Parse_TiVoNowPlaying(CComPtr<IStream> &spStream, std::vector<cTiVoFile> & TiVoFileList)
{
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
				const WCHAR* pwszPrefix; 
				const WCHAR* pwszLocalName; 
				const WCHAR* pwszValue; 
				//const WCHAR* pwsznsURI;
				UINT cwchPrefix;
				bool bIsItem = false;
				bool bIsItemDetails = false;
				bool bIsItemLinks = false;
				bool bIsItemLinksContent = false;
				bool bIsItemVideo = false;
				bool bCopyProtected = false;
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
				const CString ccsContentTypeValue(_T("video/x-tivo-raw-tts"));
				const CString ccsDetails(_T("Details"));
				const CString ccsLinks(_T("Links"));
				const CString ccsContent(_T("Content"));
				CString csTitle;
				CString csEpisodeTitle;
				CString csDescription;
				CString csSourceStation;
				CString csContentURL;
				CTime ctCaptureDate;
				CTimeSpan ctsDuration;
				unsigned long long llSourceSize;
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
								csTitle.Empty();
								csEpisodeTitle.Empty();
								csDescription.Empty();
								csSourceStation.Empty();
								csContentURL.Empty();
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
												if (!ccsContentTypeValue.CompareNoCase(pwszValue))
													bIsItemVideo = true;
												else
													bIsItemVideo = false;
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
									std::wstringstream ssFileName(std::stringstream::in | std::stringstream::out);
									ssFileName << csTitle.GetString();
									if (!csEpisodeTitle.IsEmpty())
										ssFileName << L" - ''" << csEpisodeTitle.GetString() << L"''";
									ssFileName << L" (Recorded " << ctCaptureDate.Format(_T("%b %d, %Y, ")).GetString() << csSourceStation.GetString() << L").TiVo";
									#ifdef _DEBUG
									std::wcout << L"[                   ] FileName: " << ssFileName.str().c_str() << endl;
									#endif
									cTiVoFile MyFile;
									MyFile.SetFromTiVoItem(csTitle, csEpisodeTitle, csDescription, csSourceStation, csContentURL, ctCaptureDate, ctsDuration, llSourceSize);
									TiVoFileList.push_back(MyFile);
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
}
void XML_Parse_TiVoNowPlaying(const CString & Source, std::vector<cTiVoFile> & TiVoFileList)
{
	CComPtr<IStream> spFileStream;
	if (SUCCEEDED(SHCreateStreamOnFile(Source.GetString(), STGM_READ, &spFileStream)))
		XML_Parse_TiVoNowPlaying(spFileStream, TiVoFileList);
}
bool XML_Parse_TiVoNowPlaying(const CString & Source, std::vector<cTiVoFile> & TiVoFileList, CInternetSession & serverSession)
{
	bool rval = true;
	std::cout << "[" << getTimeISO8601() << "] Attempting: " << CStringA(Source).GetString() << endl;
	DWORD dwServiceType;
	CString strServer;
	CString strObject; 
	INTERNET_PORT nPort; 
	CString strUsername; 
	CString strPassword; 
	AfxParseURLEx(Source.GetString(), dwServiceType, strServer, strObject, nPort, strUsername, strPassword);
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
							XML_Parse_TiVoNowPlaying(spMemoryStreamOne, TiVoFileList);
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
						goto TiVoNowPlayingAGAIN;
				}
			}
		}
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
