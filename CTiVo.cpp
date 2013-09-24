#include "stdafx.h"
#include "CTiVo.h"
using namespace std;

static std::string timeToISO8601(const time_t & TheTime)
{
	std::ostringstream ISOTime;
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
static std::string timeToExcelDate(const time_t & TheTime)
{
	std::ostringstream ISOTime;
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
static std::string getTimeISO8601(void)
{
	time_t timer;
	time(&timer);
	return(timeToISO8601(timer));
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
		PopulateFromFFMPEG();
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
	PopulateFromFFMPEG();
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
void cTiVoFile::SetFromTiVoItem(const CString &csTitle, const CString &csEpisodeTitle, const CString &csDescription, const CString &csSourceStation, const CString &csContentURL, const CTime &ctCaptureDate, const CTimeSpan &ctsDuration)
{
	m_Title = csTitle;
	m_EpisodeTitle = csEpisodeTitle;
	m_Description = csDescription;
	m_csURL = csContentURL;
	m_CaptureDate = ctCaptureDate;
	m_Duration = 1000 * ctsDuration.GetTotalSeconds();
	std::wstringstream ssFileName(std::stringstream::in | std::stringstream::out);
	ssFileName << L"//Acid/TiVo/";
	ssFileName << csTitle.GetString();
	if (!csEpisodeTitle.IsEmpty())
		ssFileName << L" - ''" << csEpisodeTitle.GetString() << L"''";
	ssFileName << L" (Recorded " << ctCaptureDate.Format(_T("%b %d, %Y, ")).GetString() << csSourceStation.GetString() << L").TiVo";
	m_csPathName = ssFileName.str().c_str();
}
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
void cTiVoFile::GetXML(CComPtr<IXmlWriter> & pWriter) const
{
	pWriter->WriteStartElement(NULL, L"Item", NULL);
		pWriter->WriteStartElement(NULL, L"Details", NULL);
			pWriter->WriteElementString(NULL, L"Title", NULL, m_Title.GetString());
			if (!m_EpisodeTitle.IsEmpty()) pWriter->WriteElementString(NULL, L"EpisodeTitle", NULL, m_EpisodeTitle.GetString());
			if (!m_Description.IsEmpty()) pWriter->WriteElementString(NULL, L"Description", NULL, m_Description.GetString());
			if (!m_ContentType.IsEmpty()) pWriter->WriteElementString(NULL, L"ContentType", NULL, m_ContentType.GetString());
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
				tempTime += CTimeSpan(_timezone);	// the time difference was ~8 hours, which makes sense as the diff between UTC and local time.
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
	pWriter->WriteElementString(NULL, L"recordedDuration", NULL, L"PT30M");
	pWriter->WriteStartElement(NULL, L"vActualShowing", NULL);pWriter->WriteEndElement();
	pWriter->WriteStartElement(NULL, L"vBookmark", NULL);pWriter->WriteEndElement();
	pWriter->WriteStartElement(NULL, L"recordingQuality", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"75");pWriter->WriteString(L"HIGH");pWriter->WriteEndElement();
	pWriter->WriteStartElement(NULL, L"showing", NULL);
		pWriter->WriteStartElement(NULL, L"showingBits", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"4609");pWriter->WriteEndElement();
		pWriter->WriteElementString(NULL, L"time", NULL, L"2010-03-27T02:00:00Z");
		pWriter->WriteElementString(NULL, L"duration", NULL, L"PT30M");
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
			if (!m_EpisodeTitle.IsEmpty()) pWriter->WriteElementString(NULL, L"episodeTitle", NULL, m_EpisodeTitle.GetString());
			pWriter->WriteStartElement(NULL, L"vExecProducer", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"vProgramGenre", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"vGuestStar", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"vHost", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteElementString(NULL, L"isEpisode", NULL, L"true");
			pWriter->WriteElementString(NULL, L"originalAirDate", NULL, L"1992-09-07T00:00:00Z");
			pWriter->WriteStartElement(NULL, L"vProducer", NULL);
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"series", NULL);
				pWriter->WriteElementString(NULL, L"isEpisodic", NULL, L"true");
				pWriter->WriteStartElement(NULL, L"vSeriesGenre", NULL);
					pWriter->WriteElementString(NULL, L"element", NULL, L"Talk Show");
					pWriter->WriteElementString(NULL, L"element", NULL, L"News Magazine");
					pWriter->WriteElementString(NULL, L"element", NULL, L"News and Business");
					pWriter->WriteElementString(NULL, L"element", NULL, L"Talk Shows");
				pWriter->WriteEndElement();
				pWriter->WriteElementString(NULL, L"seriesTitle", NULL, m_Title.GetString());
			pWriter->WriteEndElement();
			pWriter->WriteStartElement(NULL, L"showType", NULL);pWriter->WriteAttributeString(NULL,L"value",NULL,L"5");pWriter->WriteString(L"SERIES");pWriter->WriteEndElement();
			pWriter->WriteElementString(NULL, L"title", NULL, m_Title.GetString());
			pWriter->WriteStartElement(NULL, L"vWriter", NULL);
			pWriter->WriteEndElement();
		pWriter->WriteEndElement();
		pWriter->WriteStartElement(NULL, L"channel", NULL);
			pWriter->WriteElementString(NULL, L"displayMajorNumber", NULL, L"0");
			pWriter->WriteElementString(NULL, L"displayMinorNumber", NULL, L"0");
			pWriter->WriteElementString(NULL, L"callsign", NULL, NULL);
		pWriter->WriteEndElement();
	pWriter->WriteEndElement();
	pWriter->WriteElementString(NULL, L"startTime", NULL, L"2010-03-27T01:59:58Z");
	pWriter->WriteElementString(NULL, L"stopTime", NULL, L"2010-03-27T02:30:00Z");
}
bool cTiVoFileCompareDate(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_CaptureDate > b.m_CaptureDate); }
bool cTiVoFileCompareDateReverse(const cTiVoFile & a, const cTiVoFile & b) { return(a.m_CaptureDate < b.m_CaptureDate); }
