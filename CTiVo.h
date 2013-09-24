#pragma once

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
	CString m_csURL;
	CString m_Title;
	CString m_EpisodeTitle;
	CString m_Description;
	CString m_ContentType;
	CString m_SourceFormat;
	CString m_SourceStation;
	unsigned long long m_SourceSize;
    unsigned long long m_Duration;
	CTime m_CaptureDate;
	CString m_csMAK;
public:
	cTiVoFile() : 
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
	// Access Routines
	void SetPathName(const CString csNewPath);
	void SetPathName(const CFileFind & csNewPath);
	const CString & GetPathName(void) const { return(m_csPathName); }
	void SetFromTiVoItem(const CString &csTitle, const CString &csEpisodeTitle, const CString &csDescription, const CString &csSourceStation, const CString &csContentURL, const CTime &ctCaptureDate, const CTimeSpan &ctsDuration, const CString & csMAK, const unsigned long long llSourceSize = 0);
	#ifdef AVCODEC_AVCODEC_H
	void PopulateFromFFMPEG(void);
	#endif
	const CString & GetURL(void) const { return(m_csURL); }
	const CString & GetTitle(void) const { return(m_Title); }
	const CString & GetEpisodeTitle(void) const { return(m_EpisodeTitle); }
	const CString & GetDescription(void) const { return(m_Description); }
	const CTime & GetCaptureDate(void) const { return(m_CaptureDate); }
	const CString & GetMAK(void) const { return(m_csMAK); }
	const unsigned long long & GetDuration(void) const { return(m_Duration); }
	const unsigned long long & GetSourceSize(void) const { return(m_SourceSize); }
	void GetXML(CComPtr<IXmlWriter> & pWriter) const;
	void GetTvBusEnvelope(CComPtr<IXmlWriter> & pWriter) const;
};
bool cTiVoFileCompareDate(const cTiVoFile & a, const cTiVoFile & b);
bool cTiVoFileCompareDateReverse(const cTiVoFile & a, const cTiVoFile & b);
extern const CString csUrlPrefix;
/////////////////////////////////////////////////////////////////////////////
void XML_Parse_TiVoNowPlaying(CComPtr<IStream> &spStream, const CString & csMAK, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers);
void XML_Parse_TiVoNowPlaying(const CString & Source, const CString & csMAK, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers);
bool XML_Parse_TiVoNowPlaying(const CString & Source, std::vector<cTiVoFile> & TiVoFileList, std::vector<CTiVoContainer> & TiVoTiVoContainers, CInternetSession & serverSession);
bool GetTiVoFile(const cTiVoFile & TiVoFile, CInternetSession & serverSession, const CString & csTiVoMAK, const CString & csFileLocation = _T("//Acid/TiVo/"));
/////////////////////////////////////////////////////////////////////////////
