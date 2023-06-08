-----DESCRIPTION-----
Net Responsibility is intended to be a good open source accountability software
for all major platforms. It's purpose is to prevent people from certain surfing
habits by holding them accountable to others. 
This is done by logging all internet traffic and generating clear reports that
are emailed regularly to the accountability partner(s).




-----INSTALLATION-----
Install [VCPKG](https://vcpkg.io/en/), the C++ Package Manager.

Build as a CMAKE Project

```
cd netresponsibility
cmake -S . -B cmake-build-debug -DCMAKE_TOOLCHAIN_FILE={path to vcpkg}/scripts/buildsystems/vcpkg.cmake
cmake --build cmake-build-debug
```



-----CONTACT-----
Find the source code at https://github.com/kkeiper1103/netresponsibility-fork




-----LICENSE-----
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.




-----CONTRIBUTE-----
If you want to help us out with this project, please read the file DEVELOPERS 
and contact us. There are several ways in which you can help, depending on 
your skills.




-----CREDITS-----
Net Responsibility was first developed by Scott Severence. Without his efforts 
this project would probably not be what it is. His code is found in all versions 
up to 2.0.2.
This project heavily relies on the Poco C++ Library, found at 
www.pocoproject.org. This library have made the whole journey a lot easier!
Other software with the same purpose, mainly x3watch and Covenant Eyes, have 
been a big inspiration. If Net Responsibility is not working good enough for 
you, consider using one of these.
