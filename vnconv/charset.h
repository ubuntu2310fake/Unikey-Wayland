/*------------------------------------------------------------------------------
VnConv: Vietnamese Encoding Converter Library
UniKey Project: http://unikey.sourceforge.net
Copyleft (C) 1998-2002 Pham Kim Long
Contact: longp@cslab.felk.cvut.cz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
--------------------------------------------------------------------------------*/

#ifndef __CHARSET_CONVERT_H
#define __CHARSET_CONVERT_H

#include "vnconv.h"
#include "byteio.h"
#include "pattern.h"

#define TOTAL_VNCHARS 213
#define TOTAL_ALPHA_VNCHARS 186

#if defined(_WIN32)
typedef unsigned __int32 StdVnChar;
#else
typedef unsigned int StdVnChar; //the size should be more specific
#endif

typedef unsigned short UnicodeChar;
typedef unsigned short WORD;

#if !defined(_WIN32)

typedef unsigned int DWORD; //the size should be more specific

#endif

#ifndef LOWORD
#define LOWORD(l)           ((WORD)(l))
#endif

#ifndef HIWORD
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif

const StdVnChar VnStdCharOffset = 0x10000;
const StdVnChar INVALID_STD_CHAR = 0xFFFFFFFF;
const unsigned char PadChar = '#';
const unsigned char PadStartQuote = '\"';
const unsigned char PadEndQuote = '\"';
const unsigned char PadEllipsis = '.';

class VnCharset {
public:
	virtual void startInput() {};
	virtual void startOutput() {};
//	virtual BYTE *nextInput(BYTE *input, int inLen, StdVnChar & stdChar, int & bytesRead) = 0;
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead) = 0;

	//------------------------------------------------------------------------
	// put a character to the output after converting it
	// Arguments:
	//     output[in]: output buffer
	//     stdChar[in]: character in standard charset
	//     outLen[out]: length of converted sequence
	//     maxAvail[in]: max length available.
	// Returns: next position in output
	//------------------------------------------------------------------------
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen) = 0;
};

//--------------------------------------------------
class SingleByteCharset: public VnCharset {
protected:
	WORD m_stdMap[256];
	unsigned char * m_vnChars;
public:
	SingleByteCharset(unsigned char * vnChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UnicodeCharset: public VnCharset {
protected:
	DWORD m_vnChars[TOTAL_VNCHARS];
	UnicodeChar * m_toUnicode;
public:
	UnicodeCharset(UnicodeChar *vnChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class DoubleByteCharset: public VnCharset {
protected:
	WORD m_stdMap[256];
	DWORD m_vnChars[TOTAL_VNCHARS];
	WORD * m_toDoubleChar;
public:
	DoubleByteCharset(WORD *vnChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UnicodeUTF8Charset: public UnicodeCharset
{
public:
	UnicodeUTF8Charset(UnicodeChar *vnChars) : UnicodeCharset(vnChars)	{}

	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UnicodeRefCharset: public UnicodeCharset
{
public:
	UnicodeRefCharset(UnicodeChar *vnChars) : UnicodeCharset(vnChars)	{}

	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UnicodeHexCharset: public UnicodeRefCharset
{
public:
	UnicodeHexCharset(UnicodeChar *vnChars) : UnicodeRefCharset(vnChars) {}
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UnicodeCStringCharset: public UnicodeCharset
{
protected:
	int m_prevIsHex;
public:
	UnicodeCStringCharset(UnicodeChar *vnChars) : UnicodeCharset(vnChars) {}
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
	virtual void startInput();
};

//--------------------------------------------------
class WinCP1258Charset: public VnCharset {
protected:
	WORD m_stdMap[256];
	DWORD m_vnChars[TOTAL_VNCHARS*2];
	WORD *m_toDoubleChar;
	int m_totalChars;

public:
	WinCP1258Charset(WORD *compositeChars, WORD *precomposedChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
struct UniCompCharInfo {
	DWORD	compChar;
	int stdIndex;
};

class UnicodeCompCharset: public VnCharset {
protected:
	UniCompCharInfo m_info[TOTAL_VNCHARS*2];
	DWORD *m_uniCompChars;
	int m_totalChars;
public:
	UnicodeCompCharset(UnicodeChar *uniChars, DWORD *uniCompChars);
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class VIQRCharset: public VnCharset {
protected:
	DWORD *m_vnChars;
	WORD m_stdMap[256];
	int m_atWordBeginning;
	int m_escapeBowl;
	int m_escapeRoof;
	int m_escapeHook;
	int m_escapeTone;
	int m_gotTone;
	int m_escAll;
public:
	int m_suspicious;
	VIQRCharset(DWORD *vnChars);
	virtual void startInput();
	virtual void startOutput();
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};

//--------------------------------------------------
class UTF8VIQRCharset: public VnCharset {

protected:
	VIQRCharset *m_pViqr;
	UnicodeUTF8Charset *m_pUtf;

public:
	UTF8VIQRCharset(UnicodeUTF8Charset *pUtf, VIQRCharset *pViqr);
	virtual void startInput();
	virtual void startOutput();
	virtual int nextInput(ByteInStream & is, StdVnChar & stdChar, int & bytesRead);
	virtual int putChar(ByteOutStream & os, StdVnChar stdChar, int & outLen);
};


//--------------------------------------------------
class CVnCharsetLib {
protected:
	SingleByteCharset * m_sgCharsets[CONV_TOTAL_SINGLE_CHARSETS];
	DoubleByteCharset * m_dbCharsets[CONV_TOTAL_DOUBLE_CHARSETS];
	UnicodeCharset * m_pUniCharset;	
	UnicodeCompCharset * m_pUniCompCharset;
	UnicodeUTF8Charset * m_pUniUTF8;
	UnicodeRefCharset * m_pUniRef;
	UnicodeHexCharset * m_pUniHex;
	VIQRCharset * m_pVIQRCharObj;
	UTF8VIQRCharset * m_pUVIQRCharObj;
	WinCP1258Charset * m_pWinCP1258;
	UnicodeCStringCharset *m_pUniCString;

public:
	PatternList m_VIQREscPatterns;
	VnConvOptions m_options;
	CVnCharsetLib();
	~CVnCharsetLib();
	VnCharset * getVnCharset(int charsetIdx);
};

extern unsigned char SingleByteTables[][TOTAL_VNCHARS];
extern WORD DoubleByteTables[][TOTAL_VNCHARS];
extern UnicodeChar UnicodeTable[TOTAL_VNCHARS];
extern DWORD VIQRTable[TOTAL_VNCHARS];
extern DWORD UnicodeComposite[TOTAL_VNCHARS];
extern WORD WinCP1258[TOTAL_VNCHARS];
extern WORD WinCP1258Pre[TOTAL_VNCHARS];

extern CVnCharsetLib VnCharsetLibObj;
extern VnConvOptions VnConvGlobalOptions;
extern int StdVnNoTone[TOTAL_VNCHARS];

int genConvert(VnCharset & incs, VnCharset & outcs, ByteInStream & input, ByteOutStream & output);

StdVnChar StdVnToUpper(StdVnChar ch);
StdVnChar StdVnToLower(StdVnChar ch);
StdVnChar StdVnRemoveTone(StdVnChar ch);

#endif
