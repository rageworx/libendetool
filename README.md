# libendetool

## What is this ?

* A C++ open source project for helping easily encode or decode string and stream data with AES-256 and BASE-64.

## Compatible
* Lastest version of libendetool not compatibles with 0.4-prerelease by updating AES-256 encryption method.

## Latest Update 

* 19-Nov-2019
    - Fixed an error case on libende size check modulates zero.

## Previous Update 

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

## Used encoding.

* BASE-64 for encode and decode datas as ASCII ranged text string.
* AES-256 CBC.
* LZMA-Tiny (may for compressing binary data in future)

## Supported compilers 

* LLVM GCC for Apple Mac OS.
* gcc 32bit, 64bit ( inc. Apple HPC gcc )
* MinGW, MinGW-W64 32bit, 64bit
* Visual Studio 2008, 2012, and later

## Supported to encode and decode.

* Multibyte (or ASCII) strings
* UTF-8 strings
* UNICODE as binary encoding.

## External Licenses

* tiny-AES-c (new)
    - Author: Kokke
    - Source repo.: https://github.com/kokke/tiny-AES-c

* LZMAT v1.01
    - Author: Vitaly Evseenko ( ve@matcode.com )
    - Homepage: http://www.matcode.com/lzmat.htm
