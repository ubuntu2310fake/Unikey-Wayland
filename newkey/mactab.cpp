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
#include <stdio.h>
#include "mactab.h"

//---------------------------------------------------------------
CMacroTable::CMacroTable(int memSize)
{
	m_count = 0;
	m_table = new MacroDef[MAX_MACRO_ITEMS];
	m_macroMem = new char[MACRO_MEM_SIZE];
	m_memSize = memSize;
	m_occupied = 0;
}

//---------------------------------------------------------------
CMacroTable::~CMacroTable()
{
	delete [] m_macroMem;
	delete [] m_table;
}

//---------------------------------------------------------------
int CMacroTable::loadFromFile(const TCHAR *fname)
{
	FILE *f = _tfopen(fname, _TEXT("rt"));
	if (f == NULL) 
		return 0;
	char line[MAX_MACRO_LINE];
	int len;
	while (!feof(f)) {
		fgets(line, sizeof(line), f);
		len = strlen(line);
		if (line[len-1] == '\n')
			line[len-1] = '\0';
		addItem(line);
	}
	fclose(f);
	return 1;
}

//---------------------------------------------------------------
int CMacroTable::writeToFile(const TCHAR *fname)
{
	FILE *f = _tfopen(fname, _TEXT("wt"));
	if (f == NULL)
		return 0;

	char line[MAX_MACRO_LINE];
	for (int i=0; i < m_count; i++) {
		if (i < m_count-1)
			sprintf(line, "%s:%s\n", m_table[i].key, m_table[i].text);
		else
			sprintf(line, "%s:%s", m_table[i].key, m_table[i].text);
		fputs(line, f);
	}
	fclose(f);
	return 1;
}

//---------------------------------------------------------------
int CMacroTable::addItem(const char *key, const char *text)
{
	if (m_count >= MAX_MACRO_ITEMS)
		return -1;

	int keyLen, textLen;
	keyLen = strlen(key);
	if (keyLen == 0)
		return -1;
	if (keyLen > MAX_MACRO_KEY_LEN -1)
		keyLen = MAX_MACRO_KEY_LEN - 1;

	textLen = strlen(text);
	if (textLen == 0)
		return -1;
	if (textLen > MAX_MACRO_TEXT_LEN - 1)
		textLen = MAX_MACRO_TEXT_LEN - 1;


	// add to the sorted table
	int c;
	int i = 0;
	while (i<m_count) {
		c = strcmp(m_table[i].key, key);
		if (c == 0) 
			// string already exists
			return i;
		if (c > 0)
			break;
		i++;
	}

	if (m_occupied + keyLen + textLen + 2 > m_memSize)
		return -1;

	// moving
	for (int k=m_count; k>i; k--)
		m_table[k] = m_table[k-1];

	// store new macro
	char *p = m_macroMem + m_occupied;
	m_table[i].key = p;
	memcpy(p, key, keyLen);
	p[keyLen] = 0;
	p += keyLen+1;
	m_table[i].text = p;
	memcpy(p, text, textLen);
	p[textLen] = 0;

	m_occupied += keyLen+textLen+2;
	m_count++;

	return i;
}

//---------------------------------------------------------------
// add a new macro into the sorted macro table
// item format: key:text (key and text are separated by a colon)
//---------------------------------------------------------------
int CMacroTable::addItem(const char *item)
{
	char key[MAX_MACRO_KEY_LEN];

	// Parse the input item
	char * pos = strchr(item, ':');
	if (pos == NULL)
		return -1;
	int keyLen = (int)(pos - item);
	if (keyLen > MAX_MACRO_KEY_LEN-1)
		keyLen = MAX_MACRO_KEY_LEN-1;
	strncpy(key, item, keyLen);
	key[keyLen] = '\0';
	return addItem(key, ++pos);
}

//---------------------------------------------------------------
void CMacroTable::resetContent()
{
	m_occupied = 0;
	m_count = 0;
}
