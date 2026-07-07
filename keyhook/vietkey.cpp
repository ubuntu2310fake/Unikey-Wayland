/*------------------------------------------------------------------------------
UniKey - Vietnamese Keyboard for Windows
Copyright (C) 1998-2002 Pham Kim Long
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
#include "prehdr.h"
#include <ctype.h>
#include "keycons.h"
#include "keyhook.h"
#include "vietkey.h"
#include <algorithm>
using std::max;
using std::min;

// key category constants
#define BACK_CHAR 8
#define ENTER_CHAR 13

#define BREVE_MARK 1	// for marks: ( in "a(", and also "+" in u+, o+
#define TONE_MARK 2		// tone marks, eg. a', a`,...
#define DOUBLE_KEY 3
#define SHORT_KEY 4
#define VOWEL_CHAR 5
#define SEPARATOR_KEY 6
#define VNI_DOUBLE_CHAR_MARK 7 // in VNI method
#define ESCAPE_KEY 8
#define SOFT_SEPARATOR_KEY 9

#define VNI_CIRCUMFLEX_INDEX 1
#define VNI_HORN_INDEX 2
#define VNI_BREVE_INDEX 3
#define VNI_D_INDEX 4

#define MAX_AFTER_VOWEL 2  // the max number of characters after a vowels
#define MAX_VOWEL_SQUENCE 3
#define MAX_MODIFY_LENGTH 6

extern SharedMem *pShMem;
extern unsigned char KeyState[256];

// for processing in VIQR mode
unsigned char VIQRsym[] = {'\'','`','?','~','.','^','(','+'};
//unsigned char ShortcutReverse[] = {'o', 'u', 'u', 'U'};  // We have 4 shortcuts: [,],w,W (see tcvn.cpp)
/////
unsigned char DoubleReverse[] = {'d','D','e','E','a','A','o','O'};
unsigned char WReverse[] = {'a','A','o','O','u','U'};

//------------------------------------
// Unicode composite information
//------------------------------------
// composite bases that are greater than FF
// 0: A( 0x102
// 1: a( 0x103
// 2: O+ 0x1A0
// 3: o+ 0x1A1
// 4: U+ 0x1AF
// 5: u+ 0x1B0
// 6: DD 0x110
// 7: dd 0x111
//
// Use high byte as the index to the tone table below:
// 1: '
// 2: `
// 3: ?
// 4: ~
// 5: .
WORD UnicodeCompBigBase[] = {0x102, 0x103, 0x1A0, 0x1A1, 0x1AF, 0x1B0, 0x110, 0x111};
WORD UnicodeCompTone[] = {0x301, 0x300, 0x309, 0x303, 0x323};

//-----------------------------------------------
int uniCharLen(WORD ch, int encoding)
{
	if (encoding == UNICODE_UTF8)
		return (ch < 0x0080)? 1 : ((ch < 0x0800)? 2 : 3);
	if (encoding == UNICODE_REF)
		return (ch < 128)? 1 : ((ch < 1000)? 6 : ((ch < 10000)? 7 : 8));
	if (encoding == UNICODE_HEX)
		return (ch < 256)? 1 : ((ch < 0x1000)? 7 : 8);
	if (encoding == UNICODE_CSTRING) {
		if (ch > 0x00FF && ch < 0x1000)
			return 5;
		if (ch >= 0x1000)
			return 6;
		if (isxdigit(ch))
			return 4;
		return 1;
	}
//		return (ch < 128)? 1 : ((ch < 0x100)? 6 : ((ch < 0x1000)? 7 : 8));
	return 2;
}

VietKey::VietKey() {
	keys = 0;
	DT = 0;
	BD = 0;
	BK = 0;
	BW = 0;
	BT = 0;
	lastWConverted = 0;
	lastIsEscape = 0;
	tempVietOff = 0;
}

unsigned char ChangedChar, OldChar;

//-----------------------------------------------
// puts char into buffer without processing it
//-----------------------------------------------
void VietKey::putChar(unsigned char ch, int isLowerCase)
{
	if (keys==KEY_BUFSIZE) 
		throwBuf();
	lowerCase[keys] = isLowerCase; //(OldChar);
	buf[keys] = ch;
	keys++;
}

//-----------------------------------------------
void VietKey::process(unsigned char c)
{
	int kieu;

	keysPushed = 0;
	backs = 0;
	OldChar = c;
	int thisWConverted = 0;


	if (pShMem->codeTable.multiBytes)
		c = tolower(c);


	kieu = keyCategory(c);

	if (!pShMem->vietKey && pShMem->options.alwaysMacro) {
		//vietnamese is off, but macro is enabled
		if (pShMem->options.macroEnabled &&	
			((kieu == SOFT_SEPARATOR_KEY && ATTR_IS_WORD_STOP(DT[c])) || c == ENTER_CHAR)) {
			if (checkMacro(OldChar))
				return;
		}
		if (kieu == SEPARATOR_KEY) {
			if (c == BACK_CHAR)
				processBackspace();
			else
				clearBuf();
		}
		else putChar(c, islower(OldChar));

		return;
	}


	if (lastIsEscape && keys > 0 && kieu != SEPARATOR_KEY && kieu != SOFT_SEPARATOR_KEY) {
		backs = 1;
		ChangedChar = buf[keys-1];
		buf[keys-1] = c;
		lowerCase[keys-1] = islower(OldChar);
		ansiPush[keysPushed++] = c;
		postProcess();
		lastIsEscape = 0;
		lastWConverted = 0;
		return;
	}
	lastIsEscape = 0;

	if (pShMem->options.macroEnabled &&	
		((kieu == SOFT_SEPARATOR_KEY && ATTR_IS_WORD_STOP(DT[c])) || c == ENTER_CHAR)) {
		if (checkMacro(OldChar))
			return;
	}

	if (tempVietOff) {
		if (!isalpha(c))
			tempVietOff = 0;
		if (kieu == SEPARATOR_KEY) {
			if (c == BACK_CHAR)
				processBackspace();
			else
				clearBuf();
		}
		else putChar(c, islower(OldChar));
		lastWConverted = 0;
		return;
	}

	switch(kieu){
	case BREVE_MARK:
		if (pShMem->inMethod == TELEX_INPUT && lastWConverted && (c == 'w' || c == 'W'))
			shortKey(c);
		else {
			putBreveMark(c);
			if (pShMem->inMethod == TELEX_INPUT && keysPushed == 0 && backs == 0 && (c == 'w' || c == 'W')) {
				shortKey(c); // Special case for W key in TELEX mode.
				thisWConverted = 1;
			}
		}
		break;
	case DOUBLE_KEY:
		doubleChar(c);
		break;
	case TONE_MARK:
		putToneMark(c);
		break;
	case SHORT_KEY:
		shortKey(c);
		break;
	case VNI_DOUBLE_CHAR_MARK:
		vniDoubleCharMark(c);
		break;
	case ESCAPE_KEY:
		lastIsEscape = 1;
		break;
	case SEPARATOR_KEY:
		if (c == BACK_CHAR)
			processBackspace();
		else
			clearBuf();

		lastWConverted = 0;
		return;
	/*
	case SOFT_SEPARATOR_KEY:
		if (pShMem->options.macroEnabled) {
			if (checkMacro(OldChar))
				return;
		}
		break;
	*/
	}
	lastWConverted = thisWConverted;

	if (keysPushed==0 && backs==0) {
		if (pShMem->inMethod != VIQR_INPUT && 
			pShMem->keyMode == VIQR_CHARSET &&
			checkEscapeVIQR(OldChar))
			return;
		putChar(c, islower(OldChar));
	}

	if (keysPushed == 0) {
		if (backs == 0 && pShMem->keyMode== UNICODE_CHARSET && 
			(pShMem->codeTable.encoding==UNICODE_CSTRING || pShMem->codeTable.encoding==UNICODE_UTF8)) {
			keysPushed = 1;
			ansiPush[0] = c;
		}
		else return;
	}

	if (keys < keysPushed && pShMem->codeTable.multiBytes) {
		keysPushed = 0;
		backs = 0;
		return;
	}

	postProcess();
}

static unsigned char tempPush[KEY_BUFSIZE*8];

void VietKey::processBackspace()
{
	if (keys <= 0)
		return;

	unsigned char t;
	WORD mapping;

	backs = 1;
	if (pShMem->keyMode == VIQR_CHARSET) {
		mapping = lowerCase[keys-1]? ToUniL[buf[keys-1]] : ToUniH[buf[keys-1]];
		if (HIBYTE(mapping))
			backs++;
		t = LOBYTE(mapping);
		mapping = lowerCase[keys-1]? ToUniL[t] : ToUniH[t];
		if (HIBYTE(mapping))
			backs++;
	}
	else if (pShMem->keyMode == UNICODE_CHARSET){
		if (!pShMem->codeTable.singleBackspace) {
			mapping = (lowerCase[keys-1])? ToUniL[buf[keys-1]] : ToUniH[buf[keys-1]];
			if (pShMem->codeTable.encoding != UNICODE_UCS2)
				backs += uniCharLen(mapping, pShMem->codeTable.encoding) - 1;
		}
	}
	else if (pShMem->codeTable.multiBytes) {
		mapping = (lowerCase[keys-1])? ToUniL[buf[keys-1]] : ToUniH[buf[keys-1]];
		if (HIBYTE(mapping))
			backs++;
	}
	keys--;
}

//-------------------------------------
void VietKey::postProcess()
{
	unsigned char t;
	int i, j, k;
	WORD mapping, map2;

	if (pShMem->keyMode == UNICODE_CHARSET) {
		if (pShMem->codeTable.encoding == UNICODE_UCS2) {
			for (i = 0, j = keys-keysPushed; i < keysPushed; i++, j++)
				uniPush[i] = (lowerCase[j]) ? ToUniL[ansiPush[i]] : ToUniH[ansiPush[i]];
		}
		else 
			encodeUnicode(pShMem->codeTable.encoding);
		return;
	}

	if (pShMem->keyMode == VIQR_CHARSET) {
		memcpy(tempPush, ansiPush, keysPushed);
		for (i = k = 0, j=keys-keysPushed; i<keysPushed; i++, j++) {
			mapping = lowerCase[j]? ToUniL[tempPush[i]] : ToUniH[tempPush[i]];
			t = LOBYTE(mapping);
			map2 = lowerCase[j]? ToUniL[t] : ToUniH[t];
			ansiPush[k++] = LOBYTE(map2);
			if (HIBYTE(map2)) {
				ansiPush[k++] = HIBYTE(map2);
				if (backs > 0 && i > 0)
					backs++;
			}
			if (HIBYTE(mapping)) {
				ansiPush[k++] = HIBYTE(mapping);
				if (backs > 0 && i > 0) // if backs was already = 0, we never have to push any more back
					backs++;
			}
		}
		// determine the number of backs
		if (backs > 0) {
			// check the changed char
			mapping = lowerCase[keys-keysPushed]? ToUniL[ChangedChar] : ToUniH[ChangedChar];
			if (HIBYTE(mapping))
				backs++;
			t = LOBYTE(mapping);
			mapping = lowerCase[keys-keysPushed]? ToUniL[t] : ToUniH[t];
			if (HIBYTE(mapping))
				backs++;
		}
		keysPushed = k;
		return;
	}

	if (pShMem->keyMode == DECOMPOSED_UNICODE_CHARSET) {
		WORD base, tone;
		for (i = k = 0, j=keys-keysPushed; i<keysPushed; i++, j++) {
			mapping = lowerCase[j]? ToUniL[ansiPush[i]] : ToUniH[ansiPush[i]];
			base = LOBYTE(mapping);
			if (base < 0x20)
				base = UnicodeCompBigBase[base];
			uniPush[k++] = base;

			tone = HIBYTE(mapping);
			if (tone > 0) {
				uniPush[k++] = UnicodeCompTone[tone-1];
				if (backs > 0 && i > 0) // if backs was already = 0, we never have to push any more back
					backs++;
			}
		}

		// determine the number of backs
		if (backs > 0) {
			mapping = lowerCase[keys-keysPushed]? ToUniL[ChangedChar] : ToUniH[ChangedChar];
			if (HIBYTE(mapping))
				backs++;
		}
		keysPushed = k;
		return;
	}

	if (pShMem->codeTable.multiBytes) {
		memcpy(tempPush, ansiPush, keysPushed);
		for (i = k = 0, j=keys-keysPushed; i<keysPushed; i++, j++) {
			mapping = lowerCase[j]? ToUniL[tempPush[i]] : ToUniH[tempPush[i]];
			ansiPush[k++] = LOBYTE(mapping);
			if (HIBYTE(mapping)) {
				ansiPush[k++] = HIBYTE(mapping);
				if (backs > 0 && i > 0) // if backs was already = 0, we never have to push any more back
					backs++;
			}
		}
		// determine the number of backs
		if (backs > 0) {
			mapping = lowerCase[keys-keysPushed]? ToUniL[ChangedChar] : ToUniH[ChangedChar];
			if (HIBYTE(mapping))
				backs++;
		}
		keysPushed = k;
		return;
	}
}

//-------------------------------------
void VietKey::clearBuf()
{
	keys = 0;
	lastWConverted = 0;
	lastIsEscape = 0;
	tempVietOff = 0;
}


//--------------------------------
int VietKey::keyCategory(unsigned char c)
{
	long index, attr;
	attr = DT[c];

	if (ATTR_IS_BREVE(attr) > 0)
		return BREVE_MARK;

	if (ATTR_TONE_INDEX(attr) > 0)
		return TONE_MARK;

	index = ATTR_DBCHAR_INDEX(attr);
	if ((pShMem->inMethod == TELEX_INPUT || pShMem->inMethod == TELEX_SIMPLIFIED_INPUT) && index > 0 && index < 9)
		return DOUBLE_KEY;
	if (pShMem->inMethod != TELEX_INPUT && pShMem->inMethod != TELEX_SIMPLIFIED_INPUT && ATTR_VNI_DOUBLE_INDEX(attr) > 0)
		return VNI_DOUBLE_CHAR_MARK;

	if (ATTR_MACRO_INDEX(attr) > 0)
		return SHORT_KEY;

	if (ATTR_IS_SEPARATOR(attr) > 0)
		return SEPARATOR_KEY;

	if ((pShMem->inMethod == VIQR_INPUT || pShMem->inMethod == VIQR_STAR_INPUT) && c == '\\')
		return ESCAPE_KEY;

	if (ATTR_IS_SOFT_SEPARATOR(attr))
		return SOFT_SEPARATOR_KEY;
	return 0;
}

//--------------------------------
void VietKey::putBreveMark(unsigned char c)
{
	int i, k;
	long attr;
	unsigned char t, newChar;
	int leftMost;

	int index, index_c, toneIndex = 0;
	// determine the position to drop the breve mark
	i = keys-1;
	if (pShMem->inMethod != TELEX_INPUT && pShMem->inMethod != TELEX_SIMPLIFIED_INPUT)
		index_c = ATTR_VNI_DOUBLE_INDEX(DT[c]);
	leftMost = pShMem->options.freeMarking ? 0 : keys-1;
	leftMost = max(leftMost, keys - MAX_MODIFY_LENGTH);

	while (i >= leftMost) {
		attr = DT[buf[i]];
		toneIndex = ATTR_CURRENT_TONE(DT[buf[i]]);
		if (toneIndex == 0 || toneIndex == 6)
			index = ATTR_DBCHAR_INDEX(attr);
		else
			index = ATTR_DBCHAR_INDEX(DT[BD[ATTR_VOWEL_INDEX(attr)-1][5]]);
		if (index > 4) { // from 11 are the indexes of VNI double-charater marks
			if (pShMem->inMethod != TELEX_INPUT && pShMem->inMethod != TELEX_SIMPLIFIED_INPUT) {
				if ((index_c == VNI_HORN_INDEX && index > 6) ||
					(index_c == VNI_BREVE_INDEX && index <= 6))
					break;
			}
			else 
				break;
		}
		else if (ATTR_IS_SEPARATOR(attr)  || ATTR_IS_SOFT_SEPARATOR(attr))
			break;
		i--;
	}
	if (i < leftMost || (index <= 4))
		return;

	if (pShMem->options.freeMarking && i > 0) {
		unsigned char prevChar;
		int tmpIdx;
		if (ATTR_VOWEL_INDEX(DT[prevChar = buf[i-1]]) > 0)
			prevChar = BD[ATTR_VOWEL_INDEX(DT[prevChar])-1][5];

		tmpIdx = ATTR_DBCHAR_INDEX(DT[prevChar]);
		if (tmpIdx > 4)
			prevChar = WReverse[tmpIdx-4-1];
		if ((prevChar == 'o' || prevChar == 'O' || prevChar == 'u' || prevChar == 'U') && 
			(buf[i] == 'u' || buf[i] == 'U')) {
			i--;
			toneIndex = ATTR_CURRENT_TONE(DT[buf[i]]);
		}

		if (i > 0) {
			if (ATTR_VOWEL_INDEX(DT[prevChar = buf[i-1]]) > 0)
				prevChar = BD[ATTR_VOWEL_INDEX(DT[prevChar])-1][5];
			if ((prevChar == 'U' || prevChar == 'u') &&
				(i == 1 || (i > 1 && buf[i-2] != 'q' && buf[i-2] != 'Q'))) {
				t = buf[i];
				if (ATTR_VOWEL_INDEX(DT[t]) > 0)
					t = BD[ATTR_VOWEL_INDEX(DT[t])-1][5];
				tmpIdx = ATTR_DBCHAR_INDEX(DT[t]) - 4;
				t = toupper(t);
				if ((t == 'A' && (pShMem->inMethod == TELEX_INPUT || pShMem->inMethod == TELEX_SIMPLIFIED_INPUT)) ||
					((t == 'O' || WReverse[tmpIdx-1] == 'o' || WReverse[tmpIdx-1] == 'O') && i != keys-1)) {
					i--;
					toneIndex = ATTR_CURRENT_TONE(DT[buf[i]]);
				}
			}
		}
	}

	if (toneIndex == 0 || toneIndex == 6) {
		index = ATTR_DBCHAR_INDEX(DT[buf[i]]) - 4;
		newChar = BW[index - 1];
	}
	else {
		index = ATTR_DBCHAR_INDEX(DT[BD[ATTR_VOWEL_INDEX(DT[buf[i]])-1][5]]) - 4;
		newChar = BD[ATTR_VOWEL_INDEX(DT[BW[index-1]])-1][toneIndex-1];
	}
	if (newChar != buf[i]) {
		if (pShMem->keyMode == VIQR_CHARSET) {
			if ((buf[i]=='a' && c==VIQRsym[7]) || (buf[i] != 'a' && c == VIQRsym[6]))
				return;
		}

		bool modify_u = false;
		unsigned char newU = 0;
		bool modify_o = false;
		unsigned char newO = 0;
		if ((pShMem->inMethod == TELEX_INPUT || pShMem->inMethod == TELEX_SIMPLIFIED_INPUT) && (c == 'w' || c == 'W')) {
			if (i > 0) {
				unsigned char prev = buf[i-1];
				if ((prev == 'u' || prev == 'U') && (buf[i] == 'o' || buf[i] == 'O')) {
					modify_u = true;
					int u_toneIndex = ATTR_CURRENT_TONE(DT[prev]);
					int u_index;
					if (u_toneIndex == 0 || u_toneIndex == 6) {
						u_index = ATTR_DBCHAR_INDEX(DT[prev]) - 4;
						newU = BW[u_index - 1];
					} else {
						u_index = ATTR_DBCHAR_INDEX(DT[BD[ATTR_VOWEL_INDEX(DT[prev])-1][5]]) - 4;
						newU = BD[ATTR_VOWEL_INDEX(DT[BW[u_index-1]])-1][u_toneIndex-1];
					}
				}
			}
			if (i < keys - 1) {
				unsigned char next = buf[i+1];
				if ((buf[i] == 'u' || buf[i] == 'U') && (next == 'o' || next == 'O')) {
					modify_o = true;
					int o_toneIndex = ATTR_CURRENT_TONE(DT[next]);
					int o_index;
					if (o_toneIndex == 0 || o_toneIndex == 6) {
						o_index = ATTR_DBCHAR_INDEX(DT[next]) - 4;
						newO = BW[o_index - 1];
					} else {
						o_index = ATTR_DBCHAR_INDEX(DT[BD[ATTR_VOWEL_INDEX(DT[next])-1][5]]) - 4;
						newO = BD[ATTR_VOWEL_INDEX(DT[BW[o_index-1]])-1][o_toneIndex-1];
					}
				}
			}
		}

		if (modify_u) {
			backs = keys - i + 1;
			ChangedChar = buf[i-1];
			ansiPush[keysPushed++] = buf[i-1] = newU;
			ansiPush[keysPushed++] = buf[i] = newChar;
			for (k = i+1; k < keys; k++)
				ansiPush[keysPushed++] = buf[k];
		} else if (modify_o) {
			backs = keys - i;
			ChangedChar = buf[i];
			ansiPush[keysPushed++] = buf[i] = newChar;
			ansiPush[keysPushed++] = buf[i+1] = newO;
			for (k = i+2; k < keys; k++)
				ansiPush[keysPushed++] = buf[k];
		} else {
			backs = keys - i;
			ChangedChar = buf[i];
			ansiPush[keysPushed++] = buf[i] = newChar;
			for (k = i+1; k < keys; k++)
				ansiPush[keysPushed++] = buf[k];
		}
	}
	else {
		// duplicate, convert back to english
		backs = keys - i;
		ChangedChar = buf[i];
		if (toneIndex == 0 || toneIndex == 6)
			ansiPush[keysPushed++] = buf[i] = WReverse[index-1];
		else
			ansiPush[keysPushed++] = buf[i] = BD[ATTR_VOWEL_INDEX(DT[WReverse[index-1]])-1][toneIndex-1];
		for (k = i+1; k < keys; k++)
			ansiPush[keysPushed++] = buf[k];
		putChar(c, islower(OldChar));
		ansiPush[keysPushed++] = c;
		tempVietOff = 1;
	}
}

//--------------------------------
void VietKey::doubleChar(unsigned char c)
{
	int i, k, leftMost;
	long attr;
	unsigned char newChar;

	int index, index_c, toneIndex = 0;
	// determine the position to drop the cirumflex mark
	i = keys-1;
	if (pShMem->inMethod != TELEX_INPUT && pShMem->inMethod != TELEX_SIMPLIFIED_INPUT)
		index_c = ATTR_VNI_DOUBLE_INDEX(DT[c]);
	else
		index_c = ATTR_DBCHAR_INDEX(DT[c]);
	leftMost = pShMem->options.freeMarking ? 0 : keys -1;
	leftMost = max(leftMost, keys - MAX_MODIFY_LENGTH);

	while (i >= leftMost) {
		attr = DT[buf[i]];
		toneIndex = ATTR_CURRENT_TONE(DT[buf[i]]);
		if (toneIndex == 0 || toneIndex == 6)
			index = ATTR_DBCHAR_INDEX(attr);
		else
			index = ATTR_DBCHAR_INDEX(DT[BD[ATTR_VOWEL_INDEX(attr)-1][5]]);
		if (index > 0 && index < 9) {
			if (pShMem->keyMode == VIQR_CHARSET && c == VIQRsym[5])
				break;
			if (pShMem->inMethod != TELEX_INPUT && pShMem->inMethod != TELEX_SIMPLIFIED_INPUT) {
				if ( (index_c == VNI_CIRCUMFLEX_INDEX && index > 2) ||
					 (index_c == VNI_D_INDEX && index <= 2) )
					break;
			}
			else if (index  == index_c)
				break;
		}
		else if (ATTR_IS_SEPARATOR(attr)  || ATTR_IS_SOFT_SEPARATOR(attr)) {
			break;
		}
		i--;
	}
	if (i < leftMost || index == 0 || index >= 9)
		return;

	if (pShMem->keyMode == VIQR_CHARSET && c==VIQRsym[5] && (buf[i] == 'd' || buf[i] == 'D'))
		return;

	// if oeo, o must not be understood as a double character
	if ((pShMem->inMethod == TELEX_INPUT || pShMem->inMethod == TELEX_SIMPLIFIED_INPUT) && toupper(c) == 'O' && i < keys-1) {
		unsigned char ch = buf[i+1];
		int vowelIndex = ATTR_VOWEL_INDEX(DT[ch]);
		if (vowelIndex > 0 && BD[vowelIndex-1][5] == 'e')
			return;
	}


	if (toneIndex == 0 || toneIndex == 6)
		newChar = BK[index-1];
	else
		newChar = BD[ATTR_VOWEL_INDEX(DT[BK[index-1]])-1][toneIndex-1];

	if (newChar != buf[i]) {
		backs = keys - i;
		ChangedChar = buf[i];
		ansiPush[keysPushed++] = buf[i] = newChar;
		for (k = i+1; k < keys; k++)
			ansiPush[keysPushed++] = buf[k];
	}
	else {
		// back to english
		backs = keys - i;
		ChangedChar = buf[i];
		if (toneIndex == 0 || toneIndex == 6)
			ansiPush[keysPushed++] = buf[i] = DoubleReverse[index-1];
		else
			ansiPush[keysPushed++] = buf[i] = BD[ATTR_VOWEL_INDEX(DT[DoubleReverse[index-1]])-1][toneIndex-1];
		for (k = i+1; k < keys; k++)
			ansiPush[keysPushed++] = buf[k];
		putChar(c, islower(OldChar));
		ansiPush[keysPushed++] = c;
		tempVietOff = 1;
	}
}


//--------------------------------
void VietKey::shortKey(unsigned char c)
{
	int lower;
	if (isupper(OldChar) || c == '{' || c == '}') {
		lower = 0;
	} else {
		lower = 1;
	}

	unsigned char newChar;
	int duplicate;
	int index = ATTR_MACRO_INDEX(DT[c]);
	newChar = BT[index-1];

	if (pShMem->codeTable.multiBytes) {
		// hard-coded!!!!
		if (c == '{')
			newChar = BT[0];
		else if (c == '}')
			newChar = BT[1];
	}

	keysPushed = 0;

	duplicate = (keys > 0) && (buf[keys-1] == newChar);
	if (duplicate) {
		// convert back to english
		ChangedChar = buf[keys-1];
		buf[keys-1] = c;
		ansiPush[keysPushed++] = c;
		backs = 1;
		tempVietOff = 1;
		return;
	}

	backs = 0;
	ansiPush[keysPushed++] = newChar;
	if (keys == KEY_BUFSIZE) 
		throwBuf();

	lowerCase[keys] = lower;
	buf[keys] = newChar;
	keys++;
}


//--------------------------------
void VietKey::putToneMark(unsigned char c)
{
	int i,k,l,cuoi,index,vowel, duplicate, leftMost;
	unsigned char newChar,t;
	long attr;

	// Tim nguyen am dau tien ke tu ben phai
	i = keys-1;
	leftMost = (pShMem->options.toneNextToVowel)? i : 0;
	leftMost = max(keys - 1 - MAX_AFTER_VOWEL, leftMost);
	while (i >= leftMost) {
		attr = DT[buf[i]];
		if (ATTR_IS_SEPARATOR(attr) || ATTR_IS_SOFT_SEPARATOR(attr) || ATTR_VOWEL_INDEX(attr))
			break;
		i--;
	}
	if (i < leftMost || ATTR_VOWEL_INDEX(attr) == 0)
		return;

	// Tim day cac nguyen am lien tiep
	// neu gap 1 nguyen am da co dau thi dung
	cuoi = i;
	leftMost = (pShMem->options.toneNextToVowel)? i : 0;
	leftMost = max(cuoi - MAX_VOWEL_SQUENCE + 1, leftMost);
	while (i >= leftMost && ATTR_VOWEL_INDEX(DT[buf[i]])
				&& ((buf[i]<='z' && buf[i]>='a')
					|| (buf[i]<='Z' && buf[i]>='A')))
		i--;

	if (i<leftMost || ATTR_VOWEL_INDEX(DT[buf[i]])==0) {
		l = cuoi-i; // l la do dai day nguyen am
		switch (l) {
		case 2:
			if (pShMem->options.modernStyle &&
				( (buf[cuoi-1] == 'o' && buf[cuoi] == 'a') ||
				  (buf[cuoi-1] == 'o' && buf[cuoi] == 'e') ||
				  (buf[cuoi-1] == 'u' && buf[cuoi] == 'y') ))
			   i = cuoi;
			else {
				t = toupper(buf[i]);
				if (i>=0 && (t=='Q' || (t=='G' && toupper(buf[i+1])=='I')))
					i = cuoi;
				else if (keys>cuoi+1) 
					i = cuoi; // co phu am di sau buf[cuoi]
				else 
					i = cuoi-1;
			}
			break;
		case 3:
			i = cuoi - 1;
			break;
		default:
			i = cuoi;
		}
	}
	vowel = ATTR_VOWEL_INDEX(DT[buf[i]]) - 1;

	if (c>=5) 
		index = ATTR_TONE_INDEX(DT[c]) - 1;
	else 
		index = c;

	newChar = BD[vowel][index];
	duplicate = (newChar == buf[i]);
	if (duplicate)
		newChar = BD[vowel][5];

	if (duplicate && (index == 5 || (pShMem->keyMode == VIQR_CHARSET && c < 5)))
		return;
	backs = keys - i;
	ChangedChar = buf[i];
	buf[i] = ansiPush[keysPushed++] = newChar;
	for (k=1; k<keys-i; k++) 
		ansiPush[keysPushed++] = buf[i+k];
	if (duplicate) {
		ansiPush[keysPushed++] = c;
		putChar(c, islower(OldChar));
		tempVietOff = 1;
	}
}

//--------------------------------
void VietKey::setCodeTable(CodeInfo * pInfo)
{
	DT = pInfo->DT;
	BD = pInfo->BD;
	BK = pInfo->BK;
	BW = pInfo->BW;
	BT = pInfo->BT;
	ToUniH = pInfo->ToUniH;
	ToUniL = pInfo->ToUniL;
}

//--------------------------------
void VietKey::vniDoubleCharMark(unsigned char c)
{
	long index;//, n;
	if (keys == 0)
		return;
	index = ATTR_VNI_DOUBLE_INDEX(DT[c]); // in VNI method, index is from 11 to 14
	switch (index) {
	case VNI_CIRCUMFLEX_INDEX:
	case VNI_D_INDEX: 
		doubleChar(c);
		break;
	case VNI_HORN_INDEX: // mark + for u, o
	case VNI_BREVE_INDEX: // mark ( for a
		putBreveMark(c);
		break;
	}
}


//--------------------------------
void VietKey::throwBuf()
{
	memmove(buf, &buf[keys-KEYS_MAINTAIN], KEYS_MAINTAIN * sizeof(buf[0]));
	memmove(lowerCase, &lowerCase[keys-KEYS_MAINTAIN], KEYS_MAINTAIN * sizeof(lowerCase[0]));
	keys = KEYS_MAINTAIN;
}

//--------------------------------
unsigned char *putUnicodeCharRef(unsigned char *buf, WORD ch, int & len)
{
//	if (ch < 256) {
	if (ch < 128) {
		len = 1;
		*buf++ = (unsigned char)ch;
	}
	else {
		*buf++ = '&';
		*buf++ = '#';
		len = 2;
		int i, digit, prev, base;
		prev = 0;
		base = 10000;
		for (i=0; i < 5; i++) {
			digit = ch / base;
			if (digit || prev) {
				prev = 1;
				*buf++ = '0' + ((unsigned char)digit);
				len++;
			}
			ch %= base;
			base /= 10;
		}
		*buf++ = ';';
		len++;
	}
	return buf;
}

#define HEX_DIGIT(x) ((x < 10)? ('0'+x) : ('A'+x-10))

//--------------------------------
unsigned char *putUnicodeCharHex(unsigned char *buf, WORD ch, int & len)
{
//	if (ch < 128) {
	if (ch < 256) {
		len = 1;
		*buf++ = (unsigned char)ch;
	}
	else {
		*buf++ = '&';
		*buf++ = '#';
		*buf++ = 'x';
		len = 3;

		int i, digit;
		int prev = 0;
		int shifts = 12;
		for (i=0; i < 4; i++) {
			digit = ((ch >> shifts) & 0x000F);
			if (digit > 0 || prev) {
				prev = 1;
				*buf++ = HEX_DIGIT(digit);
				len++;
			}
			shifts -= 4;
		}
		*buf++ = ';';
		len++;
	}
	return buf;
}

//-----------------------------------------------------
unsigned char *putUnicodeCharCString(unsigned char *buf, WORD ch, int & len)
{
	if (ch < 256 && !isxdigit(ch)) {
		len = 1;
		*buf++ = (unsigned char)ch;
	}
	else {
		*buf++ = '\\';
		*buf++ = 'x';

		len = 2;

		int i, digit;
		int prev = 0;
		int shifts = 12;
		for (i=0; i < 4; i++) {
			digit = ((ch >> shifts) & 0x000F);
			if (digit > 0 || prev) {
				prev = 1;
				*buf++ = HEX_DIGIT(digit);
				len++;
			}
			shifts -= 4;
		}
	}
	return buf;
}
//--------------------------------
unsigned char *putUnicodeCharUtf8(unsigned char *buf, WORD ch, int & len)
{
	if (ch < 0x0080) {
		len = 1;
		*buf++ = (unsigned char)ch;
	} else if (ch < 0x0800) {
		len = 2;
		*buf++ = (0xC0 | (BYTE)(ch >> 6));
		*buf++ = (0x80 | (BYTE)(ch & 0x003F));
	} else {
		len = 3;
		*buf++ = (0xE0 | (BYTE)(ch >> 12));
		*buf++ = (0x80 | (BYTE)((ch >> 6) & 0x003F));
		*buf++ = (0x80 | (BYTE)(ch & 0x003F));
	}
	return buf;	
}

//--------------------------------
void VietKey::encodeUnicode(int encoding)
{
	int i, j;
	memcpy(tempPush, ansiPush, keysPushed);
	WORD w;
	int len, bytes;
	BYTE *p = ansiPush;
	bytes = 0;

	for (i = 0, j = keys-keysPushed; i < keysPushed; i++, j++) {
		w = (lowerCase[j]) ? ToUniL[tempPush[i]] : ToUniH[tempPush[i]];
		if (encoding == UNICODE_REF)
			p = putUnicodeCharRef(p, w, len);
		else if (encoding == UNICODE_HEX)
			p = putUnicodeCharHex(p, w, len);
		else if (encoding == UNICODE_CSTRING)
			p = putUnicodeCharCString(p, w, len);
		else
			p = putUnicodeCharUtf8(p, w, len);
		bytes += len;

		if (!pShMem->codeTable.singleBackspace && backs > 0 && i > 0)
			backs += len-1;
	}

	if (!pShMem->codeTable.singleBackspace && backs > 0) {
		w = lowerCase[keys-keysPushed]? ToUniL[ChangedChar] : ToUniH[ChangedChar];
		len = uniCharLen(w, encoding);
		backs += len - 1;
	}

	keysPushed = bytes;
}

//--------------------------------
int macroKeyCompare(const void *ele1, const void *ele2)
{
	char *key = pShMem->macroMem + ((HookMacroDef *)ele2)->keyOffset;
	return strcmp((const char *)ele1, key);
//	return strcmp((const char *)ele1, ((MacroDef *)ele2)->key);
}

//--------------------------------
int VietKey::checkMacro(unsigned char lastChar)
{
	if ((KeyState[VK_SHIFT] & 0x80) && lastChar==' ')
		return 0;  //SHIFT+SPACEBAR: disable macro

	// get the macro key
	HookMacroDef *pMacro = NULL;
	char key[MAX_MACRO_KEY_LEN+1];
	int i, j, keyLen;
	i = keys-1;

	while (i >= 0) {
		while (i>=0 && !ATTR_IS_WORD_STOP(DT[buf[i]]) && (keys-i <= MAX_MACRO_KEY_LEN - 1))
			i--;
		if (i>=0 && !ATTR_IS_WORD_STOP(DT[buf[i]]))
			return 0;

		if (i>=0)
			key[0] = buf[i];

		for (j=i+1; j<keys; j++) {
			if (lowerCase[j])
				key[j-i] = buf[j];
			else
				key[j-i] = toupper(buf[j]);
		}
		key[keys-i] = '\0';

		//search macro table
		keyLen = keys-i-1;
		pMacro = (HookMacroDef *)bsearch(key+1, pShMem->macroTable, 
									pShMem->macroCount, sizeof(HookMacroDef), 
									macroKeyCompare);
		if (pMacro)
			break;
		if (i>=0) {
			keyLen = keys-i;
			pMacro = (HookMacroDef *)bsearch(key, pShMem->macroTable, 
									pShMem->macroCount, sizeof(HookMacroDef), 
									macroKeyCompare);
		}
		if (pMacro || keys-i > MAX_MACRO_KEY_LEN-1)
			break;
		i--;
	}
	if (!pMacro) {
		return 0;
	}

	backs = keyLen;

	if (pShMem->keyMode == UNICODE_CHARSET && pShMem->codeTable.encoding==UNICODE_CSTRING) {
		backs = 0;
		for (int l=0; l<keyLen; l++) {
			if (isxdigit(buf[keys-1-l]))
				backs += 4;
			else
				backs += 1;
		}
	}

	if (pShMem->keyMode == UNICODE_CHARSET) {
		WORD *p = (WORD *)(pShMem->macroMem + pMacro->textOffset);
		if (pShMem->codeTable.encoding == UNICODE_UCS2) {
			for (keysPushed=0; p[keysPushed]; keysPushed++) {
				uniPush[keysPushed] = p[keysPushed];
				//ansiPush[keysPushed] = '?';
			}
			WideCharToMultiByte(CP_US_ANSI, 0, p, -1, (char *)ansiPush, sizeof(ansiPush), NULL, NULL);
			uniPush[keysPushed] = lastChar;
			ansiPush[keysPushed] = lastChar;
			keysPushed++;
		}
		else {
			BYTE *buf = (BYTE *)ansiPush;
			WORD w;
			int len;
			for (i=0, keysPushed = 0; (w = p[i]) != 0; i++) {
				switch (pShMem->codeTable.encoding) {
				case UNICODE_REF:
					buf = putUnicodeCharRef(buf, w, len);
					break;
				case UNICODE_HEX:
					buf = putUnicodeCharHex(buf, w, len);
					break;
				case UNICODE_CSTRING:
					buf = putUnicodeCharCString(buf, w, len);
					break;
				default:
					buf = putUnicodeCharUtf8(buf, w, len);
				}
				keysPushed += len;
			}
			ansiPush[keysPushed++] = lastChar;
		}
	}
	else if (pShMem->keyMode == DECOMPOSED_UNICODE_CHARSET) {
		WORD *p = (WORD *)(pShMem->macroMem + pMacro->textOffset);
		for (keysPushed=0; p[keysPushed]; keysPushed++)
			uniPush[keysPushed] = p[keysPushed];
		uniPush[keysPushed++] = lastChar;
	}
	else {
		char *p = pShMem->macroMem + pMacro->textOffset;
		for (keysPushed=0; p[keysPushed]; keysPushed++)
			ansiPush[keysPushed] = p[keysPushed];
		ansiPush[keysPushed++] = lastChar;
	}
	clearBuf();
	return 1;
}

//-------------------------------------------
int VietKey::checkEscapeVIQR(unsigned char ch)
{
	if (keys <= 0)
		return 0;
	int escape = 0;
	unsigned char prevCh = toupper(buf[keys-1]);
	if (ch == '^')
		escape = (prevCh == 'A' || prevCh == 'O');
	else if (ch == '(')
		escape = (prevCh == 'A');
	else if (ch == '+')
		escape = (prevCh == 'O' || prevCh == 'U');
	else if (ch == '\'' || ch == '`' || ch == '?' || ch == '~' || ch == '.') {
		long attr, vowelIndex, currentTone;
		attr = DT[buf[keys-1]];
		vowelIndex = ATTR_VOWEL_INDEX(attr);
		currentTone = ATTR_CURRENT_TONE(attr);
		escape = (vowelIndex > 0 && currentTone == 6);
	}
	if (escape) {
		putChar('\\');
		putChar(ch);
		backs = 0;
		keysPushed = 2;
		ansiPush[0] = '\\';
		ansiPush[1] = ch;
	}
	return escape;
}