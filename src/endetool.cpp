#ifdef DEBUG
#include <signal.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "base64.h"
#include "aes.hpp"
#include "lzmat.h"
#include "endetool.h"

///////////////////////////////////////////////////////////////

#define LIBENDETOOL_MAX_STRING_LENGTH       65535

///////////////////////////////////////////////////////////////

const unsigned int  LZMAT_COMPRESS_HEADER   = 0x544D5A4C;

///////////////////////////////////////////////////////////////

EnDeTool::EnDeTool()
 : origintext(NULL),
   encrypttext(NULL),
   cryptcontext(NULL),
   origintextlen(0),
   encryptedtextlen(0),
   paddedorigintextlen(0),
   isencoded(false),
   doingcompress(false)
{
    memset( encryptkey, 0, ENDETOOL_KEYLEN );
    memset( encryptiv, 0, ENDETOOL_KEYLEN );

    AES_ctx* aesctx = new AES_ctx;
    cryptcontext = (void*)aesctx;
}

EnDeTool::~EnDeTool()
{
    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;
    }

    if ( encrypttext != NULL )
    {
        delete[] encrypttext;
        encrypttext = NULL;
    }

    if ( cryptcontext != NULL )
    {
        AES_ctx* aesctx = (AES_ctx*)cryptcontext;
        cryptcontext = NULL;
        delete aesctx;
    }
}

void EnDeTool::reset()
{
    if ( encrypttext != NULL )
    {
        delete[] encrypttext;
        encrypttext = NULL;
    }

    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;
    }

    return;
}

void EnDeTool::compress( bool enabled )
{
    doingcompress = enabled;
}

long long EnDeTool::encodebinary( const char* src, unsigned srcsize, char* &out )
{
    if ( ( src == NULL ) || ( srcsize < AES_BLOCKLEN ) )
        return -1;

#ifdef DEBUG
    printf( "=== encodebianry:sourcetest:begin ===\n" );
    printf( "srcsize = %u\n", srcsize );
    for( size_t x=0; x<srcsize; x++ )
    {
        printf( "%c",src[x] );
    }
    printf( "\n=== encodebianry:sourcetest:end ===\n" );
    fflush( stdout );
#endif

    generateiv();
    AES_ctx* actx = (AES_ctx*)cryptcontext;
    AES_init_ctx_iv( actx,
                     (const uint8_t*)encryptkey ,
                     (const uint8_t*)encryptiv );

    unsigned encbuffsz = srcsize + 4;
    char* encbuff = new char[ encbuffsz ];
    if ( encbuff == NULL )
        return -2;

    // [----4----][---- + srcsize .... ]
    // srcsize    data ...
    memcpy( encbuff, &srcsize, 4 );
    memcpy( &encbuff[4], src, srcsize );

    if ( doingcompress == true )
    {
#ifdef DEBUG
        printf( "compressbuffer( %llu bytes );\n", encbuffsz );
        fflush( stdout );
#endif
        encbuffsz = compressbuffer( encbuff, encbuffsz );
        if ( encbuffsz == 0 ) /// failure case.
        {
            return -3; /// decompress failure.
        }
#ifdef DEBUG
        printf( "compressed %llu to %llu bytes.\n", encbuffsz, srcsize + 4 );
        fflush( stdout );
#endif
    }

    unsigned tmpCiperLen  = encbuffsz;

    if ( tmpCiperLen % AES_BLOCKLEN  > 0 )
    {
        tmpCiperLen += AES_BLOCKLEN - ( tmpCiperLen % AES_BLOCKLEN );
    }

    // resizing padded by AES_BLOCKLEN.
    if ( tmpCiperLen > encbuffsz )
    {
        char* swapbuff = new char[ tmpCiperLen ];
        if ( swapbuff == NULL )
        {
            return -4; /// cannot allocate swap buffer.
        }
        memset( swapbuff, 0, tmpCiperLen );
        memcpy( swapbuff, encbuff, encbuffsz );
        delete[] encbuff;
        encbuff = swapbuff;
    }

#ifdef DEBUG
    if ( srcsize < tmpCiperLen )
    {
        raise( 100 );
    }
#endif

    unsigned encloop = ( tmpCiperLen / AES_BLOCKLEN ) + 1;

    for ( unsigned cnt=0; cnt<encloop; cnt++ )
    {
        AES_CBC_encrypt_buffer( actx,
                                (uint8_t*)&encbuff[ cnt*AES_BLOCKLEN ],
                                AES_BLOCKLEN );
    }

    out = encbuff;

#ifdef DEBUG    
    printf( "=== encodebianry:out:begin ===\n" );
    printf( "tmpCiperLen = %u\n", tmpCiperLen );
    for( size_t x=0; x<tmpCiperLen; x++ )
    {
        printf( "%02X",(uint8_t)out[x] );
    }
    printf( "\n=== encodebianry:out:end ===\n" );
    fflush( stdout );
#endif /// of DEBUG

    return (long long)tmpCiperLen;
}

long long EnDeTool::decodebinary( const char* src, unsigned srcsize, char* &out )
{
    if ( ( src == NULL ) || ( srcsize < AES_BLOCKLEN ) )
        return -1;

#ifdef DEBUG
    printf( "=== decodebinary:sourcetest:begin ===\n" );
    printf( "srcsize = %u\n", srcsize );
    for( size_t x=0; x<srcsize; x++ )
    {
        printf( "%02X",(uint8_t)src[x] );
    }
    printf( "\n=== decodebinary:sourcetest:end ===\n" );
    fflush( stdout );
#endif

    unsigned        decbuffsz   = srcsize;
    unsigned int    realsz      = 0;

    if ( decbuffsz % AES_BLOCKLEN > 0 )
    {
        decbuffsz += AES_BLOCKLEN - ( decbuffsz % AES_BLOCKLEN );
    }
    // decode cipher first.
    generateiv();
    AES_ctx* actx = (AES_ctx*)cryptcontext;
    AES_init_ctx_iv( actx,
                     (const uint8_t*)encryptkey ,
                     (const uint8_t*)encryptiv );

    char* decptr = new char[ decbuffsz ];

    if ( decptr == NULL )
        return -2; /// memory allocation failure.
    
    memset( decptr, 0, decbuffsz );
    memcpy( decptr, src, srcsize );

    unsigned decloop = ( decbuffsz / AES_BLOCKLEN );

    for ( unsigned cnt=0; cnt<decloop; cnt++ )
    {
        AES_CBC_decrypt_buffer( actx,
                                (uint8_t*)&decptr[cnt * AES_BLOCKLEN],
                                AES_BLOCKLEN );
    }

    // checks is it compressed ..
    if ( memcmp( &decptr[4], &LZMAT_COMPRESS_HEADER, 4 ) == 0 )
    {
        unsigned worksz = srcsize - 4;

        // reallocate buffer for decompress buffer.
        decbuffsz = decompressbuffer( decptr, worksz );

        if ( decbuffsz < AES_BLOCKLEN )
        {
            delete[] decptr;
            return -4; /// decompress buffer error.
        }

        memcpy( &realsz, decptr, 4 );

        if ( realsz == 0 )
        {
            return -5; /// checking compressed buffer size check error.
        }

        if ( realsz != ( decbuffsz - 4 ) )
        {
            realsz = decbuffsz - 4;
        }
    }
    else
    {
        memcpy( &realsz, decptr, 4 );
    }

    if ( realsz > 0 )
    {
        out = new char[ realsz + 1 ];
        if ( out != NULL )
        {
            memset( out, 0, realsz + 1 );
 #ifdef DEBUG
            printf( "#TEST#\n" );
            printf( "decptr = %llX ~ %llX, size = %llu\n", 
                    decptr, decptr + decbuffsz, decbuffsz );
            printf( "out = %llX~%llX, size = %llu\n", 
                    out, out + realsz + 1, realsz );
            fflush( stdout );
 #endif /// of DEBUG
            memcpy( out, &decptr[4], realsz );
 #ifdef DEBUG
            printf( "=== decodebinary:out:begin ===\n" );
            printf( "realsz = %u\n", realsz );
            for( size_t x=0; x<realsz; x++ )
            {
                printf( "%c",out[x] );
            }
            printf( "\n=== decodebinary:out:end ===\n" );
            fflush( stdout );
 #endif /// of DEBUG
            delete[] decptr;
        }
        else
        {
            realsz = 0;
        }
    }

    return realsz;
}

void EnDeTool::text( const char* srctext )
{
    if ( srctext == NULL )
        return;

    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;
        origintextlen = 0;
        paddedorigintextlen = 0;

        isencoded = false;
    }

    origintextlen = strlen( srctext );

    if ( origintextlen >= LIBENDETOOL_MAX_STRING_LENGTH )
        return;

    // padding size by AES_BLOCKLEN
    paddedorigintextlen = origintextlen;

    if ( origintextlen % AES_BLOCKLEN > 0 )
    {
         paddedorigintextlen += AES_BLOCKLEN - ( origintextlen % AES_BLOCKLEN );
    }

    origintext = new char[ paddedorigintextlen ];
    if ( origintext != NULL )
    {
        memset( origintext, 0, paddedorigintextlen );
        memcpy( origintext, srctext, origintextlen );
        isencoded = encode();
    }
    else
    {
        isencoded = false;
    }
}

void EnDeTool::encodedtext( const char* srctext )
{
    if ( srctext == NULL )
        return;

    if ( encrypttext != NULL )
    {
        delete[] encrypttext;
        encrypttext = NULL;
    }

    unsigned srclen = strlen( srctext ) + 1;

    if ( srclen >= LIBENDETOOL_MAX_STRING_LENGTH )
        return;

    encrypttext = new char[ srclen ];
    if ( encrypttext != NULL )
    {
        memset( encrypttext, 0, srclen );
        strcpy( encrypttext, srctext );

        decode();
    }
}

void EnDeTool::cryptkey( const char* key, const char* iv )
{
    if ( key == NULL )
        return;

    size_t cpylen = strlen( key );

    if ( cpylen > ENDETOOL_KEYLEN )
    {
        cpylen = ENDETOOL_KEYLEN;
    }

    memset( encryptkey, 0, ENDETOOL_KEYLEN );
    memcpy( encryptkey, key, cpylen );
    memset( encryptiv, 0, ENDETOOL_KEYLEN );
    if ( iv != NULL )
    {
        cpylen = strlen( iv );

        if ( cpylen > ENDETOOL_KEYLEN )
        {
            cpylen = ENDETOOL_KEYLEN;
        }

        memcpy( encryptiv, iv, cpylen );
    }

    if ( isencoded == true )
    {
        isencoded = false;

        if ( encrypttext != NULL )
        {
            delete[] encrypttext;
            encrypttext = NULL;
        }
    }
}

bool EnDeTool::encode()
{
    if ( ( paddedorigintextlen == 0 ) || ( origintextlen == 0 ) )
        return false;

    char* tmpEncBuff = NULL;

    int tmpEncLen = (int)encodebinary( origintext, paddedorigintextlen, tmpEncBuff );
    if ( tmpEncLen > 0 )
    {
        char* tmpBase64Buff = NULL;
        int retI = base64_encode( tmpEncBuff, tmpEncLen, &tmpBase64Buff );
        if ( retI > 0 )
        {
            if ( encrypttext != NULL )
            {
                delete[] encrypttext;
                encrypttext = NULL;
            }

            encrypttext = new char[ retI + 1 ];
            if ( encrypttext != NULL )
            {
                memset( encrypttext, 0, retI + 1 );
                strncpy( encrypttext, tmpBase64Buff, retI );
            }
            else
            {
                return false;
            }
        }

        if ( tmpBase64Buff != NULL )
        {
            delete[] tmpBase64Buff;
            tmpBase64Buff = NULL;
        }

        return true;
    }

    return false;
}

bool EnDeTool::decode()
{
    unsigned tmpCiperLen  = strlen(encrypttext);
    if ( tmpCiperLen == 0 )
        return false;

    char* tmpCiperBuff = new char[ tmpCiperLen + 1 ];

    if ( tmpCiperBuff == NULL )
        return false;

    memset( tmpCiperBuff, 0, tmpCiperLen + 1 );

    int retI = base64_decode( encrypttext, (unsigned char*)tmpCiperBuff, tmpCiperLen );
    if ( retI == 0 )
        return false;

    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;
    }

    char* tmpDecdBuff = NULL;

    long long declen = decodebinary( tmpCiperBuff, (unsigned)retI, tmpDecdBuff );
    if ( declen > 0 )
    {
        origintext = tmpDecdBuff;
        delete[] tmpCiperBuff;
        return true;
    }

    if ( tmpCiperBuff != NULL )
    {
        delete[] tmpCiperBuff;
    }

    return false;
}

#ifdef DEBUG
void debug_printkey( const char* prefix, const char* key )
{
    printf( "%s = '", prefix );
    for( unsigned cnt=0; cnt<ENDETOOL_KEYLEN; cnt++ )
    {
        printf( "%c", key[cnt] );
    }
    printf( "'\n" );
}
#endif

void EnDeTool::generateiv()
{
    if ( encryptiv[0] == 0 )
    {
        // erase iv once more.
        memset( encryptiv, 0, ENDETOOL_KEYLEN );

        for( unsigned cnt=0; cnt<ENDETOOL_KEYLEN; cnt++ )
        {
           encryptiv[ cnt ] = encryptkey[ (ENDETOOL_KEYLEN-1)-cnt ];
        }
    }
}

unsigned EnDeTool::compressbuffer( char* &buff, unsigned blen )
{
    if ( ( buff != NULL ) && ( blen > 0 ) )
    {
        unsigned allocsz = MAX_LZMAT_ENCODED_SIZE( blen );
        MP_U8* compbuff = new MP_U8[ allocsz ];
        if ( compbuff != NULL )
        {
            MP_U32 complen = allocsz;

            int retcode = lzmat_encode( compbuff, &complen,
                                        (MP_U8*)buff, (MP_U32)blen );
            if ( retcode == LZMAT_STATUS_OK )
            {
                MP_U8* switchbuff = new MP_U8[ complen + 8 ];
                if ( switchbuff != NULL )
                {
                    // put header
                    memcpy( switchbuff + 0, &LZMAT_COMPRESS_HEADER, 4 );
                    memcpy( switchbuff + 4, &blen, 4 );
                    memcpy( switchbuff + 8, compbuff, complen );
                    delete[] buff;
                    buff = (char*)switchbuff;
                    delete[] compbuff;

                    return complen + 8;
                }
            }

            delete[] compbuff;
        }
    }

    return 0;
}

unsigned EnDeTool::decompressbuffer( char* &buff, unsigned blen )
{
    if ( ( buff != NULL ) && ( blen > 8 ) )
    {
        // check header
        if ( memcmp( buff, &LZMAT_COMPRESS_HEADER, 4 ) == 0 )
        {
            //Check original size.
            MP_U32* olen = (MP_U32*)&buff[4];

            if ( *olen > 0 )
            {
                MP_U32 buffolen = *olen;
                MP_U8* rebuff = (MP_U8*)buff + 8;
                MP_U32 rebufflen = blen - 8;

                MP_U8* debuff = new MP_U8[ buffolen ];
                if ( debuff != NULL )
                {
                    int retcode = lzmat_decode( debuff, &buffolen,
                                                rebuff, rebufflen );

                    if ( retcode == LZMAT_STATUS_OK )
                    {
                        delete[] buff;
                        buff = (char*)debuff;

                        return buffolen;
                    }
                }
            }
        }
    }

    return 0;
}
