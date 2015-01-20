#pragma once

#ifndef PEKSPRODUCTIONS_TEXTFILE
#define PEKSPRODUCTIONS_TEXTFILE

/*
основано на исходниках http://www.codeproject.com/file/textfiledocument.asp
*/

#include <string>

using namespace std;

// тип названия файла
typedef WCHAR FILENAMECHAR;

// размер буфера чтения/записи
#define BUFFSIZE 1024

// проверяет валидность кодировки
#define IsLegalCodePage(cp) (CP_ACP == cp || CP_MACCP == cp || CP_OEMCP == cp || CP_SYMBOL == cp || CP_THREAD_ACP == cp || CP_UTF7 == cp || CP_UTF8 == cp || IsValidCodePage(cp))


//////////////////////////////////////////////////////////////////////////
// Класс-исключение
//
class CTextFileException
{
	protected:

		DWORD m_errorCode;

	public:

		CTextFileException(DWORD err)
		{
			m_errorCode = err;
		}
};


//////////////////////////////////////////////////////////////////////////
// Базовый класс
// 
class CTextFileBase
{
	public:

		enum TEXTENCODING {ASCII, UNI16_BE, UNI16_LE, UTF_8};

		// контруктор по-умолчанию
		CTextFileBase()
		{
			m_codepage    = CP_ACP;
			m_unknownChar = 0;
			m_hFile       = INVALID_HANDLE_VALUE;
			m_datalost    = FALSE;
			m_buffpos     = -1;
		}

		// деструктор по-умолчанию
		~CTextFileBase()
		{
			Close();
		}

	protected:
		
		TEXTENCODING m_encoding;      // The enocoding of the file
		HANDLE       m_hFile;
		BOOL         m_endoffile;     // True if end of file
		CHAR         m_buf[BUFFSIZE]; // Readingbuffer
		INT          m_buffpos;       // Bufferposition
		INT          m_buffsize;      // Size of buffer
		CHAR         m_unknownChar;   // Character used when converting Unicode->multi byte and an unknown character was found
		BOOL         m_datalost;      // Is true if data was lost when converting Unicode->multi-byte
		UINT         m_codepage;

	protected:

		// Convert char* to wstring
		VOID CharToWstring(const char* from, wstring &to) const
		{
			ConvertCharToWstring(from, to, m_codepage);
		}

		//Convert wchar_t* to string
		VOID WcharToString(const wchar_t* from, string &to)
		{
			ConvertWcharToString(from, to, m_codepage, &m_datalost, m_unknownChar);
		}

	public:

		// Файл открыт ?
		INT IsOpen()
		{
			return INVALID_HANDLE_VALUE != m_hFile;
		}


		// Закрыть файл
		virtual VOID Close()
		{
			if(TRUE == IsOpen())
			{
				ATLVERIFY(::CloseHandle(m_hFile));

				m_hFile = INVALID_HANDLE_VALUE;
			}
		}


		// возвращает кодировку файла
		TEXTENCODING GetEncoding() const
		{
			return m_encoding;
		}


		// Set which character that should be used when converting
		// Unicode->multi byte and an unknown character is found ('?' is default)
		VOID SetUnknownChar(const char unknown)
		{
			m_unknownChar = unknown;
		}

		// Returns true if data was lost
		// (happens when converting Unicode->multi byte string and an unmappable
		// characters is found).
		BOOL IsDataLost() const
		{
			return m_datalost;
		}


		// Reset the data lost flag
		VOID ResetDataLostFlag()
		{
			m_datalost = FALSE;
		}


		// Set codepage to use when working with none-Unicode strings
		VOID SetCodePage(const UINT codepage)
		{
			ATLASSERT(IsLegalCodePage(codepage));
			m_codepage = codepage;
		}


		//Get codepage to use when working with none-Unicode strings
		UINT GetCodePage() const
		{
			return m_codepage;
		}


		//Convert char* to wstring
		static VOID ConvertCharToWstring(const char* from, wstring &to, UINT codepage = CP_ACP)
		{
			ATLASSERT(IsLegalCodePage(codepage));

			to = L"";

			//Use api convert routine
			INT wlen = MultiByteToWideChar(codepage, 0, from, -1, NULL, 0);

			// if wlen == 0, some unknown codepage was probably used.
			ATLASSERT(wlen);
			if (0 == wlen) 
			{
				return;
			}

			PWCHAR wbuffer = new WCHAR[wlen + 2];

			MultiByteToWideChar(codepage, 0, from, -1, wbuffer, wlen);

			to = wbuffer;

			delete [] wbuffer;
		}


		//Convert wchar_t* to string
		static VOID ConvertWcharToString(const wchar_t* from, string &to, UINT codepage = CP_ACP, BOOL* datalost = NULL, char unknownchar = 0)
		{
			ATLASSERT(IsLegalCodePage(codepage));

			to = "";

			INT alen = WideCharToMultiByte(codepage, 0, from, -1, NULL, 0, NULL, NULL);

			// if alen == 0, some unknown codepage was probably used.
			ATLASSERT(alen);
			if (0 == alen) 
			{
				return;
			}

			// Use mfc convert routine
			char* abuffer     = new CHAR[alen + 2]; 
			BOOL  UsedDefault = FALSE;

			WideCharToMultiByte(codepage, 0, from, -1, abuffer, alen, (unknownchar != 0 ? &unknownchar : NULL), (datalost != NULL ? &UsedDefault : NULL));

			if ((NULL != datalost) && (FALSE != UsedDefault))
			{
				*datalost = TRUE;
			}

			to = abuffer;

			delete [] abuffer;
		}
};


//////////////////////////////////////////////////////////////////////////
// Запись текстовых файлов
//
class CTextFileWrite : public CTextFileBase
{
	public:
		CTextFileWrite(const FILENAMECHAR* filename, TEXTENCODING type = ASCII)
		{
			m_hFile = ::CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			m_buffpos  = -1;
			m_buffsize = 0;
			m_encoding = type;

			WriteBOM();
		}

		~CTextFileWrite()
		{
			Close();
		}

	private:

		// Write and empty buffer
		VOID Flush()
		{
			DWORD nWritten;

			if (!::WriteFile(m_hFile, m_buf, m_buffpos + 1, &nWritten, NULL))
			{
				// Something bad has happend! Close file
				CTextFileBase::Close();

				// Throw exception
				throw CTextFileException(GetLastError());
			}

			m_buffpos = -1;
		}


		// Write a single one wchar_t, convert first
		VOID WriteWchar(const wchar_t ch)
		{
			// Write HO byte first?
			if (UNI16_BE == m_encoding)
			{
				// Write HO byte
				WriteByte((unsigned char)(ch >> 8));

				// Write LO byte
				WriteByte((unsigned char)ch);
			}
			else 
	
			if (UNI16_LE == m_encoding)
			{
				// Write LO byte
				WriteByte((unsigned char)ch);

				// Write HO byte
				WriteByte((unsigned char)(ch >> 8));
			}
			else
			{
				// http://www.cl.cam.ac.uk/~mgk25/unicode.html#examples
				// http://www.ietf.org/rfc/rfc3629.txt

				// Just a single byte?
				if(ch <= 0x7F)
				{
					// U-00000000 - U-0000007F:  0xxxxxxx  
					WriteByte((unsigned char)ch);
				}
				else 

				//Two bytes?
				if(ch <= 0x7FF)
				{
					// U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
					WriteByte((unsigned char)(0xC0 | (ch >> 6)));
					WriteByte((unsigned char)(0x80 | (ch & 0x3F)));
				}
				else 

				// Three bytes?
				if(ch <= 0xFFFF)
				{
					// U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx  
					WriteByte((unsigned char)(0xE0 | (ch >> 12)));
					WriteByte((unsigned char)(0x80 | ((ch >> 6) & 0x3F)));
					WriteByte((unsigned char)(0x80 | (ch & 0x3F)));
				}
			}
		}

	
		// Write one byte
		VOID WriteByte(const unsigned char byte)
		{
			// Instead of writing, save data in buffer and write when buffer is full
			if(m_buffpos + 1 >= BUFFSIZE)
			{
				Flush();
			}

			m_buffpos++;
			m_buf[m_buffpos] = byte;
		}

	
		// Write a c-string in ASCII-format
		VOID WriteAsciiString(const char* s)
		{
			while(*s != '\0')
			{
				WriteByte(*s);
				s++;
			}
		}

		// Write byte order mark
		VOID WriteBOM()
		{
			ATLASSERT(IsOpen());
			if(IsOpen())
			{
				if ((UNI16_BE == m_encoding) || (UNI16_LE == m_encoding))
				{
					WriteWchar( 0xFEFF );
				}
				else 

				if (UTF_8 == m_encoding)
				{
					// Write UTF-8 BOM.  0xEF 0xBB 0xBF
					WriteByte(0xEF);
					WriteByte(0xBB);
					WriteByte(0xBF);
				}
			}
		}

	public:

		// Write routines
		VOID Write(const char* text)
		{
			// ASCIItext file format?
			if (ASCII == m_encoding)
			{
				WriteAsciiString(text);
			}
			else
			{
				// Convert text to unicode
				wstring utext;

				CharToWstring(text, utext);

				// OK, lets write unicode
				for (wstring::const_iterator i = utext.begin(); i != utext.end(); i++)
				{
					WriteWchar(*i);
				}
			}
		}


		VOID Write(const wchar_t* utext)
		{
			// ASCII text file format?
			if(ASCII == m_encoding)
			{
				// Convert to string and write
				string text;

				WcharToString(utext, text);
				WriteAsciiString(text.c_str());
			}
			else
			{
				while (*utext != 0)
				{
					WriteWchar(*utext);
					utext++;
				}
			}
		}


		VOID Write(const string& text)
		{
			Write(text.c_str());
		}


		VOID Write(const wstring& text)
		{
			Write(text.c_str());
		}

		
		CTextFileWrite& operator << (const char c)
		{
			string text;
			
			text = c;

			Write(text.c_str());

			return *this;
		}


		CTextFileWrite& operator << (const char* text)
		{
			Write(text); 

			return *this;
		}


		CTextFileWrite& operator << (const string& text)
		{
			Write(text.c_str());

			return *this;
		}


		CTextFileWrite& operator << (const wchar_t wc)
		{
			//Not the perfect code, but it's easy!
			wstring text;
			
			text = wc;

			Write(text.c_str());

			return *this;
		}


		CTextFileWrite& operator << (const wchar_t* text)
		{
			Write(text); 

			return *this;
		}


		CTextFileWrite& operator << (const wstring& text)
		{
			Write(text.c_str());

			return *this;
		}


		// Write new line (two characters, 13 and 10)
		VOID WriteEndl()
		{
			if (ASCII == m_encoding)
			{
				WriteByte(0x0D);
				WriteByte(0x0A);
			}
			else 
			{
				WriteWchar(0x0D);
				WriteWchar(0x0A);
			}
		}

		// Close the file
		virtual VOID Close()
		{
			ATLASSERT(IsOpen());
			if(IsOpen())
			{
				Flush();
			}

			CTextFileBase::Close();
		}

};


//////////////////////////////////////////////////////////////////////////
// Класс чтения текстовых файлов
//
class CTextFileRead : public CTextFileBase
{
	public:

		CTextFileRead(const FILENAMECHAR* filename)
		{
			m_hFile = ::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			m_firstLine = TRUE;
			m_endoffile = (0 == IsOpen());

			// Force reading to buffer next time
			m_buffpos = -1;

			m_useExtraBuffer = FALSE;

			ReadBOM();
		}

	private:
	
		// Use extra buffer. Sometimes we read one character to much, save it.
		BOOL m_useExtraBuffer;

		// Used to read see if the first line in file is to read 
		// (so we know how to handle \n\r)
		BOOL m_firstLine;

		// Extra buffer. It's ok to share the memory
		union
		{
			CHAR  m_extraBuffer_char;
			WCHAR m_extraBuffer_wchar;
		};


	public:

		// Returns false if end-of-file was reached
		// (line will not be changed). If returns true,
		// it means that last line ended with a line break.
		BOOL ReadLine(string& line)
		{
			// EOF?
			if(Eof())
			{
				return FALSE;
			}

			if (ASCII == m_encoding)
			{
				return ReadCharLine(line);
			}

			wstring wline;

			if(!ReadWcharLine(wline))
			{
				return FALSE;
			}

			// Convert
			WcharToString(wline.c_str(), line);

			return TRUE;
		}


		BOOL ReadLine(wstring& line)
		{
			// EOF?
			if (Eof())
			{
				return FALSE;
			}

			if (ASCII == m_encoding)
			{
				string cline;

				if (!ReadCharLine(cline))
				{
					return FALSE;
				}

				// Convert to wstring
				CharToWstring(cline.c_str(), line);

				return TRUE;
			}

			return ReadWcharLine(line);
		}


		// Returns everything from current position.
		BOOL Read(string& all, const string newline = "\r\n")
		{
			if (!IsOpen())
			{
				return FALSE;
			}

			INT buffsize = GuessCharacterCount()+2;
			INT buffpos = 0;

			// Create buffer
			CHAR* buffer = new CHAR[buffsize];

			// If not possible, don't use any buffer
			if(NULL == buffer) 
			{
				buffsize = 0;
			}

			string temp;
			
			all = temp;
			all.reserve(buffsize);
			
			bool firstLine=true;

			while (!Eof())
			{
				if (ReadLine(temp))
				{
					// Add new line, if not first line
					if(!firstLine)
					{
						temp.insert(0, newline.c_str());
					}
					else
					{
						firstLine = FALSE;
					}

					// Add to buffer if possible
					if (buffpos + (INT) temp.length() < buffsize)
					{
						strcpy_s(buffer + buffpos, temp.length() - 1, temp.c_str());
						
						buffpos += (INT) temp.size();
					}
					else
					{
						// Copy to all string
						if (0 != buffpos)
						{
							all.append(buffer, buffpos);
							buffpos = 0;
						}

						all += temp;
					}
				}
			};

			//Copy to all string
			if(0 != buffpos)
			{
				all.append(buffer, buffpos);
			}

			if(NULL != buffer)
			{
				delete [] buffer;
			}

			return TRUE;
		}


		BOOL Read(wstring& all, const wstring newline = L"\r\n")
		{
			if(!IsOpen())
			{
				return FALSE;
			}

			INT buffsize = GuessCharacterCount() + 2;
			INT buffpos  = 0;

			// Create buffer
			wchar_t* buffer = new wchar_t[buffsize];

			// If not possible, don't use any buffer
			if (NULL == buffer) 
			{
				buffsize = 0;
			}

			wstring temp;
			
			all = temp;
			all.reserve(buffsize);
			
			BOOL firstLine = TRUE;

			while (!Eof())
			{
				if (ReadLine(temp))
				{
					// Add new line, if not first line
					if (!firstLine)
					{
						temp.insert(0, newline.c_str());
					}
					else
					{
						firstLine = FALSE;
					}

					// Add to buffer if possible
					if (buffpos + (int) temp.length() < buffsize)
					{
						wcscpy_s(buffer + buffpos, temp.length() - 1, temp.c_str());
						
						buffpos += (int) temp.size();
					}
					else
					{
						// Copy to all string
						if (0 != buffpos)
						{
							all.append(buffer, buffpos);
							
							buffpos = 0;
						}

						all += temp;
					}
				}
			};

			// Copy to all string
			if (0 != buffpos)
			{
				all.append(buffer, buffpos);
			}

			if (NULL != buffer)
			{
				delete [] buffer;
			}

			return TRUE;
		}

		// End of file?
		BOOL Eof() const
		{
			return m_endoffile;
		}

	private:

		// Guess the number of characters in the file
		INT GuessCharacterCount()
		{
			INT bytecount = 1024*1024; // Default: 1 MB

			// If ASCII, the number of characters is the byte count.
			// If UTF-8, it can't be more than bytecount, so use byte count
			if ((ASCII == m_encoding) || (UTF_8 == m_encoding))
			{
				return bytecount;
			}

			// Otherwise, every two byte in one character
			return bytecount / 2;
		}


		// Read line to wstring
		BOOL ReadWcharLine(wstring& line)
		{
			// EOF?
			if (Eof())
			{
				return FALSE;
			}

			wchar_t ch = 0;

			// Ignore 0x0D and 0x0A
			// or just 0x0D
			// or just 0x0A
			// except when we read the first line
			ReadWchar(ch);

			if (!m_firstLine)
			{
				if (0x0D == ch) //If next is 0x0A, ignore that too
				{
					ReadWchar(ch);

					if (0x0A == ch)
					{
						ReadWchar(ch);
					}
				}
				else 
				
				if (0x0A == ch)
				{
					ReadWchar(ch);
				}
			}
			else
			{
				// Next time we reads we don't read the first line in file.
				// (then we should ignore \r\n)
				m_firstLine = FALSE;
			}

			// Clear line
			line = L"";

			// It would be a lot easier if we didn't use a buffer, and added directly to
			// line, but that is quite slow.
			WCHAR buffer[BUFFSIZE];

			buffer[BUFFSIZE-1] = '\0';

			// Where to insert next character
			INT bufpos = 0;

			// Read line
			while ((0x0D != ch)  && (0x0A != ch) && !Eof())
			{
				// End of buffer?
				if((bufpos + 1) >= BUFFSIZE)
				{
					// Add to line
					line.append(buffer, bufpos);
					
					bufpos = 0;
				}

				buffer[bufpos] = ch;
				bufpos++;

				ReadWchar(ch);
			};

			buffer[bufpos] = L'\0';
			line += buffer;

			// Save currents character in extra buffer
			m_useExtraBuffer    = TRUE;
			m_extraBuffer_wchar = ch;

			return TRUE;
		}

		// Read line to string
		BOOL ReadCharLine(string& line)
		{
			// EOF?
			if (Eof())
			{
				return FALSE;
			}

			UCHAR ch = 0;

			// Ignore 0x0D and 0x0A
			// or just 0x0D
			// or just 0x0A
			// except when we read the first line
			ReadByte(ch);

			if (!m_firstLine)
			{		
				//If next is 0x0A, ignore that too
				if (0x0D == ch)
				{
					ReadByte(ch);

					if (0x0A == ch)
					{
						ReadByte(ch);
					}
				}
				else 
					
				if (0x0A == ch)
				{
					ReadByte(ch);
				}
			}
			else
			{
				// Next time we reads we don't read the first line in file.
				// (then we should ignore \r\n)
				m_firstLine = FALSE;
			}

			// Clear line
			line = "";

			// It would be a lot easier if we didn't use a buffer, and added directly to
			// line, but that is quite slow.
			CHAR buffer[BUFFSIZE];
			
			buffer[BUFFSIZE - 1] = '\0';
			
			// Where to insert next character
			INT bufpos = 0;

			// Read line
			while ((0x0D != ch) && (0x0A != ch) && !Eof())
			{
				// End of buffer?
				if (bufpos + 1 >= BUFFSIZE)
				{
					// Add to line
					line.append(buffer, bufpos);
					
					bufpos = 0;
				}

				buffer[bufpos] = ch;
				bufpos++;

				ReadByte(ch);
			};

			buffer[bufpos] = L'\0';
			line += buffer;

			// Save currents character in extra buffer
			m_useExtraBuffer   = TRUE;
			m_extraBuffer_char = ch;

			return TRUE;
		}

		// Reset the filepointer to start
		VOID ResetFilePointer()
		{
			m_useExtraBuffer = FALSE;

			::SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);

			//Force reread buffer
			m_buffpos = -1;

			m_firstLine = true;
			m_endoffile = false;
		}

		// Read one wchar_t
		VOID ReadWchar(wchar_t& ch)
		{
			if (m_useExtraBuffer)
			{
				m_useExtraBuffer = FALSE;
				ch = m_extraBuffer_wchar;
				
				return;
			}

			if (UTF_8 == m_encoding)
			{	
				// This is quite tricky :-/
				// http://www.cl.cam.ac.uk/~mgk25/unicode.html#examples
				UCHAR byte;
				
				ReadByte(byte);

				INT onesBeforeZero = 0;

				// Calc how many ones before the first zero...
				{	
					UCHAR temp = byte;

					while (0 != (temp & 0x80))
					{
						temp = (UCHAR)(temp << 1);
						onesBeforeZero++;
					}
				}

				if (0 == onesBeforeZero)
				{
					ch = byte;
					
					return;
				}
				else 
					
				if (2 == onesBeforeZero)
				{
					//U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
					UCHAR byteb;
					
					ReadByte(byteb);

					ch = (WCHAR)(((0x1F & byte) << 6) | (0x3F & byteb));

					return;
				}
				else 
					
				if (3 == onesBeforeZero)
				{
					//U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
					UCHAR byteb, bytec;
					
					ReadByte(byteb);
					ReadByte(bytec);

					ch = (WCHAR)(((0x0F & byte) << 12) | ((0x3F & byteb) << 6) | (0x3F & bytec));

					return;
				}

				// This should never happend! It it do, something is wrong with the file.
				ch = 0xFFFD;
			}
			else
			{
				UCHAR bytes[2];
				
				ReadByte(bytes[0]);
				ReadByte(bytes[1]); 

				if (UNI16_BE == m_encoding)
				{
					ch = (WCHAR)(((WCHAR)bytes[0] << 8) | (WCHAR)bytes[1]);
				}
				else
				{
					ch = (WCHAR)(((WCHAR)bytes[1] << 8) | (WCHAR)bytes[0]);
				}
			}
		}


		// Read one byte
		VOID ReadByte(unsigned char& ch)
		{
			// Use extrabuffer if needed
			if(m_useExtraBuffer)
			{
				m_useExtraBuffer = FALSE;
				ch = m_extraBuffer_char;
				
				return;
			}

			// In Windows, do this...

			// If buffer used or not read
			if ((-1 == m_buffpos) || ((BUFFSIZE - 1) == m_buffpos))
			{
				DWORD dwRead;

				if (!::ReadFile(m_hFile, m_buf, BUFFSIZE, &dwRead, NULL))
				{
					// Couldn't read!
					Close();

					m_buffsize = 0;

					// Throw exception
					throw CTextFileException(GetLastError());
				}
				else
				{
					m_buffsize = (INT)dwRead;
				}

				if (0 == m_buffsize)
				{
					m_endoffile = TRUE;
					ch = 0;

					return;
				}

				m_buffpos = 0;
			}
			else
			{
				m_buffpos++;

				if (m_buffpos >= m_buffsize)
				{
					m_endoffile = TRUE;
					ch = 0;

					return;
				}
			}

			ch = m_buf[m_buffpos];
		}


		// Detect encoding
		VOID ReadBOM()
		{
			if (IsOpen())
			{
				UCHAR bytes[2];

				// Read the first two bytes
				ReadByte(bytes[0]);
				ReadByte(bytes[1]);

				// Figure out what format the file is in
				if ((0xFF == bytes[0]) && (0xFE == bytes[1]))
				{
					m_encoding = UNI16_LE;
				}
				else 
				
				if ((0xFE == bytes[0]) && (0xFF == bytes[1]))
				{
					m_encoding = UNI16_BE;
				}
				else 
					
				if ((0xEF == bytes[0]) && (0xBB == bytes[1]))
				{
					// This is probably UTF-8, check the third byte
					UCHAR temp;
					
					ReadByte(temp);
					
					if (0xBF == temp)
					{
						m_encoding = UTF_8;
					}
					else
					{
						// Set text format.
						m_encoding = ASCII;
						
						ResetFilePointer();
					}
				}
				else
				{
					m_encoding = ASCII;

					// Set start pos
					ResetFilePointer();
				}
			}
		}
};

#endif // PEKSPRODUCTIONS_TEXTFILE