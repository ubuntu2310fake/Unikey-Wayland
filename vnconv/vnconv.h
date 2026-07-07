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

#ifndef __VN_CONVERT_H
#define __VN_CONVERT_H

#define CONV_CHARSET_UNICODE	0
#define CONV_CHARSET_UNIUTF8    1
#define CONV_CHARSET_UNIREF     2  //&#D;
#define CONV_CHARSET_UNIREF_HEX 3
#define CONV_CHARSET_UNIDECOMPOSED 4
#define CONV_CHARSET_WINCP1258	5
#define CONV_CHARSET_UNI_CSTRING 6

#define CONV_CHARSET_VIQR		10
#define CONV_CHARSET_UTF8VIQR 11


#define CONV_CHARSET_TCVN3		20
#define CONV_CHARSET_VPS		21
#define CONV_CHARSET_VISCII		22
#define CONV_CHARSET_BKHCM1		23
#define CONV_CHARSET_VIETWAREF	24

#define CONV_CHARSET_VNIWIN		40
#define CONV_CHARSET_BKHCM2		41
#define CONV_CHARSET_VIETWAREX	42

#define CONV_TOTAL_SINGLE_CHARSETS 5
#define CONV_TOTAL_DOUBLE_CHARSETS 3


#define IS_SINGLE_BYTE_CHARSET(x) (x >= CONV_CHARSET_TCVN3 && x < CONV_CHARSET_TCVN3+CONV_TOTAL_SINGLE_CHARSETS)
#define IS_DOUBLE_BYTE_CHARSET(x) (x >= CONV_CHARSET_VNIWIN && x < CONV_CHARSET_VNIWIN+CONV_TOTAL_DOUBLE_CHARSETS)

typedef unsigned char BYTE;

int VnConvert(int inCharset, int outCharset, BYTE *input, BYTE *output, int & inLen, int & maxOutLen);
int VnFileConvert(int inCharset, int outCharset, const char *inFile, const char *outFile);

const char * VnConvErrMsg(int errCode);

enum VnConvError {
	VNCONV_NO_ERROR,
	VNCONV_UNKNOWN_ERROR,
	VNCONV_INVALID_CHARSET,
	VNCONV_ERR_INPUT_FILE,
	VNCONV_ERR_OUTPUT_FILE,
	VNCONV_OUT_OF_MEMORY,
	VNCONV_ERR_WRITING,
	VNCONV_LAST_ERROR
};

struct CharsetNameId {
	char *name;
	int id;
};

struct VnConvOptions {
	int viqrMixed;
	int viqrEsc;
	int toUpper;
	int toLower;
	int removeTone;
};

void VnConvSetOptions(VnConvOptions *pOptions);
void VnConvGetOptions(VnConvOptions *pOptions);
void VnConvResetOptions(VnConvOptions *pOptions);

extern CharsetNameId CharsetIdMap[];
extern const int CharsetCount;

#endif
