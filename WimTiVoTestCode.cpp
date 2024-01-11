#include "stdafx.h"
using namespace std;

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
			std::cout << "[" << getTimeISO8601() << "] Error creating file writer, error is: " << hex << hr << endl;
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
void XML_Test_FileReformat(const CString & Source = _T("D:\\Videos\\chunk-01-0001.xml"), const CString Destination = _T("D:/Videos/Evening-1.xml"))
{
	// crap to clean up a file...  
	HRESULT hr = S_OK;
	CComPtr<IXmlReader> pReader; 
	if (SUCCEEDED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL))) 
	{
		if (SUCCEEDED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit))) 
		{
			CComPtr<IStream> spFileStreamOne;
			if (SUCCEEDED(hr = SHCreateStreamOnFile(Source.GetString(), STGM_READ, &spFileStreamOne)))
			{
				if (SUCCEEDED(hr = pReader->SetInput(spFileStreamOne))) 
				{
					CComPtr<IXmlWriter> pWriter;
					if (SUCCEEDED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
					{
						CComPtr<IStream> spFileStreamTwo;
						if (SUCCEEDED(hr = SHCreateStreamOnFile(Destination.GetString(), STGM_CREATE | STGM_WRITE, &spFileStreamTwo)))
						{
							if (SUCCEEDED(hr = pWriter->SetOutput(spFileStreamTwo)))
							{
								pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
								while (S_OK == (hr = pWriter->WriteNode(pReader, TRUE)));	// loops over entire xml file, writing it out with indenting
								pWriter->Flush();
							}
						}
					}
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
#ifdef _WIM_TIVO_CONVERT_
std::queue<cTiVoFile> TiVoFilesToConvert;
UINT TiVoConvertFileThread(LPVOID lvp)
{
	TRACE(__FUNCTION__ "\n");
	CString m_csTDCatPath(FindEXEFromPath(_T("tdcat.exe")));
	CString m_csTiVoDecodePath(FindEXEFromPath(_T("tivodecode.exe")));
	CString m_csFFMPEGPath(FindEXEFromPath(_T("ffmpeg.exe")));
	while (!TiVoFilesToConvert.empty())
	{
		cTiVoFile TiVoFile(TiVoFilesToConvert.front());
		TiVoFilesToConvert.pop();
		CString csTiVoFileName(TiVoFile.GetPathName());
		CFileStatus status;
		if (TRUE == CFile::GetStatus(csTiVoFileName, status)) // Test to make sure the .TiVo file exists!
		{
			CString csMPEGPathName(ReplaceExtension(csTiVoFileName, _T(".mpeg")));
			CString csXMLPathName(ReplaceExtension(csTiVoFileName, _T(".xml")));
			CString csMP4PathName(ReplaceExtension(csTiVoFileName, _T(".mp4")));
			if (TRUE != CFile::GetStatus(csMPEGPathName, status))
			{
				if (!m_csTDCatPath.IsEmpty())
				{
					std::cout << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(m_csTDCatPath).GetString() << " " << CStringA(m_csTDCatPath).GetString() << "  --mak  " << CStringA(TiVoFile.GetMAK()).GetString() << " --out " << CStringA(QuoteFileName(csXMLPathName)).GetString() << " " << CStringA(csXMLPathName).GetString() << " --chunk-2 " << CStringA(QuoteFileName(csTiVoFileName)).GetString() << std::endl;
					if (-1 == _tspawnl(_P_WAIT, m_csTDCatPath.GetString(), m_csTDCatPath.GetString(), _T("--mak"), TiVoFile.GetMAK(), _T("--out"), QuoteFileName(csXMLPathName).GetString(), _T("--chunk-2"), QuoteFileName(csTiVoFileName).GetString(), NULL))
						std::cout << "[                   ]  _tspawnlp failed: " /* << strerror(errno) */ << std::endl;
				}
				if (TRUE == CFile::GetStatus(csXMLPathName, status))
				{
					status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
					CFile::SetStatus(csXMLPathName, status);
				}

				std::cout << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(m_csTiVoDecodePath).GetString() << " " << CStringA(m_csTiVoDecodePath).GetString() << " --mak " << CStringA(TiVoFile.GetMAK()).GetString() << " --out " << CStringA(QuoteFileName(csMPEGPathName)).GetString() << " " << CStringA(QuoteFileName(csTiVoFileName)).GetString() << std::endl;
				if (-1 == _tspawnl(_P_WAIT, m_csTiVoDecodePath.GetString(), m_csTiVoDecodePath.GetString(), _T("--mak"), TiVoFile.GetMAK(), _T("--out"), QuoteFileName(csMPEGPathName).GetString(), QuoteFileName(csTiVoFileName).GetString(), NULL))
					std::cout << "[                   ]  _tspawnlp failed: " /* << strerror(errno) */ << std::endl;
				if (TRUE == CFile::GetStatus(csMPEGPathName, status))
				{
					status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
					CFile::SetStatus(csMPEGPathName, status);
					DeleteFile(csTiVoFileName);
					if (TRUE != CFile::GetStatus(csMP4PathName, status))
					{
						CString csTitle(TiVoFile.GetTitle()); while(0 < csTitle.Replace(_T("\""), _T("'"))); csTitle.Insert(0,_T("title=\""));csTitle.Append(_T("\""));
						CString csShow(TiVoFile.GetTitle()); while(0 < csShow.Replace(_T("\""), _T("'"))); csShow.Insert(0,_T("show=\""));csShow.Append(_T("\""));
						CString csDescription(TiVoFile.GetDescription()); while(0 < csDescription.Replace(_T("\""), _T("'"))); csDescription.Insert(0,_T("description=\""));csDescription.Append(_T("\""));
						CString csEpisodeID(TiVoFile.GetEpisodeTitle()); while(0 < csEpisodeID.Replace(_T("\""), _T("'"))); csEpisodeID.Insert(0,_T("episode_id=\""));csEpisodeID.Append(_T("\""));
						std::cout << "[" << getTimeISO8601() << "]\tspawn: " << CStringA(m_csFFMPEGPath).GetString() << " " << CStringA(m_csFFMPEGPath).GetString() << " -i " << CStringA(QuoteFileName(csMPEGPathName)).GetString() << " -metadata " << CStringA(csTitle).GetString() << " -metadata " << CStringA(csShow).GetString() << " -metadata " << CStringA(csDescription).GetString() << " -metadata " << CStringA(csEpisodeID).GetString() << " -vcodec copy -acodec copy -y " << CStringA(QuoteFileName(csMP4PathName)).GetString() << std::endl;
						if (-1 == _tspawnlp(_P_WAIT, m_csFFMPEGPath.GetString(), m_csFFMPEGPath.GetString(), _T("-i"), QuoteFileName(csMPEGPathName).GetString(), 
							_T("-metadata"), csTitle.GetString(),
							_T("-metadata"), csShow.GetString(),
							_T("-metadata"), csDescription.GetString(),
							_T("-metadata"), csEpisodeID.GetString(),
							_T("-vcodec"), _T("copy"),
							_T("-acodec"), _T("copy"),
							_T("-y"), // Cause it to overwrite exiting output files
							QuoteFileName(csMP4PathName).GetString(), NULL))
							std::cout << "[                   ]  _tspawnlp failed: " /* << _sys_errlist[errno] */ << std::endl;
					}
					if (TRUE == CFile::GetStatus(csMP4PathName, status))
					{
						status.m_ctime = status.m_mtime = TiVoFile.GetCaptureDate();
						CFile::SetStatus(csMP4PathName, status);
						DeleteFile(csMPEGPathName);
					}
				}
			}
		}
	}
	TRACE(__FUNCTION__ " Exiting\n");
	return(0);
}
#endif
UINT WatchTiVoDirectories(LPVOID lvp)
{
	UINT rval = 0;
	// Obtaining Directory Change Notifications
	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365261(v=vs.85).aspx
	HANDLE dwChangeHandles[2];
	TCHAR * lpDir = _T("//Acid/TiVo/");
	dwChangeHandles[0] = FindFirstChangeNotification( 
		lpDir,                         // directory to watch 
		FALSE,                         // do not watch subtree 
		FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes 
	if (dwChangeHandles[0] == INVALID_HANDLE_VALUE) 
	{
		std::cout << "ERROR: FindFirstChangeNotification function failed." << std::endl;
		rval = GetLastError();
	}
	else if (dwChangeHandles[0] == NULL)
	{
		std::cout << "ERROR: Unexpected NULL from FindFirstChangeNotification." << std::endl;
		rval = GetLastError();
	}
	else
	{
		while (TRUE) 
		{ 
			// Wait for notification.
 			std::cout << "Waiting for notification..." << std::endl;
			DWORD dwWaitStatus = WaitForMultipleObjects(1, dwChangeHandles, FALSE, INFINITE); 
			switch (dwWaitStatus) 
			{ 
			case WAIT_OBJECT_0: 
				// A file was created, renamed, or deleted in the directory.
				// Refresh this directory and restart the notification.
				std::cout << "Directory (" << CStringA(lpDir).GetString() << ") changed." << std::endl;
				if ( FindNextChangeNotification(dwChangeHandles[0]) == FALSE )
				{
					std::cout << "ERROR: FindNextChangeNotification function failed." << std::endl;
					rval = GetLastError();
					return(rval);
				}
				break; 
 
			//case WAIT_OBJECT_0 + 1: 
			//	// A directory was created, renamed, or deleted.
			//	// Refresh the tree and restart the notification.
			//	RefreshTree(lpDrive); 
			//	if (FindNextChangeNotification(dwChangeHandles[1]) == FALSE )
			//	{
			//		std::cout << "ERROR: FindNextChangeNotification function failed." << std::endl;
			//		rval = GetLastError();
			//		return(rval);
			//	}
			//	break; 
 
			case WAIT_TIMEOUT:
				// A timeout occurred, this would happen if some value other 
				// than INFINITE is used in the Wait call and no changes occur.
				// In a single-threaded environment you might not want an
				// INFINITE wait. 
				std::cout << "No changes in the timeout period." << std::endl;
				break;

			default: 
				std::cout << "ERROR: Unhandled dwWaitStatus." << std::endl;
				rval = GetLastError();
				return(rval);
				break;
			}
		}
	}
	return(rval);
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
				std::cout << "[" << getTimeISO8601() << "] " << inet_ntoa(saServer.sin_addr) << " " << csServerBroadcast.GetString() << endl;
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

				#ifdef _DE_CHUNK_FILE_
				std::ifstream FileToDeChunk;
				FileToDeChunk.open("C:\\Users\\Wim\\Videos\\The.Daily.Show.2013.05.02.Eric.Greitens.HDTV.x264-EVOLVE (Recorded May 02, 2013).pytivo.TiVo", ios_base::in | ios_base::binary);
				std::ofstream OutFile("C:\\Users\\Wim\\Videos\\The.Daily.Show.2013.05.02.Eric.Greitens.HDTV.x264-EVOLVE (Recorded May 02, 2013).pytivo.dechunked.TiVo", ios_base::out | ios_base::binary);
				long long chunksize = 0;
				char ChunkHeader[64];
				if (FileToDeChunk.is_open() && OutFile.is_open())
				{
					do {
						FileToDeChunk.getline(ChunkHeader, sizeof(ChunkHeader));
						stringstream ss(ChunkHeader);
						ss << hex;
						ss >> chunksize;
						char * ChunkData = new char[chunksize];
						FileToDeChunk.read(ChunkData, chunksize);
						OutFile.write(ChunkData, chunksize);
						delete[] ChunkData;
						FileToDeChunk.getline(ChunkHeader, sizeof(ChunkHeader)); // gets the chunk footer
					}
					while (chunksize > 0);
					FileToDeChunk.close();
					OutFile.close();
				}
				#endif
				#ifdef _COMPARE_PYTIVO_FILE
				std::ifstream FileOne, FileTwo;
				FileOne.open("C:\\Users\\Wim\\Videos\\The.Daily.Show.2013.05.02.Eric.Greitens.HDTV.x264-EVOLVE (Recorded May 02, 2013).pytivo.dechunked.TiVo", ios_base::in | ios_base::binary);
				FileTwo.open("C:\\Users\\Wim\\Videos\\The.Daily.Show.2013.05.02.Eric.Greitens.HDTV.x264-EVOLVE (Recorded May 02, 2013).wim.1.1.TiVo", ios_base::in | ios_base::binary);
				if (FileOne.is_open() && FileTwo.is_open())
				{
					FileOne.seekg(0x3000);	// Beginning of MPEG
					FileTwo.seekg(0x1c00);	// Beginning of MPEG
					char ByteOne, ByteTwo;
					unsigned long long Position = 0;
					while (!FileOne.eof() && !FileTwo.eof())
					{
						FileOne.read(&ByteOne, 1);
						FileTwo.read(&ByteTwo, 1);
						if (ByteOne != ByteTwo)
						{
							std::cout << "Bytes Don't Match at MPEG Offset " << Position << std::endl;
							break;
						}
						Position++;
					}
					FileOne.close();
					FileTwo.close();
					std::cout << "Finished Comparing " << Position << " bytes" << std::endl;
				}
				#endif
				#ifdef _WIM_TIVO_CONVERT_
				CFileFind finder;
				BOOL bWorking = finder.FindFile(_T("//Acid/TiVo/*.tivo"));
				while (bWorking)
				{
					bWorking = finder.FindNextFile();
					if (finder.IsDots())
						continue;
					if (finder.IsDirectory())
						continue;
					if (finder.IsHidden())
						continue;
					if (finder.IsSystem())
						continue;
					if (finder.IsTemporary())
						continue;
					if (finder.GetLength() > 0)
					{
						cTiVoFile myFile;
						myFile.SetPathName(finder);
						myFile.SetMAK(_T("1760168186"));
						TiVoFilesToConvert.push(myFile);
					}
				}
				finder.Close();
				AfxBeginThread(TiVoConvertFileThread, NULL);
				#endif

			#ifdef _Original_Download_Tests_
			if (SUCCEEDED(CoInitializeEx(0, COINIT_MULTITHREADED))) // COINIT_APARTMENTTHREADED
			{
				//XML_Test_FileReformat(_T("D:\\Videos\\chunk-01-0001.xml"), _T("D:/Videos/Evening Magazine (Recorded Mar 26, 2010, KINGDT).1.xml"));
				//XML_Test_FileReformat(_T("D:\\Videos\\chunk-02-0002.xml"), _T("D:/Videos/Evening Magazine (Recorded Mar 26, 2010, KINGDT).2.xml"));
				//XML_Test_Read_ElementsOnly();
				//XML_Test_Read();
				//XML_Test_Write();
				//XML_Test_Write_InMemory();
				std::vector<cTiVoFile> FilesToGetFromTiVo;
				std::vector<CTiVoContainer> TiVoContainers;
				//XML_Parse_TiVoNowPlaying(_T("WimTivoServer.3.xml"), FilesToGetFromTiVo);
				//std::sort(FilesToGetFromTiVo.begin(),FilesToGetFromTiVo.end(),cTiVoFileCompareDateReverse);
				FilesToGetFromTiVo.clear();
				CInternetSession serverSession0;
				XML_Parse_TiVoNowPlaying(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying&Recurse=Yes&SortOrder=!CaptureDate")), FilesToGetFromTiVo, TiVoContainers, serverSession0);
				std::sort(FilesToGetFromTiVo.begin(),FilesToGetFromTiVo.end(),cTiVoFileCompareDateReverse);				
				FilesToGetFromTiVo.clear();	// This line is temporary just to make sure that no files are downloaded or converted
				for (auto TiVoFileToGet = FilesToGetFromTiVo.begin(); TiVoFileToGet != FilesToGetFromTiVo.end(); TiVoFileToGet++)
				{
					CString csPathName(_T("//Acid/TiVo/"));
					csPathName.Append(TiVoFileToGet->GetPathName());
					CFileStatus status;
					if (TRUE != CFile::GetStatus(csPathName, status))
						while (false == GetTiVoFile(*TiVoFileToGet, serverSession0, _T("1760168186"), _T("//Acid/TiVo/")));
					if (TRUE == CFile::GetStatus(csPathName, status))
					{
						status.m_ctime = status.m_mtime = TiVoFileToGet->GetCaptureDate();
						CFile::SetStatus(csPathName, status);
						CString csMPEGPathName(csPathName);
						csMPEGPathName.Replace(_T(".TiVo"), _T(".mpeg"));
						CString csMP4PathName(csPathName);
						csMP4PathName.Replace(_T(".TiVo"), _T(".mp4"));
						if (TRUE != CFile::GetStatus(csMP4PathName, status)) // Only do a bunch of this stuff if the mp4 file doesn't already exist
						{
							// This is just demo code making sure I can create a temporary file properly for future use with tdcat or tivodecode
							TCHAR lpTempPathBuffer[MAX_PATH];
							DWORD dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
							if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
							{
								std::cout << "[" << getTimeISO8601() << "] GetTempPath failed" << endl;
								_tcscpy(lpTempPathBuffer,_T("."));
							}
							//  Generates a temporary file name. 
							TCHAR szTempFileName[MAX_PATH];  
							if (0 != GetTempFileName(lpTempPathBuffer, AfxGetAppName(), 0, szTempFileName))
							{
								wcout << L"[                   ]  GetTempFileName: " << szTempFileName << endl;
								if (-1 == _tspawnlp(_P_WAIT, _T("tdcat.exe"), _T("tdcat.exe"), _T("--mak"), _T("1760168186"), _T("--out"), szTempFileName, _T("--chunk-2"), QuoteFileName(csPathName).GetString(), NULL))
									std::cout << "[" << getTimeISO8601() << "] _tspawnlp failed: " << _sys_errlist[errno] << endl;

								if (TRUE != CFile::GetStatus(csMPEGPathName, status))
									if (-1 == _tspawnlp(_P_WAIT, _T("tivodecode.exe"), _T("tivodecode.exe"), _T("--mak"), _T("1760168186"), _T("--out"), QuoteFileName(csMPEGPathName).GetString(), QuoteFileName(csPathName).GetString(), NULL))
										std::cout << "[" << getTimeISO8601() << "] _tspawnlp failed: " << _sys_errlist[errno] << endl;
								if (TRUE == CFile::GetStatus(csMPEGPathName, status))
								{
									status.m_ctime = status.m_mtime = TiVoFileToGet->GetCaptureDate();
									CFile::SetStatus(csMPEGPathName, status);
									if (TRUE != CFile::GetStatus(csMP4PathName, status))
									{
										CString csTitle(TiVoFileToGet->GetTitle()); csTitle.Insert(0,_T("title=\""));csTitle.Append(_T("\""));
										CString csShow(TiVoFileToGet->GetTitle()); csShow.Insert(0,_T("show=\""));csShow.Append(_T("\""));
										CString csDescription(TiVoFileToGet->GetDescription()); csDescription.Insert(0,_T("description=\""));csDescription.Append(_T("\""));
										CString csEpisodeID(TiVoFileToGet->GetEpisodeTitle()); csEpisodeID.Insert(0,_T("episode_id=\""));csEpisodeID.Append(_T("\""));
										if (-1 == _tspawnlp(_P_WAIT, _T("ffmpeg.exe"), _T("ffmpeg.exe"), _T("-i"), QuoteFileName(csMPEGPathName).GetString(), 
											_T("-metadata"), csTitle.GetString(),
											_T("-metadata"), csShow.GetString(),
											_T("-metadata"), csDescription.GetString(),
											_T("-metadata"), csEpisodeID.GetString(),
											_T("-vcodec"), _T("copy"),
											_T("-acodec"), _T("copy"),
											_T("-y"), // Cause it to overwrite exiting output files
											QuoteFileName(csMP4PathName).GetString(), NULL))
											std::cout << "[" << getTimeISO8601() << "] _tspawnlp failed: " << _sys_errlist[errno] << endl;
									}
									if (TRUE == CFile::GetStatus(csMP4PathName, status))
									{
										status.m_ctime = status.m_mtime = TiVoFileToGet->GetCaptureDate();
										CFile::SetStatus(csMP4PathName, status);
										DeleteFile(csMPEGPathName);
									}
								}
								DeleteFile(szTempFileName);	// I must delete this file because the zero in the unique field up above causes a file to be created.
							}
						}
					}
				}

				std::vector<CString> myURLS;
				//myURLS.push_back(CString(_T("http://192.168.0.108/TiVoConnect?Command=ResetServer")));
				//myURLS.push_back(CString(_T("http://192.168.0.108/TiVoConnect?Command=QueryContainer&Container=/")));
				myURLS.push_back(CString(_T("http://192.168.0.108/TiVoConnect?Command=QueryFormats&SourceFormat=video%2Fx-tivo-mpeg")));
				//myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying")));
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying&Recurse=Yes&SortOrder=!CaptureDate")));
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying/9948")));
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoVideoDetails?id=984268")));	// It appears that after a "Details" call it uses a "Connection: keep-alive" call, so I should probably make another call to the now playing list to see if that makes the tivo more stable.
				myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying/9948")));
				//myURLS.push_back(CString(_T("http://192.168.1.11/TiVoConnect?Command=QueryContainer&Container=/")));
				//myURLS.push_back(CString(_T("https://tivo:9371539867@192.168.1.11:443/TiVoConnect?Command=QueryContainer&Container=/NowPlaying")));
				//myURLS.push_back(CString(_T("http://192.168.0.5:8080/TiVoConnect?Command=QueryContainer&Container=/")));
				//myURLS.push_back(CString(_T("http://192.168.0.5:8080/TiVoConnect?Command=QueryFormats&SourceFormat=video%2Fx-tivo-mpeg")));
				//myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.5:4430/TiVoConnect?Command=QueryContainer&Container=/TivoNowPlaying")));

				//myURLS.push_back(CString(_T("http://tivo:1760168186@192.168.0.108/download/Evening Magazine.TiVo?Container=/NowPlaying&id=984268")));
				//myURLS.push_back(CString(_T("http://tivo:1760168186@192.168.0.108/download/Evening Magazine.TiVo?Container=/NowPlaying&id=879493")));
				//myURLS.push_back(CString(_T("http://tivo:1760168186@192.168.0.108/download/Community.TiVo?Container=/NowPlaying&id=3002485")));
				//myURLS.push_back(CString(_T("https://tivo:1760168186@192.168.0.108:443/TiVoConnect?Command=ResetServer")));
				myURLS.push_back(CString(_T("http://tivo:1760168186@192.168.0.108/TiVoConnect?Command=ResetServer")));
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
					std::wcout << L"[                   ] strServer: " << strServer.GetString() << endl;
					std::wcout << L"[                   ] strObject: " << strObject.GetString() << endl;
					std::wcout << L"[                   ] nPort: " << nPort << endl;
					std::wcout << L"[                   ] strUsername: " << strUsername.GetString() << endl;
					std::wcout << L"[                   ] strPassword: " << strPassword.GetString() << endl;
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
									if (!csContentType.CompareNoCase(_T("text/xml")))
									{								
										CComPtr<IStream> spMemoryStreamOne(::SHCreateMemStream(NULL, 0));
										CComPtr<IStream> spMemoryStreamTwo(::SHCreateMemStream(NULL, 0));
										if ((spMemoryStreamOne != NULL) && (spMemoryStreamTwo != NULL))
										{
											char ittybittybuffer;
											ULONG cbWritten;
											while (0 < serverFile->Read(&ittybittybuffer, sizeof(ittybittybuffer)))
												spMemoryStreamOne->Write(&ittybittybuffer, 1, &cbWritten);

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
														CComPtr<IXmlWriter> pWriter;
														if (SUCCEEDED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL))) 
														{
															pWriter->SetOutput(spMemoryStreamTwo);
															pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
															while (S_OK == (hr = pWriter->WriteNode(pReader, TRUE)));	// loops over entire xml file, writing it out with indenting
															//pWriter->WriteEndDocument();
															pWriter->Flush();

															// Allocates enough memeory for the xml content.
															STATSTG ssStreamData = {0};
															spMemoryStreamTwo->Stat(&ssStreamData, STATFLAG_NONAME);
															SIZE_T cbSize = ssStreamData.cbSize.LowPart;
															char *XMLDataBuff = new char[cbSize+1];

															// Copies the content from the stream to the buffer.
															LARGE_INTEGER position;
															position.QuadPart = 0;
															spMemoryStreamTwo->Seek(position, STREAM_SEEK_SET, NULL);

															ULONG cbRead;
															spMemoryStreamTwo->Read(XMLDataBuff, cbSize, &cbRead);
															XMLDataBuff[cbSize] = '\0';
															std::stringstream OutPutFileName;
															//std::stringstream ReturnedData;
															OutPutFileName << "WimTivoServer." << FileIndex++ << ".xml";
															std::cout << "[                   ] Writing File: " << OutPutFileName.str() << endl;
															std::ofstream OutputFile(OutPutFileName.str(), ios_base::binary);
															if (OutputFile.is_open())
																OutputFile.write(XMLDataBuff,cbSize);
															OutputFile.close();
															delete[] XMLDataBuff;
														}
													}
												}
											}
										}
									}
									else if (!csContentType.CompareNoCase(_T("video/x-tivo-mpeg")))
									{
										std::stringstream OutPutFileName;
										OutPutFileName << "WimTivoServer." << FileIndex++ << ".TiVo";
										std::cout << "[                   ] Writing File: " << OutPutFileName.str() << endl;
										std::ofstream OutputFile(OutPutFileName.str(), ios_base::binary);
										if (OutputFile.is_open())
										{
											static size_t ReadWriteBufferSize = 1024;
											char* ReadWriteBuffer = new char[ReadWriteBufferSize];
											long long TotalRead = 0;
											UINT uiRead;
											CTime ctCurrent(CTime::GetCurrentTime());
											CTime ctStart(ctCurrent);
											CTime ctLastOutput(ctStart);
											CTimeSpan ctsTotal = ctCurrent - ctStart;
											std::locale mylocale("");   // get global locale
											std::locale OriginalLocale(std::cout.imbue(mylocale));  // imbue global locale
											std::cout << "[                   ] ReadWriteBufferSize: " << ReadWriteBufferSize << endl;
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
											delete[] ReadWriteBuffer;
											std::cout << "[" << getTimeISO8601() << "] Elapsed: " << CStringA(ctsTotal.Format(_T("%H:%M:%S"))).GetString() << " Bytes: " << TotalRead << " bytes/second: " <<  (TotalRead / ctsTotal.GetTotalSeconds()) << " ReadWriteBufferSize: " << ReadWriteBufferSize << endl;
											std::cout.imbue(OriginalLocale);
											ReadWriteBufferSize *= 10;
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
										csURL--;
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
								}
							}
						}
					}
				}
			}
			CoUninitialize();

			if (csMyHostName.CompareNoCase(_T("INSPIRON")))
			{
				std::cout << "[" << getTimeISO8601() << "] Listening for TiVo Broadcast packets on UDP port 2190" << endl;
				// Set up CTR-C signal handler
				typedef void (*SignalHandlerPointer)(int);
				SignalHandlerPointer previousHandler = signal(SIGINT, SignalHandler);
				int loopCounter = 0;
				bRun = true;
				SOCKADDR_IN saServer;
				while (bRun && (TiVoBeaconListen(saServer) == true))
					std::cout << "[" << getTimeISO8601() << "] \r";
				// remove our special Ctrl-C signal handler and restore previous one
				signal(SIGINT, previousHandler);
				std::cout << "\n[" << getTimeISO8601() << "] No longer listening for TiVo Broadcast packets on UDP port 2190" << endl;
			}
			#endif

extern CWinApp theApp;
volatile bool bRun = true;
void SignalHandler(int signal)
{
	bRun = false;
	cerr << "[" << getTimeISO8601() << "] SIGINT: Caught Ctrl-C, cleaning up and moving on." << endl;
	std::string csTiVoPacket("tivoconnect=0\r\n");
//	TiVoBeaconSend(csTiVoPacket);
}
void oldmainstuff(void)
{
			CString UserName;
			if (UserName.IsEmpty())
			{
				TCHAR UserNameBuff[256];
				DWORD UserNameSize = sizeof(UserNameBuff)/sizeof(TCHAR);
				GetUserName(UserNameBuff,&UserNameSize); // this is "wim" when I run it.
				UserName = CString(UserNameBuff,UserNameSize);
			}

			CString csMyProgramGuid(theApp.GetProfileString(_T("Settings"), _T("GUID")));
			if (csMyProgramGuid.IsEmpty())
			{
				GUID MyProgramGuid;
				if (SUCCEEDED(CoCreateGuid(&MyProgramGuid)))
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

			CString csBuildDateTime;
			if (csBuildDateTime.IsEmpty())
			{
				const char * BuildDateTime = "<BuildDateTime>" __DATE__ " " __TIME__ "</BuildDateTime>";
				char cmonth[4];
				int year, day, hour, min, sec;
				sscanf(&(BuildDateTime[15]),"%s %d %d %d:%d:%d",cmonth,&day,&year,&hour,&min,&sec);
				const char * Months[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
				int month = 0;
				while (strcmp(cmonth,Months[month]) != 0)
					month++;
				month++;
				csBuildDateTime.Format(_T("%04d%02d%02d%02d%02d%02d"),year,month,day,hour,min,sec);
			}


			//std::wcout << L"[                   ] Snowman: ☃" << endl;
			std::cout << "[" << getTimeISO8601() << "] Use key combination Ctrl-C to end the program." << endl;
			std::locale mylocale("");   // get global locale
			std::locale OriginalLocale(std::cout.imbue(mylocale));  // imbue global locale
			std::cout << "[" << getTimeISO8601() << "] Test of digit grouping: " << 1024000 << endl;
			std::cout.imbue(OriginalLocale);
			std::cout << "[" << getTimeISO8601() << "] Test of digit grouping: " << 1024000 << endl;

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
}