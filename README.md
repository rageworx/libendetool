# libendetool

## What is this ?

* A C++ open source project for helping easily encode or decode string and stream data with AES-256 and BASE-64.

## Compatibility with old versions
* Please remind this version issues,
  * 1.1.x.y not compatible with 1.0.x.y. 
  * 1.1.x.y and 1.0.x.y are not compatible with 0.x.y.z
* Please use tagged versions if needed for each different compatibility dependencies.

## Latest Update

* 17-June-2020
	- Version 1.1.3.6
	- Supporting variable AES cipher levels for each 256, 192 and 128.
	- Fixed many minor bugs like unstable memory broken or AES key length issues.
	- NOT COMPATIBLE with v1.0.3.3.

## Previous Updates 

* 20-July-2020
  - Version 1.1.0.5
  - Big changes for doing compress buffer before AES-256 encoding when compression flag enabled.
  - Compressed buffer may not compatible with version 1.0.3.3.
  - Use seperately 1.0.3.3. and 1.1.0.5 if user using compressed option. 
* 16-June-2020
	- Version 1.0.3.3
	- Fixed variable bugs.
* 01-Dec-2019
    - Hided original size of data.
	- Version 1.0.3.0 not compatible with previous encrypted data.
* 21-Nov-2019
    - Fixed decode binary returns not original buffer size.
	- version updated to 1.0.2.2
* 19-Nov-2019
    - Fixed an error case on libende size check modulates zero.
* 26-Sep-2019
    - able to set IV through encryptkey(). 
* 16-May-2019
    - Updated AES-56 with Kokke's tine-AES-c source.
    - Fixed a bug not releases AES context memory.
    - Removed old AES-256-ECB sources.
* 10-Aril-2019
    - Fixed key length limited in 8 bytes to 32 bytes.
* 04-March-2017
    - Included LLVM-GCC make file for MAC OS.
* 21-Jan-2017
    - Added visual studio projects to in vc90prj and vc11proj.
    - Enhanced some issues for Makefile
    - Updated AES-256 code.

## Supporting encodings.

* BASE-64 for encode and decode method as simple text string
* AES CBC with 128, 192 and 256 bits
* LZMA-Tiny compression for smaller binary data

## Supported compilers 

* Xcode (llvm) for Apple Mac OS up to Big Sur
* POSIX gcc
* Visual Studio 2008, 2012, and later

## Supported to encode and decode.

* UTF-8, MBCS strings by simple text() and encodedtext() method
* wide charactors as binary encoding

## External Licenses

* tiny-AES-c
    - Author: Kokke
    - Source repo.: https://github.com/kokke/tiny-AES-c

* LZMAT v1.01
    - Author: Vitaly Evseenko ( ve@matcode.com )
    - Homepage: http://www.matcode.com/lzmat.htm
