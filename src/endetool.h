#ifndef __ENDETOOL_H__
#define __ENDETOOL_H__

////////////////////////////////////////////////////////////////////////////////
// EnDeTool for Crypting data with AES256-CBC and BASE64.
// ===========================================================================
// A Front-End of C++ class library for gcc/MinGW
// (C)Copyright 2014, 2019 Raphael Kim.
//
////////////////////////////////////////////////////////////////////////////////

class EnDeTool
{
    public:
        EnDeTool();
        virtual ~EnDeTool();

    public:
        void reset();
        void compress( bool enable = false );
        void cryptkey( const char* key );
        void text( const char* srctext );
        void encodedtext( const char* srctext );

    public:
        int  encodebinary( const char* src, unsigned srcsize, char* &out );
        int  decodebinary( const char* src, unsigned srcsize, char* &out );

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
        char  encryptkey[32];
        char  encryptiv[32];
        char* origintext;
        char* encrypttext;
        void* cryptcontext;
        bool  isencoded;
        bool  doingcompress;

    protected:
        bool encode();
        bool decode();
};

#endif /// of __ENDETOOL_H__
