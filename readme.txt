The UniKey project consists of following components:
- UniKey: Vietnamese keyboard program for Windows
- UVConverter: Universal Vietnamese Encoding Converter
- vnconv (used by UniKey, UVConverter): library for Vietnamese encoding conversion.

Official web site: http://unikey.org

These programs were released under the GNU General Public License!
Please be fair and repect the rules. I have spent hundreds of hours to write these programs.

You have to use VC.NET to build UniKey.

- Open the workspace newkey\newkey.sln
- Build NewKey project with configuration "Win32 Release" or "Win 32 Unicode Release".
Other configurations were not well tested.

From version 3.62, UniKey uses RtfIO library for converting rich texts. At the moment I cannot
release source code of this library. Therefore, this package includes only the binary library
in rtfio\rtfio.dll. You will need rtfio\rtfio.lib to build UniKey. The header file of 
this library is rtfio\intrtfio.h. To run UniKey, rtfio.dll must be in the same folder
as UniKey program. 

The official UniKey program on UniKey web site was compiled with RtfIO library inside, therefore
does not need rtfio.dll to run.

Build UVConverter in VC.NET
- Open the workspace uvconvert\uvconvert.vcproj
- Build

Build UVConverter in Linux:
- Go to directory uvconvert
- Issue command: make

For support, questions, please visit UniKey forum at: http://unikey.org/forum

Pham Kim Long 
unikey@gmail.com
