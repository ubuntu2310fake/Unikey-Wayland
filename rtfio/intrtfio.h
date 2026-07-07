#ifndef __RTFIO_INTERFACE_H

#define __RTFIO_INTERFACE_H

#ifdef	__cplusplus
extern "C" {
#endif

int RtfFileConvert(int inCharset, int outCharset, const char *inFile, const char *outFile, int stdFontset);
int RtfFileStreamConvert(int inCharset, int outCharset, FILE *inf, FILE *outf, int stdFontset);
int RtfMemConvert(int inCharset, int outCharset, BYTE *input, BYTE *output, int & inLen, int & maxOutLen, int stdFontset);
int RtfMemExtractText(BYTE *input, BYTE *output, int & inLen, int & maxOutLen, int unicode);
void InitRtfIO();

#ifdef	__cplusplus
}
#endif

#endif
