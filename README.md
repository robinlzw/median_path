# Median Path

## What is it
This project is meant to compute, process, analyse and render medial skeletons.
It is based on my research code base that I progressively put into this 
open source project during my free time. It is far from finished, so keep in
mind that documentation is still missing and bugs are not yet corrected. In
particular, some people report bugs in CGAL code when building the skeletal
structure benchmark with the benchmark application while it works perfectly
fine with the skeletonizer application.

## Configuration
The code has been tested on a limited set of platforms, including:
- GNU/Linux platforms, compiled with gcc 5.2, 5.3, 6.1 and 6.2
- ~~Windows 64 bits, with Visual Studio 14~~ not supported anymore. I never use
the Windows version due to the lack of OpenMP support in MSVC. Thus, I prefer to use
my free time to work only on the Linux version, for now.

## Installation
Sorry, no documentation about the installation yet. Ask me or dig into the 
CMakeLists.txt file.

## Licensing
This code is release under the MIT Licence. Please see the file called LICENCE.

## Contacts
Thomas Delame, tdelame@gmail.com