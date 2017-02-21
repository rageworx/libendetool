# libendetool

## What is this ?

* A C++ open source project for helping easily encode or decode string and stream data with AES-256 and BASE-64.

## Latest Update 

* 21-Jan-2017
    - Added visual studio projects to in vc90prj and vc11proj.
    - Enhanced some issues for Makefile
    - Updated AES-256 code.

## Used encoding.

* BASE-64 for encode and decode datas as ASCII ranged text string.
* AES-256
* LZMA-Tiny (may for compressing binary data in future)

## Supported compilers 

* gcc 32bit, 64bit ( inc. Apple X-Code )
* MinGW, MinGW-W64 32bit, 64bit
* Visual Studio 2008, 2012, and later

## Supported to encode and decode.

* Multibyte (or ASCII) strings
* UTF-8 strings
* UNICODE as binary encoding.

## External Licenses

* AES-256 
    - Author: Ilya O. Levin & Hal Finny
    - Homepage: http://www.literatecode.com/aes256
* LZMAT v1.01
    - Author: Vitaly Evseenko ( ve@matcode.com )
    - Homepage: http://www.matcode.com/lzmat.htm