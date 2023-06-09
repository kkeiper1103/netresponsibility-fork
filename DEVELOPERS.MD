-----GOALS OF NET RESPONSIBILITY-----
Net Responsibility is intended to be a good open source accountability software
for all major platforms. It's purpose is to prevent people from certain surfing
habits by holding them accountable to others. 
This is done by logging all internet traffic and generating clear reports that
are emailed regularly to the accountability partner(s).

We want Net Responsibility to be:

* Accurate and effective
* Secure and hard to circumvent
* Easy to use, both for users and accoutability partners
* Almost invisible
* Cross-platform
* Customizable and unlimited

OPEN SOURCE
Net Responsibility is, and will always be, open source software. It is released
under GNU GPL 2, or any later version, and should only rely on open source software.

ACCURATE AND EFFECTIVE
Net Responsibility will always strive towards being more accurate in it's
reports. A lot have happened from 0.5.0 to 2.0.2, but we can always be better. 
The users can help us out a lot here by sending anonymous improvement data.
Yet we don't want Net Responsibility to slow down the surfing experience, at
least not too much.

SECURE AND HARD TO CIRCUMVENT
In order to be a good accountability software, Net Responsibility has to be hard
to circumvent. We may not be able to prevent the user from shutting down the
program, but as long as possible we have to log and report when that's done. 
There will always be a tension between being open and being secure here. With
too much customization there is a risk to be unsecure. Therefore all changes
that affect the security have to be reported to the accountability partner.

EASY TO USE
It should be as easy as possible for anyone who wants to be held accountable to
setup and install Net Responsibility. 
We also want to make it easy to be an accountability partner, by making as easy
and accurate reports as possible.

ALMOST INVISIBLE
Net Responsibility should run in the background and do it's job automatically.
It should also use as little memory and processor power as possible. 
The user won't have to worry about if Net Responsibility is running or not, and
shouldn't be able to control it too much. 

CROSSPLATFORM
We want to provide Net Responsibility for Linux, Windows and Macintosh users.
If there's a need for support on even more platforms, we should encourage anyone
who wants to port it.

CUSTOMIZABLE AND UNLIMITED
The user should be able to customize the software as she/he wants to, as long
as it doesn't allow her/him to circumvent the software. 
There will never be limitations such as a max number of accountability partners,
or certain predetermined intervals between the reports.




-----REASONS TO REWRITE NET RESPONSIBILITY------
We've decided to rewrite the entire software in C++. There are several reasons
for this. To list them very shortly:
* Efficiency
* Security
* Cross-platform
* Invite more developers




-----HOW TO KEEP CONTACT-----
Until now, the main way to keep contact related to development, support etc.
have been one single thread at ubuntuforums.org. From now on we will keep
contact through email and the forum at netresponsibility.com/forum. 
Messages that are clearly directed to one specific person, such as questions,
corrections is better to send with email. This is especially important when
you want a fast answer.
Messages that are of interest for a wider crowd, such as updates, bug reports,
feature requests etc. may be posted at the forum, either as new threads or as
replies to existing threads.
We are also moving from Launchpad to Sourceforge. Eventually the only two
places to look for things related to Net Responsibility should be
www.netresponsibility.com and www.sourceforge.net/projects/responsibility/
Before you make any bigger changes in the code or structure, send an email to
those who are currently working on that specific part, or to the main developer.
Then we can discuss it together.

EMAIL ADDRESSES:
robertrosman@gmx.com - Robert, main developer and organizer.
mpnordland@gmail.com - Micah, Proxy, GUI, multi-users, and sundry other things
corwatts@gmail.com   - Corey, website developer.
[Fill in more]




-----THE SOURCE CODE-----
The source code will be hosted at Sourceforge. current SVN repository for
the rewrite to C++ is:
https://responsibility.svn.sourceforge.net/svnroot/responsibility/trunk/

COMPILATION:
First you need to install libPoco (www.pocoproject.org). Either find a 
precompiled package and install (it should be version 1.4.0 or later), or 
download the complete source and compile it yourself.
Net Responsibility is designed to follow the regular GNU make-chain.
To install the software, open a terminal and run:

cd [Net Responsibility Top Dir (this dir)]
autoreconf -i   #Only required for the repo-version
./configure
make
make install  #As root

On Linux and Mac you'll need GCC and libpcap, while Windows requires MinGW,
MSYS and WinPcap to be installed. Note that you'll have to run the commands
inside the MSYS-terminal on Windows.
To uninstall Net Responsibility you can run:

make uninstall  #As root
make clean

More installation instructions can be found in the file INSTALL.
The Code::Blocks project files are set up to use the generated Makefile,
which means you have to run the autoreconf -i; ./configure; make; make install
procedure before being able to do development.

DIRECTORY LAYOUT:
/                   Root directory.
   include/         All header files (.h)
   data/            Data files.
   lib/             External libraries
   projectFiles/
      linux/        Project files for Linux.
      windows/      Project files for Windows
   src/             All source files (.cpp)
   scripts/         Scripts for installation etc.

Files needed by Autotools is found in the root directory, as well as this file.
The object files produces by GCC will be placed in the src dir, and the
binaries in the root dir.

PROGRAMMING GUIDELINES:
For a good extended document with c++ programming guidelines, look here:
http://geosoft.no/development/cppstyle.html. This document would be way too
long if we included all important guidelines, so I've only included some
short rules to remember.

* Variables that are private or protected inside a class have an underscore
  prefix, like this: _someVariable.
* All classnames start with a capital letter.
* I prefer tab indent, since every programmer can choose his/her own indent
  that way and be comfortable with it.
* The declarations will be done in the header files, but not the
  implementations, they will be done in the source files.
* Platform-specific code should be included or defined by the preprocessor at
  the top of the header file. We should as long as possible use the same source
  code for all platforms, and when needed, we can include different files for
  the different platforms. In that case the filename should tell which platform
  the code is written for, but the function or class name should be the same
  for all platforms. Let's say we need a way to get the computers hardware
  address, but it's done differently on different platforms. The filenames
  would be "GetHardwareAddressWindows.h" and "GetHardwareAddressLinux.h" etc.
  The function would be called getHardwareAddress(). The preprocessor will
  decide which file to include, but the code will simply call the function
  getHardwareAddress().

MODULAR ARCHITECTURE:
The Idea is that eventually, except for a small main daemon, all of the
components of Net Responsibility would be replaceable. I'm right now imagining
in ways for this to work, probably with dynamic libraries loaded at run time. 
That way a module would only have to implement certain methods or functions, and 
the daemon would transparently handle the interactions between the modules. So, 
what you would do is create a library and then give it a specific name, and put 
it in the right place, and Net Responsibility would do the rest.




-----CURRENT STATE OF REWRITE-----
The rewrite is shaping up, but there is a bit left.

MAIN AREAS TO FINISH:
* Write a transparent proxy that's sniffing all traffic (Currently it's working 
  using libpcap/winpcap, but with our own proxy we could do the filtering faster 
  and add options for blocking content etc.) Micah is working on this.
* GUI. The goal is to make configurations and display messages through the 
  browser. One alternative is to use the current GUI as long and turn it into 
  an executable. It's written in Python.

WISH LIST:
* Support for multiple users
* More report plugins
* Website blocking
* Critical mode (shut down internet access until restored by
  accountability partner)
* Text alerts to cellphone
