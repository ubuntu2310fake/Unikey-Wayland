#include "prehdr.h"
#include "byteio.h"

//------------------------------------------------
StringBIStream::StringBIStream(BYTE *data, int len)
{
	m_data = m_current = data;
	m_len = m_left = len;
	if (len == -1)
		m_eos = (*data == 0);
	else
		m_eos = (len <= 0);
	m_didBookmark = 0;
}

//------------------------------------------------
int StringBIStream::eos()
{
	return m_eos;
}

//------------------------------------------------
int StringBIStream::getNext(BYTE & b)
{
	if (m_eos)
		return 0;
	b = *m_current++;
	if (m_len == -1) {
		m_eos = (b == 0);
	}
	else {
		m_left--;
		m_eos = (m_left <= 0);
	}
	return 1;
}

//------------------------------------------------
int StringBIStream::unget(BYTE b)
{
	if (m_current != m_data) {
		*--m_current = b;
		m_eos = 0;
		if (m_len != -1)
			m_left++;
/*
		if (m_left > m_len) {
			int err;
			err = 1;
		}
*/
	}
	return 1;
}

//------------------------------------------------
int StringBIStream::getNextW(WORD & w)
{
	if (m_eos) return 0;
	w = *((WORD *)m_current);
	m_current += 2;
	if (m_len == -1)
		m_eos = (w == 0);
	else {
		m_left -= 2;
		m_eos = (m_left <= 0);
	}
	return 1;
}


//------------------------------------------------
int StringBIStream::peekNext(BYTE & b)
{
	if (m_eos)
		return 0;
	b = *m_current;
	return 1;
}

//------------------------------------------------
int StringBIStream::peekNextW(WORD & w)
{
	if (m_eos)
		return 0;
	w = *((WORD *)m_current);
	return 1;
}

//------------------------------------------------
void StringBIStream::reopen()
{
	m_current = m_data;
	m_left = m_len;
	if (m_len == -1)
		m_eos = (m_data == 0);
	else
		m_eos = (m_len <= 0);
	m_didBookmark = 0;
}

//------------------------------------------------
int StringBIStream::bookmark()
{
	m_didBookmark = 1;
	m_bookmark.current = m_current;
	m_bookmark.data = m_data;
	m_bookmark.eos = m_eos;
	m_bookmark.left = m_left;
	m_bookmark.len = m_len;
	return 1;
}

//------------------------------------------------
int StringBIStream::gotoBookmark()
{
	if (!m_didBookmark)
		return 0;
	m_current = m_bookmark.current;
	m_data = m_bookmark.data;
	m_eos = m_bookmark.eos;
	m_left = m_bookmark.left;
	m_len = m_bookmark.len;
	return 1;
}

//------------------------------------------------
int StringBIStream::close()
{
	return 1;
};

//////////////////////////////////////////////////
// Class StringBOStream
//////////////////////////////////////////////////

//------------------------------------------------
StringBOStream::StringBOStream(BYTE *buf, int len)
{
	m_current = m_buf = buf;
	m_len = len;
	m_out = 0;
	m_bad = 0;
}

//------------------------------------------------
int StringBOStream::putB(BYTE b)
{
	m_out++;
/*
	if (m_out >= 2147483647) {
		int err;
		err = 1;
	}
*/
	if (m_bad)
		return 0;
/*
	if (m_out < 0) {
		int i;
		i = 1;
	}
*/
	if (m_out <= m_len) {
		*m_current++ = b;
		return 1;
	}
	m_bad = 1;
	return 0;
}

//------------------------------------------------
int StringBOStream::putW(WORD w)
{
	m_out += 2;
	if (m_bad)
		return 0;
	if (m_out <= m_len) {
		*((WORD *)m_current) = w;
		m_current += 2;
		return 1;
	}
	m_bad = 1;
	return 0;
}

//------------------------------------------------
int StringBOStream::puts(const char *s, int size)
{
	if (size == -1) {
		while (*s) {
			m_out++;
			if (m_out <= m_len) 
				*m_current++ = *s;
			s++;
		}
		if (!m_bad && m_out > m_len)
			m_bad = 1;
		return (!m_bad);
	}

	int n;
	if (!m_bad && m_out <= m_len) {
		n = m_len - m_out;
		if (n>size)
			n = size;
		memcpy(m_current, s, n);
		m_current += n;
	}

	m_out += size;
	if (!m_bad && m_out > m_len)
		m_bad = 1;
	return (!m_bad);
}

//------------------------------------------------
void StringBOStream::reopen()
{
	m_current = m_buf;
	m_out = 0;
	m_bad = 0;
}


//------------------------------------------------
int StringBOStream::isOK()
{
	return !m_bad;
}


////////////////////////////////////////////////////
// Class FileBIStream                             //
////////////////////////////////////////////////////

//----------------------------------------------------
FileBIStream::FileBIStream(int bufSize, char *buf) 
{
	m_file = NULL;
	m_buf = buf;
	m_bufSize = bufSize;
	m_own = 1;
	m_didBookmark = 0;
}

//----------------------------------------------------
FileBIStream::~FileBIStream()
{
	if (m_own)
		close();
}

//----------------------------------------------------
int FileBIStream::open(const char *fileName)
{
	m_file = fopen(fileName, "rb");
	if (m_file == NULL)
		return 0;
	setvbuf(m_file, m_buf, _IOFBF, m_bufSize);
	m_own = 0;
	return 1;
}

//----------------------------------------------------
int FileBIStream::close()
{
	if (m_file != NULL) {
		fclose(m_file);
		m_file = NULL;
	}
	return 1;
}

//----------------------------------------------------
void FileBIStream::attach(FILE * f)
{
	m_file = f;
	m_own = 0;
}

//----------------------------------------------------
int FileBIStream::eos()
{
	return feof(m_file);
}

//----------------------------------------------------
int FileBIStream::getNext(BYTE &b)
{
	b = fgetc(m_file);
	return (!feof(m_file));
}

//----------------------------------------------------
int FileBIStream::peekNext(BYTE &b)
{
	b = fgetc(m_file);
	if (feof(m_file))
		return 0;
	ungetc(b, m_file);
	return 1;
}

//----------------------------------------------------
int FileBIStream::unget(BYTE b)
{
	ungetc(b, m_file);
	return 1;
}

//----------------------------------------------------
int FileBIStream::getNextW(WORD &w)
{
	w = fgetwc(m_file);
	return (!feof(m_file));
}

//----------------------------------------------------
int FileBIStream::peekNextW(WORD &w)
{
	w = fgetwc(m_file);
	if (feof(m_file))
		return 0;
	ungetwc(w, m_file);
	return 1;
}

//----------------------------------------------------
int FileBIStream::bookmark()
{
	m_didBookmark = 1;
	m_bookmark.pos = ftell(m_file);
	return 1;
}


//----------------------------------------------------
int FileBIStream::gotoBookmark()
{
	if (!m_didBookmark)
		return 0;
	fseek(m_file, m_bookmark.pos, SEEK_SET);
	return 1;
}

////////////////////////////////////////////////////
// Class FileBOStream                             //
////////////////////////////////////////////////////
//----------------------------------------------------
FileBOStream::FileBOStream(int bufSize, char *buf) 
{
	m_file = NULL;
	m_buf = buf;
	m_bufSize = bufSize;
	m_own = 1;
	m_bad = 1;
}

//----------------------------------------------------
FileBOStream::~FileBOStream()
{
	if (m_own)
		close();
}

//----------------------------------------------------
int FileBOStream::open(const char *fileName)
{
	m_file = fopen(fileName, "wb");
	if (m_file == NULL)
		return 0;
	m_bad = 0;
	setvbuf(m_file, m_buf, _IOFBF, m_bufSize);
	m_own = 1;
	return 1;
}

//----------------------------------------------------
void FileBOStream::attach(FILE * f)
{
	m_file = f;
	m_own = 0;
	m_bad = 0;
}

//----------------------------------------------------
int FileBOStream::close()
{
	if (m_file != NULL) {
		fclose(m_file);
		m_file = NULL;
	}
	return 1;
}

//----------------------------------------------------
int FileBOStream::putB(BYTE b)
{
	if (m_bad)
		return 0;
	m_bad = (fputc(b, m_file) == EOF);
	return (!m_bad);
}

//----------------------------------------------------
int FileBOStream::putW(WORD w)
{
	if (m_bad)
		return 0;
	m_bad = (fputwc(w, m_file) == WEOF); 
	return (!m_bad);
}

//----------------------------------------------------
int FileBOStream::puts(const char *s, int size)
{
	if (m_bad)
		return 0;
	if (size == -1) {
		m_bad = (fputs(s, m_file) == EOF);
		return (!m_bad);
	}
	int out = fwrite(s, 1, size, m_file);
	m_bad = (out != size);
	return (!m_bad);
}

//----------------------------------------------------
int FileBOStream::isOK()
{
	return !m_bad;
}
