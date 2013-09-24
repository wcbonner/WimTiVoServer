#pragma once

class cTiVoServer
{
//192.168.1.11 tivoconnect=1 swversion=11.0k-01-2-652 method=broadcast identity=6520201806EEAAE machine=Living Room platform=tcd/Series3 services=TiVoMediaServer:80/http
	std::string m_address;
	std::string m_swversion;
	std::string m_method;
	std::string m_identity;
	std::string m_machine;
	std::string m_platform;
	std::string m_services;
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
	unsigned long long m_SourceSize;
    unsigned long long m_Duration;
	CTime m_CaptureDate;
public:
	cTiVoFile() : 
	m_SourceSize(0),
    m_Duration(0)
	{
	}
	friend bool cTiVoFileCompareDate(const cTiVoFile & a, const cTiVoFile & b);
	friend bool cTiVoFileCompareDateReverse(const cTiVoFile & a, const cTiVoFile & b);
	void SetPathName(const CString csNewPath);
	void SetPathName(const CFileFind & csNewPath);
	const CString GetPathName(void) const { return(m_csPathName); }
	void SetFromTiVoItem(const CString &csTitle, const CString &csEpisodeTitle, const CString &csDescription, const CString &csSourceStation, const CString &csContentURL, const CTime &ctCaptureDate, const CTimeSpan &ctsDuration);
	void PopulateFromFFMPEG(void);
	const CString GetURL(void) const { return(m_csURL); }
	void GetXML(CComPtr<IXmlWriter> & pWriter) const;
	void GetTvBusEnvelope(CComPtr<IXmlWriter> & pWriter) const;
};
bool cTiVoFileCompareDate(const cTiVoFile & a, const cTiVoFile & b);
bool cTiVoFileCompareDateReverse(const cTiVoFile & a, const cTiVoFile & b);
extern const CString csUrlPrefix;