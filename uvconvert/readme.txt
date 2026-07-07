UVConverter 1.1.2
UVConverter is a part of UniKey project. Visit UniKey website at: http://unikey.sourceforge.net 
This programme is released under the terms of the GNU General Public License (GPL). See the enclosed file gpl.txt

The programme uses the vnconv library (which is also a part of UniKey project). If you get this
source directory (uvconvert) from CVS, you must get the vnconv directory from CVS as well.

Compile instructions:
   make depend
   make

After the compilation, you will get the generated executable file: uvconvert/uvconv

=======
To add more charsets: See instructions in vnconv/data.cpp


=======
Usage:

uvconv [file] [OPTIONS]\n"
    file: Input file [Default: stdin - keyboard input]"
Options
    -h --help     Print Help and exit
    -f --from     Input charset [Default: TCVN3]
    -t --to       Output charset [Default: UTF-8]
    -o --output   Output file [Default: stdout - console output]
    -l --list     Print charset list and exit
    -m --mviqr    Mixed VIQR characters present in input
    -s --sviqr    Strict VIQR input, converting everything, not checking
                  escapes for URLs, etc. (a bit faster, but not recommended).
Example: 
    uvconv in.txt -f VNI -t UTF-8 -o out.txt\n";

Notes: 
If you convert a document, say in UTF-8, to VIQR while there are also VIQR sequences present
in the document, the result may be undesirable. For example: "ho.c tie^'ng Vie^.t"
will be converted to "ho\.c tie\^'ng vie\^.t", because uvconv will insert escape
characters for the tone marks. To disable this behaviour, set the option -m.

=======
Contributions, bug reports, suggestions are all welcome.

Many thanks to VietLUGers (http://vietlug.sourceforge.net), and Nguyen Dai Quy in particular.

=======

Have fun!!!

Pham Kim Long <longp@cslab.felk.cvut.cz>
