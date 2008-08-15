README - TDMInstall

TDMInstall is an aggregate work based on the NSIS installer-creation system,
comprised of a script and several plugin DLLs. The end goal of TDMInstall is to
provide a setup wizard capable of automatically downloading, installing,
upgrading, and removing versioned components of a software package.

Some of TDMInstall's features:
 * Component descriptors and installation manifests use XML
 * Discriminatory uninstallation: only remove files installed by the setup
     program
 * Support online-only or online/offline installers with inner components
 * Download latest descriptor file to determine available components
 * Update to newer component versions by automatically removing the old version,
     then installing the new version
 * Let users select from multiple download mirrors
 * Unpack zip archives, gzip and bzip2 tarballs, and 7-zip archives, maintaining
     original file attributes
 * Download component archives with resume capability (uses the Inetc plugin for
     NSIS)
 * Track multiple installations
 * No registry entries created


=====
Prerequisites
=====

TDMInstall was built and tested using NSIS 2.38; you must have a similarly-
numbered version of NSIS installed. (nsDialogs and MUI2 are required.)

tdminstall.dll was built and tested under GCC versions 3.4.5 and 4.3.1 for
MinGW, and the included Makefile is designed for MinGW/GCC. It is recommended
that you use the 3.4 series or later.

UPX (<http://upx.sourceforge.net/>) can be used to achieve a smaller size for
the final executable. For upx.exe to be found, add its directory to PATH or
place it in the base TDMInstall directory.


=====
Building
=====


=====
TDMInstall's Components
=====

 * The script (setup.nsi, main.nsh)
 * The TDMInstall plugin (plugins/tdminstall.dll)
    - Comprised of every source file in the base directory, as well as the
      source files in the "7z", "boost", "bzip2", and "zlib" subdirectories.
 * The Inetc plugin (plugins/inetc.dll)
 * The RealProgress plugin (plugins/RealProgress.dll)
 * The ShellLink plugin (plugins/ShellLink.dll)


=====
Licensing
=====

Every source file in the base directory EXCEPT "untgz.c" is licensed freely in
the public domain, subject to the following text:
   DISCLAIMER:
   The author(s) of this file's contents release it into the public domain,
   without express or implied warranty of any kind. You may use, modify, and
   redistribute this file freely.

The source files in the "7z" subdirectory are licensed under the GNU LGPL. See
7z/MODIFIED_LZMA.txt and 7z/LGPL.txt for details.

The source files in the "boost" subdirectory are licensed according to Boost's
license. See boost/LICENSE_1_0.txt for details.

The source files in the "bzip2" subdirectory are licensed according to bzip2's
license. See bzip2/MODIFIED_BZIP2.txt, bzip2/README, and bzip2/LICENSE for
details.

The source files in the "tinyxml" subdirectory are licensed according to
tinyxml's license. See tinyxml/MODIFIED_TINYXML.txt for details.

The source files in the "zlib" subdirectory, as well as the file "untgz.c" in
the base directory, are licensed according to zlib's license. See
zlib/MODIFIED_ZLIB.txt for details.

The plugin DLLs in the "plugins" subdirectory (excluding tdminstall.dll if
present) are licensed according to the "zlib/libpng" license, unless otherwise
marked. See plugins/README-plugins.txt for details.
