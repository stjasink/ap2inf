# ap2inf

Converter for Apple 2 Disk Images of Infocom games, written between 1994 and 1997.  It somehow escaped any modern
version control until now, so I am adding it for posterity rather than for any practical use.  It probably doesn't
work on many modern systems.  The "Systems that ap2inf works on" section at the end really shows its age.  The 
80-column formatted lines in this file are from the original `README` in the ZIP file distribution.

This was written to extract the data from Apple 2 emulator disk images of Infocom adventure games and store it 
in the standard Infocom data file format so that it can be played with interpreters on other systems.  The emulator
formats that it supports are Apl2em and ApplePC.  These may or may not still exist.

## Contents

* `README.md`    (this file)
* `CHANGELOG.md` (log of changes in versions)
* `ap2inf.c`     (the ANSI C source)
* `ap2inf.exe`   (very old MSDOS executable)

## Disk images

In order to extract the data file from a disk image, you first need a disk 
image. Disk images are binary dumps of all the data on a disk into a file. 
These are typically used with emulators, so that disks from the emulatED 
machine can be read as files on the emulatING machine.

## How to use ap2inf

A number of features have appeared in version 1.4 that were not in version 
1.3. All of these are available by use of command-line options, so I'll just 
give an explanation of the options. The interactive bits that were in version 
1.3 have disappeared. To use ap2inf your command line should look something 
like this:

```ap2inf [options] <disk file> <data file>```

`<disk file>` is the name of an Apl2em-compatible disk image file in the 
current directory and `<data file>` is the name of the data file that the 
program will create. If a file of this name exists, it will be overwritten, 
so be careful. `<disk file>` and `<data file>` are required, whereas 
`[options]` are... optional. 

The options can be one or more of the following:

`-h`    This gives help in using ap2inf. Since you're reading this file you 
probably won't need to use this option. The program will not extract 
anything if this option is found.

`-ix`   This is to specify the sector interleaving scheme to use. Using 
`-i0` will cause the program to assume that the disk has no interleaving, 
which is true for most games. Using `-i1` assumes standard DOS 3.3 
interleaving such as interpreter E uses.

`-lxxx` This option allows the user to specify the length of the data file 
to extract. With most games this information is stored in the game header, 
but with the older games it is not. If there is no data length in the game 
header and none is specified on the command line then the maximum possible 
length of 131072 bytes is used so as not to truncate the data.

`-cxxx` This option allows the user to specify the checksum to use to check 
the integrity of the data file. As with the data length, this is usually 
stored in the game header but is absent in the older games.

`-pxxx` This is to specify padding. Older interpreters used to a use virtual 
memory style paging scheme due to memory constraints. Thus data files were 
padded with zeros to fill up the last page to the page size the interpreter 
used. Common values are 256 or 512 bytes. I'm sure that none of the modern 
commonly-used interpreters need the files to be padded in this way but it 
doesn't hurt.

Notice that there is no space between the letter indicating the option and 
the number that follows it. Numbers can be either decimal or hexadecimal. 
Hexadecimal numbers must begin with "0x", ie 33 decimal = 0x21 hex.

## Data lengths and checksums

Comments about data lengths and checksums... Specifying a data length
longer than that of the actual data should not affect the correctness
of the data file, since the interpreter won't read past the end of where
it needs to and the checksum should still be correct because the extra
bytes are all zero. If you have a disk image of an old game (old versions
of Zork I, Zork II and Deadline) then you can either go with making up
your own length or you can use the lengths (and checksums) that are in the
Infocom Fact Sheet that Paul David Doherty maintains. (You can fine it
on ftp.gmd.de in /if-archive/infocom/docs/fact-sheet.txt.) Some of the disk
images of old games that I've come across have a bit of the third last
track repeated after the end of the data, making up the rest of the
partial last track. Because of this, extracting a data file longer than
the end of the actual data will cause the checksum to fail, even though
all the valid data is actually intact.

## Games that ap2inf works on

Ap2inf 1.4.2 has been tested on games running under interpreters B, E, H,
K and M. Earlier versions of ap2inf used to stop if they did not recognise
the interpreter, but now it just uses a default de-interleaving scheme in
such cases (which you can change with -i if it doesn't work). If anyone
has any images of games with other interpreters, I'd appreciate it if
you could send them to me so that I could get ap2inf to recognise them.
Some games have messages from hackers stuck onto the front of them, which
confuses ap2inf since it won't find the signature bytes for which it
searches. These will then have to be treated as a different case. Of course,
it's just detecting the interpreter that won't work and you can run ap2inf
on it anyway, specifying the interleave pattern if it doesn't work the
first time.

## Systems that ap2inf works on

The program is written in ANSI C and the source is provided, so it
should (at least theoretically) be portable to any system that has an
ANSI-compliant C compiler. I've compiled and run it without any problems
on the following systems:

* Systems running Unix SysVR4
* Sun machines running SunOS 5.4
* SGI machines running IRIX 5.3
* DEC Alpha AXP running OpenVMS V6.2
* PCs running Linux
* PCs running MSDOS or Windows 95 with Turbo C++ v3.0 and Borland C++ v4.5

The compiler that I used under SunOS 4.1 was nasty (and old) and wanted
me to put my definitions for the parameters under the function header in
the old way, but I didn't feel like doing that, so I didn't.

