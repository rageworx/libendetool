#ifndef __ENDETOOL_H__
#define __ENDETOOL_H__

////////////////////////////////////////////////////////////////////////////////
// EnDeTool for Crypting data with AES256-CBC and BASE64.
// ===========================================================================
// A Front-End of C++ class library for gcc/MinGW
// (C)Copyright 2014 to 2020 Raphael Kim.
//
// [ WARNING ]
// * 1.1.x does not compatibles with 1.0.x
//
////////////////////////////////////////////////////////////////////////////////

// Version means 1.1.0.5
#define ENDETOOL_VERSION    (0x01010005)

class EnDeTool
{
    public:
        EnDeTool();
        virtual ~EnDeTool();

    public:
        void reset();
        void compress( bool enable = false );
        void cryptkey( const char* key, const char* iv = NULL );
        void text( const char* srctext );
        void encodedtext( const char* srctext );

    public:
        long long encodebinary( const char* src, unsigned srcsize, char* &out );
        long long decodebinary( const char* src, unsigned srcsize, char* &out );

    public:
        bool        compress()      { return doingcompress; }
        const char* text()          { return origintext; }
        const char* encodedtext()   { return encrypttext; }
        const char* key()           { return encryptkey; }
        bool        encoded()       { return isencoded; }

    protected:
        void        generateiv();
        unsigned    compressbuffer( char* &buff, unsigned blen );
        unsigned    decompressbuffer( char* &buff, unsigned blen );

    protected:
        char        encryptkey[32];
        char        encryptiv[32];
        char*       origintext;
        char*       encrypttext;
        unsigned    origintextlen;
        unsigned    paddedorigintextlen;
        unsigned    encryptedtextlen;
        void*       cryptcontext;
        bool        isencoded;
        bool        doingcompress;

    protected:
        bool encode();
        bool decode();
};

#endif /// of __ENDETOOL_H__
