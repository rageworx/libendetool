#ifndef __ENDETOOL_H__
#define __ENDETOOL_H__

////////////////////////////////////////////////////////////////////////////////
// EnDeTool for Crypting data with AES256-CBC and BASE64. non-LZMAT
// ===========================================================================
// A Front-End of C++ class library for gcc/MinGW
// (C)Copyright 2014 to 2020 Raphael Kim.
//
////////////////////////////////////////////////////////////////////////////////

// Version means 1.0.3.3
#define ENDETOOL_VERSION    (0x01000302)

class EnDeTool
{
    public:
        EnDeTool();
        virtual ~EnDeTool();

    public:
        void reset();
        void cryptkey( const char* key, const char* iv = NULL );
        void text( const char* srctext );
        void encodedtext( const char* srctext );

    public:
        int  encodebinary( const char* src, unsigned srcsize, char* &out );
        int  decodebinary( const char* src, unsigned srcsize, char* &out );

    public:
        const char* text()          { return origintext; }
        const char* encodedtext()   { return encrypttext; }
        const char* key()           { return encryptkey; }
        bool        encoded()       { return isencoded; }

    protected:
        void        generateiv();

    protected:
        char  encryptkey[32];
        char  encryptiv[32];
        char* origintext;
        char* encrypttext;
        void* cryptcontext;
        bool  isencoded;
        
    protected:
        bool encode();
        bool decode();
};

#endif /// of __ENDETOOL_H__
