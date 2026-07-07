/*------------------------------------------------------------------------------
UVConverter: Universal Vietnamese Encoding Converter
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

#include "prehdr.h"
#include "vnconv.h"

using namespace std;
struct Options {
	char * inCharset;
	char * outCharset;
	char * inFile;
	char * outFile;
	int mixedViqr;
	int escViqr;
	Options() {
		inCharset = NULL;
		outCharset = NULL;
		inFile = NULL;
		outFile = NULL;
		mixedViqr = 0;
	}
};

Options Opt;

void usage();
void parse_switches(int argc, char * argv[]);
int getCharsetId(const char *name);
void printCharsetList();

//-------------------------------------------
int main(int argc, char* argv[])
{
	parse_switches(argc, argv);

	// set default values
	int inCharset = CONV_CHARSET_TCVN3;
	int outCharset = CONV_CHARSET_UNIUTF8;
	char *defInCharsetName = "TCVN3";
	char *defOutCharsetName = "UTF-8";

	if (Opt.inCharset) {
		inCharset = getCharsetId(Opt.inCharset);
		if (inCharset == -1) {
			cerr << "===> Invalid input charset\n\n";
			usage();
			return 1;
		}
	}
	else Opt.inCharset = defInCharsetName;

	if (Opt.outCharset) {
		outCharset = getCharsetId(Opt.outCharset);
		if (outCharset == -1) {
			cerr << "===> Invalid output charset\n\n";
			usage();
			return 1;
		}
	}
	else Opt.outCharset = defOutCharsetName;

	if (inCharset == outCharset) {
		cerr << "===> Input charset and output charset are the same. No conversion is needed!\n";
		return 1;
	}

	char *inFile = NULL;
	char *outFile = NULL;
	VnConvOptions convOptions;
	VnConvResetOptions(&convOptions);

	if (Opt.inFile)
		inFile = Opt.inFile;
	if (Opt.outFile)
		outFile = Opt.outFile;

	if (Opt.mixedViqr)
		convOptions.viqrMixed = 1;

	VnConvSetOptions(&convOptions);
	
	int ret = VnFileConvert(inCharset, outCharset, inFile, outFile);

	if (ret == 0)
		cerr << "===> Successfully converted from " << Opt.inCharset << " to " << Opt.outCharset << endl;
	else
	  cerr << "===> Error " << ret << ": " << VnConvErrMsg(ret) << endl;

	return 0;
}


//-------------------------------------------
int keymatch (char * arg, const char * keyword, int minchars)
{
  register int ca, ck;
  register int nmatched = 0;

  while ((ca = *arg++) != '\0') {
    if ((ck = *keyword++) == '\0')
      return 0;		/* arg longer than keyword, no good */
    //if (isupper(ca))		/* case sensitive */
    //  ca = tolower(ca);
    if (ca != ck)
      return 0;		/* no good */
    nmatched++;			/* count matched characters */
  }
  /* reached end of argument; fail if it's too short for unique abbrev */
  if (nmatched < minchars)
    return 0;
  return 1;
}


//-------------------------------------------
void parse_switches(int argc, char * argv[])
{
	int argn;
	char * arg;
	
	Opt.inFile = NULL;
	for (argn = 1; argn < argc; argn++) {
		arg = argv[argn];
		//arg++;
		if (keymatch(arg, "--help", 6) || keymatch(arg, "-help", 2)) {
			usage();

		} else if (keymatch(arg, "--list", 6) || keymatch(arg, "-l", 2)) {
			printCharsetList();
			exit(0);

		} else if(keymatch(arg, "-f", 2) || keymatch(arg, "--from", 6)) {
			if (++argn >= argc)	/* advance to next argument */
				usage();
			Opt.inCharset = argv[argn];

		} else if(keymatch(arg, "-t", 2) || keymatch(arg, "--to", 4)) {
			if (++argn >= argc)	/* advance to next argument */
				usage();
			Opt.outCharset = argv[argn];

		} else if(keymatch(arg, "-m", 2) || keymatch(arg, "--mviqr", 7)) {
			Opt.mixedViqr = 1;
		} else if(keymatch(arg, "-s", 2) || keymatch(arg, "--sviqr", 7)) {
			Opt.escViqr = 0;
		} else if(keymatch(arg, "-o", 2) || keymatch(arg, "--output", 8)) {
			if (++argn >= argc)	/* advance to next argument */
				usage();
			Opt.outFile = argv[argn];

		} else if (*arg == '-' || Opt.inFile != NULL) { // keymatch(arg, "-", 1) || keymatch(arg, "--", 2)) {
		  cerr << "==>Wrong options: " << arg << endl;
		  usage();
		} else {
		  // input file
		  Opt.inFile = argv[argn];
		}
	}
}


//-------------------------------------------
void usage()
{
	char usageText[] = 
		"UVConverter 1.1.2 - Vietnamese encoding converter\n"
		"UniKey project: http://unikey.sourceforge.net\n"
		"--------------------------------------------------------------------------\n"
		"Usage: uvconv [file] [OPTIONS]\n"
		"    file \t\tInput file [Default: stdin - keyboard input]\n"
		"Options:\n"
		"    -h \t--help   \tPrint Help and exit\n"
		"    -f \t--from   \tInput charset [Default: TCVN3]\n"
		"    -t \t--to     \tOutput charset [Default: UTF-8]\n"
		"    -o \t--output \tOutput file [Default: stdout - console output]\n"
		"    -l \t--list   \tPrint charset list and exit\n\n"
		"    -m \t--mviqr  \tMixed VIQR characters present in input\n"
		"    -s \t--sviqr  \tStrict VIQR input, converting everything, not checking\n" 
		"       \t         \tescapes for URLs (a bit faster, but not recommended)\n"
		"Example:\n"
		"    uvconv in.txt -f VNI -t UTF-8 -o out.txt\n";

	cerr << usageText;
	exit(1);
}

//-------------------------------------------
void printCharsetList()
{
	cerr << "List of supported charsets:" << endl;
	for (int i=0; i<CharsetCount; i++) {
		if ((i+1) % 5 == 0)
			cerr << " " << CharsetIdMap[i].name << endl;
		else
			cerr << " " << CharsetIdMap[i].name ;//<< "";
	}
	cerr << endl;
}

//-------------------------------------------
int getCharsetId(const char *name)
{
	char uname[256];
	strcpy(uname, name);

	int i;
	for (i=0; uname[i]; i++)
		uname[i] = toupper(uname[i]);

	int id;
	for (i=0; i < CharsetCount; i++) {
		if (strcmp(uname, CharsetIdMap[i].name) == 0)
			break;
	}
	id = (i < CharsetCount)? CharsetIdMap[i].id : -1;
	return id;
}
