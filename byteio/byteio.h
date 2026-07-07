#ifndef BYTE_IO_STREAM_H
#define BYTE_IO_STREAM_H


//#include "vnconv.h"
typedef unsigned char BYTE;
typedef unsigned short WORD;

//----------------------------------------------------
class ByteStream {

};

//----------------------------------------------------
class ByteInStream: public ByteStream
{
public:
	virtual int getNext(BYTE &b) = 0;
	virtual int peekNext(BYTE &b) = 0;
	virtual int unget(BYTE b) = 0;

	virtual int getNextW(WORD &w) = 0;
	virtual int peekNextW(WORD &w) = 0;

	virtual int bookmark() //no support for bookmark by default
	{
		return 0;
	}

	virtual int gotoBookmark()
	{
		return 0;
	}

	virtual int eos() = 0; //end of stream
	virtual int close() = 0;
};

//----------------------------------------------------
class ByteOutStream: public ByteStream
{
public:
	virtual int putB(BYTE b) = 0;
	virtual int putW(WORD w) = 0;
	virtual int puts(const char *s, int size = -1) = 0; // write an 8-bit string
	virtual int isOK() = 0;// get current stream state
};

//----------------------------------------------------
class StringBIStream : public ByteInStream
{
protected:
	int m_eos;
	BYTE *m_data, *m_current;
	int m_len, m_left;

	struct {
		int eos;
		BYTE *data, *current;
		int len, left;
	} m_bookmark;

	int m_didBookmark;
		
public:
	StringBIStream(BYTE *data, int len);
	virtual int getNext(BYTE &b);
	virtual int peekNext(BYTE &b);
	virtual int unget(BYTE b);

	virtual int getNextW(WORD &w);
	virtual int peekNextW(WORD &w);
	virtual int eos(); //end of stream
	virtual int close();

	virtual int bookmark();
	virtual int gotoBookmark();

	void reopen();
	int left() {
		return m_left;
	}
};

//----------------------------------------------------
class FileBIStream : public ByteInStream
{
protected:
	FILE *m_file;
	int m_bufSize;
	char *m_buf;
	int m_own;
	int m_didBookmark;

	struct {
		long pos;
	} m_bookmark;

public:

	FileBIStream(int bufsize = 8192, char *buf = NULL);
//	FileBIStream(char *fileName, int bufsize = 8192, void *buf = NULL);

	int open(const char *fileName);
	void attach(FILE *f);
	virtual int close();

	virtual int getNext(BYTE &b);
	virtual int peekNext(BYTE &b);
	virtual int unget(BYTE b);

	virtual int getNextW(WORD &w);
	virtual int peekNextW(WORD &w);
	virtual int eos(); //end of stream

	virtual int bookmark();
	virtual int gotoBookmark();

	~FileBIStream();
};


//----------------------------------------------------
class StringBOStream : public ByteOutStream
{
protected:
	BYTE *m_buf, *m_current;
	int m_out;
	int m_len;
	int m_bad;
public:
	StringBOStream(BYTE *buf, int len);
	virtual int putB(BYTE b);
	virtual int putW(WORD w);
	virtual int puts(const char *s, int size = -1);
	virtual int isOK(); // get current stream state

	virtual int close()	
	{
		return 1;
	};

	void reopen();
	int getOutBytes() {
		return m_out;
	}
};

//----------------------------------------------------
class FileBOStream : public ByteOutStream
{
protected:
	FILE *m_file;
	int m_bufSize;
	char *m_buf;
	int m_own;
	int m_bad;

public:
	FileBOStream(int bufsize = 8192, char *buf = NULL);
//	FileBOStream(char *fileName, int bufsize = 8192, void *buf = NULL);

	int open(const char *fileName);
	void attach(FILE *);
	virtual int close();

	virtual int putB(BYTE b);
	virtual int putW(WORD w);
	virtual int puts(const char *s, int size = -1);
	virtual int isOK(); // get current stream state
	~FileBOStream();
};


#endif