#pragma once

class cTiVoServer
{
//192.168.1.11  tivoconnect=1 swversion=11.0k-01-2-652 method=broadcast identity=6520201806EEAAE machine=Living Room platform=tcd/Series3 services=TiVoMediaServer:80/http
//192.168.50.43 tivoconnect=1 swversion=20.7.4d.RC15-846-6-846 method=broadcast identity=84600119023FFD0 machine=WimRomio platform=tcd/Series5 services=TiVoMediaServer:80/http
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
class CTiVoContainer
{
public:
	std::string m_title;
	std::string m_url;
	std::string m_MAK;
	std::string m_ContentType;
	std::string m_SourceFormat;
	bool operator==(const CTiVoContainer & other) const;
	std::string WriteTXT(const char seperator = '\t') const;
	bool ReadTXT(const std::string & text, const char seperator = '\t');
};
class cTiVoFile
{
private:
	CString m_csPathName;
	CString m_csURL; // only used for remote items
	CString m_Title;
	CString m_EpisodeTitle;
	int m_EpisodeNumber;
	CString m_Description;
	CString m_ContentType;
	CString m_SourceFormat;
	CString m_SourceStation;
	CString m_SourceChannel;
	CString m_vProgramGenre;
	CString m_vActor;
	unsigned long long m_SourceSize;
    unsigned long long m_Duration;
	CTime m_CaptureDate;
	CTime m_LastWriteTime;
	CString m_csMAK;
	bool m_AudioCompatible;
	bool m_VideoCompatible;
	bool m_VideoHighDefinition;
	CString m_Subtitles;
	int m_VideoWidth;
	int m_VideoHeight;
	CString m_TvBusEnvelope;
public:
	cTiVoFile() : 
	m_EpisodeNumber(0),
	m_AudioCompatible(false),
	m_VideoCompatible(false),
	m_VideoHighDefinition(false),
	m_VideoWidth(0),
	m_VideoHeight(0),
	m_SourceSize(0),
    m_Duration(0)
	{
	}
	bool operator==(const cTiVoFile & other) const;
	// Simple Sorting Routines
	friend bool cTiVoFileCompareDate(const cTiVoFile & a, const cTiVoFile & b);
	friend bool cTiVoFileCompareDateReverse(const cTiVoFile & a, const cTiVoFile & b);
	friend bool cTiVoFileComparePath(const cTiVoFile & a, const cTiVoFile & b);
	friend bool cTiVoFileComparePathReverse(const cTiVoFile & a, const cTiVoFile & b);
	friend bool cTiVoFileCompareSize(const cTiVoFile & a, const cTiVoFile & b);
	friend bool cTiVoFileCompareSizeReverse(const cTiVoFile & a, const cTiVoFile & b);
	friend bool cTiVoFileCompareTitle(const cTiVoFile & a, const cTiVoFile & b);
	friend bool cTiVoFileCompareTitleReverse(const cTiVoFile & a, const cTiVoFile & b);
	// Access Routines
	void SetPathName(const CString csNewPath);
	void SetPathName(const CFileFind & csNewPath);
	const CString & GetPathName(void) const { return(m_csPathName); }
	void SetFromTiVoItem(const CString &csTitle, const CString &csEpisodeTitle, const CString &csDescription, const CString &csSourceStation, const CString &csContentURL, const CTime &ctCaptureDate, const CTimeSpan &ctsDuration, const CString & csMAK, const unsigned long long llSourceSize = 0);
	#ifdef AVCODEC_AVCODEC_H
	void PopulateFromFFMPEG(void);
	#endif
	void PopulateFromFFProbe(void);
	const CString GetURL(void) const;
	const CString & GetTitle(void) const { return(m_Title); }
	const CString & GetEpisodeTitle(void) const { return(m_EpisodeTitle); }
	const CString & GetDescription(void) const { return(m_Description); }
	const CTime & GetCaptureDate(void) const { return(m_CaptureDate); }
	const CTime & GetLastWriteTime(void) const { return(m_LastWriteTime); }
	const CString & SetMAK(const CString & csMAK);
	const CString & GetMAK(void) const { return(m_csMAK); }
	const CString & GetContentType(void) const { return(m_ContentType); }
	const CString & GetSourceFormat(void) const { return(m_SourceFormat); }
	const unsigned long long & GetDuration(void) const { return(m_Duration); }
	const unsigned long long & GetSourceSize(void) const { return(m_SourceSize); }
	void GetTiVoItem(CComPtr<IXmlWriter> & pWriter) const;
	void GetTvBusEnvelope(CComPtr<IXmlWriter> & pWriter) const;
	const CString GetFFMPEGCommandLine(const CString & csFFMPEGPath = _T("ffmpeg.exe"), const bool bForceSubtitles = false) const;
#ifdef CACHE_FILE
	void WriteToCache(CComPtr<IXmlWriter>& pWriter);
	bool PopulateFromXML(CComPtr<IXmlReader>& pReader, CComPtr<IStream>& spStream);
#endif // CACHE_FILE
};
bool cTiVoFileCompareDate(const cTiVoFile & a, const cTiVoFile & b);
bool cTiVoFileCompareDateReverse(const cTiVoFile & a, const cTiVoFile & b);
/////////////////////////////////////////////////////////////////////////////
bool XML_Parse_TiVoNowPlaying(CComPtr<IStream> &spStream, const CString & csMAK, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers);
bool XML_Parse_TiVoNowPlaying(const CString & Source, const CString & csMAK, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers);
bool XML_Parse_TiVoNowPlaying(const CString & Source, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers, CInternetSession & serverSession);
bool GetTiVoFile(const cTiVoFile & TiVoFile, CInternetSession & serverSession, const CString & csTiVoMAK, const CString & csFileLocation = _T("//Acid/TiVo/"));
/////////////////////////////////////////////////////////////////////////////
const std::string DereferenceURL(const std::string & URL, const std::string & URLParent);
/////////////////////////////////////////////////////////////////////////////
