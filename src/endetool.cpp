#ifdef DEBUG
#include <signal.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "base64.h"
#include "aes.hpp"
#include "endetool.h"

///////////////////////////////////////////////////////////////

const char*  LZMAT_COMPRESS_HEADER = "LZMT";

///////////////////////////////////////////////////////////////

EnDeTool::EnDeTool()
 : origintext(NULL),
   encrypttext(NULL),
   cryptcontext(NULL),
   isencoded(false)
{
    memset( encryptkey, 0, 32 );
    memset( encryptiv, 0, 32 );

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

int  EnDeTool::encodebinary( const char* src, unsigned srcsize, char* &out )
{
    if ( ( src == NULL ) || ( srcsize < AES_BLOCKLEN ) )
        return -1;

    generateiv();
    AES_ctx* actx = (AES_ctx*)cryptcontext;
    AES_init_ctx_iv( actx,
                     (const uint8_t*)encryptkey ,
                     (const uint8_t*)encryptiv );

    size_t tmpCiperLen  = srcsize + 4;

    if ( ( tmpCiperLen % AES_BLOCKLEN ) > 0 )
    {
        tmpCiperLen += AES_BLOCKLEN - ( tmpCiperLen % AES_BLOCKLEN );
    }    
    
    if ( out != NULL )
    {
        delete[] out;
        out = NULL;
    }

#ifdef DEBUG
    if ( srcsize < tmpCiperLen )
    {
        raise( 100 );
    }
#endif

    uint8_t* encptr = new uint8_t[ tmpCiperLen ];

    if ( encptr == NULL )
    {
        return 0;
    }

    memset( encptr, 0, tmpCiperLen );
    memcpy( encptr, &srcsize, 4 );
    memcpy( &encptr[4], src, srcsize );

    size_t encloop = tmpCiperLen / AES_BLOCKLEN;
    for ( size_t cnt=0; cnt<encloop; cnt++ )
    {
        AES_CBC_encrypt_buffer( actx,
                                &encptr[ cnt*AES_BLOCKLEN ],
                                AES_BLOCKLEN );
    }

    out = (char*)encptr;

    return (int)tmpCiperLen;
}

int  EnDeTool::decodebinary( const char* src, unsigned srcsize, char* &out )
{
    if ( ( src == NULL ) || ( srcsize < AES_BLOCKLEN + 4 ) )
        return -1;

	char*           decbuff     = (char*)src;
	size_t          decbuffsz   = srcsize;
    unsigned int    realsz      = 0;

	// checks is it compressed ..
	if ( strncmp( src, LZMAT_COMPRESS_HEADER, 4 ) == 0 )
	{
        return -10; /// unsupported version.
	}

    generateiv();
    AES_ctx* actx = (AES_ctx*)cryptcontext;
    AES_init_ctx_iv( actx,
                     (const uint8_t*)encryptkey ,
                     (const uint8_t*)encryptiv );

    if ( out != NULL )
    {
        delete[] out;
        out = NULL;
    }

    uint8_t* decptr = new uint8_t[ decbuffsz ];

    if ( decptr == NULL )
        return 0;

    memcpy( decptr, decbuff, decbuffsz );

    size_t decloop = decbuffsz / AES_BLOCKLEN;
    for ( size_t cnt=0; cnt<decloop; cnt++ )
    {
        AES_CBC_decrypt_buffer( actx,
                                &decptr[ cnt*AES_BLOCKLEN ],
                                AES_BLOCKLEN );
    }

    // copy real size of data in 4 byte.
    memcpy( &realsz, decptr, 4 );

    if ( realsz > ( decbuffsz - 4 ) )
    {
        // this must be error case.
        delete[] decptr;
        return 0;
    }

    out = new char[ realsz ];
    if ( out == NULL )
    {
        delete[] decptr;
        return 0;
    }

    memcpy( out, &decptr[4], realsz );
    
    delete[] decptr;

    return (int)realsz;
}

void EnDeTool::text( const char* srctext )
{
    if ( srctext == NULL )
        return;

    if ( origintext != NULL )
    {
        delete[] origintext;
        origintext = NULL;

        isencoded = false;
    }

    int srclen = strlen( srctext ) + 1;
    origintext = new char[ srclen ];
    if ( origintext != NULL )
    {
        memset( origintext, 0, srclen );
        strcpy( origintext, srctext );

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

    int srclen = strlen( srctext ) + 1;
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

    int cpylen = strlen( key );
    if ( cpylen > 32 )
    {
        cpylen = 32;
    }

    memset( encryptkey, 0, 32 );
    strncpy( encryptkey, key, cpylen );

    memset( encryptiv, 0, 32 );
    if ( iv != NULL )
    {
        cpylen = strlen( iv );
        if ( cpylen > 32 )
        {
            cpylen = 32;
        }

        strncpy( encryptiv, iv, cpylen );
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
    if ( strlen(encryptkey) == 0 )
        return false;

    if ( cryptcontext == NULL )
        return false;

    generateiv();
    AES_ctx* actx = (AES_ctx*)cryptcontext;
    AES_init_ctx_iv( actx,
                     (const uint8_t*)encryptkey ,
                     (const uint8_t*)encryptiv );

    int   srcLen       = strlen(origintext);
    int   tmpCiperLen  = srcLen;

    // AES-256 encodes 16 bytes in once.
    // Make buffer pads with 16 multiply.
    if ( tmpCiperLen > AES_BLOCKLEN )
    {
        if ( ( tmpCiperLen % AES_BLOCKLEN ) != 0 )
        {
            tmpCiperLen += AES_BLOCKLEN - ( tmpCiperLen % AES_BLOCKLEN );
        }
    }
    else
    {
        // Let minimal 16 bytes
        tmpCiperLen = AES_BLOCKLEN;
    }

    int retI = 0;
    char* tmpBase64Buff = NULL;
    char* tmpCiperBuff = new char[ tmpCiperLen + 1 ];
    if ( tmpCiperBuff != NULL )
    {
        memset( tmpCiperBuff, 0, tmpCiperLen + 1 );
        memcpy( tmpCiperBuff, origintext, srcLen );

        int encloop = tmpCiperLen / AES_BLOCKLEN;
        if ( encloop == 0 )
        {
            encloop = 1;
        }

        for ( int cnt=0; cnt<encloop; cnt++ )
        {
            AES_CBC_encrypt_buffer( actx,
			                        (uint8_t*)&tmpCiperBuff[ cnt * AES_BLOCKLEN ],
                                    AES_BLOCKLEN );
        }

        int outBase64Len = tmpCiperLen * 2;

        retI = base64_encode( tmpCiperBuff, tmpCiperLen, &tmpBase64Buff );

        delete[] tmpCiperBuff;
        tmpCiperBuff = NULL;
    }
    else
    {
        return false;
    }

    if ( ( retI > 0 ) && ( tmpBase64Buff != NULL ) )
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

        free( tmpBase64Buff );
        tmpBase64Buff = NULL;

        return true;
    }

    return false;
}

bool EnDeTool::decode()
{
    if ( strlen(encryptkey) == 0 )
        return false;

    if ( cryptcontext == NULL )
        return false;

    if ( encrypttext == NULL )
        return false;

    generateiv();
    AES_ctx* actx = (AES_ctx*)cryptcontext;
    AES_init_ctx_iv( actx,
                     (const uint8_t*)encryptkey ,
                     (const uint8_t*)encryptiv );

    int tmpCiperLen  = strlen(encrypttext);
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

	// checks is it compressed ..
	if ( strncmp( tmpCiperBuff, LZMAT_COMPRESS_HEADER, 4 ) == 0 )
	{
        return -10; // unsupported version
	}

    int decloop = tmpCiperLen / AES_BLOCKLEN;
    if ( decloop == 0 )
    {
        decloop = 1;
    }

    for (int cnt=0; cnt<decloop; cnt++ )
    {
        AES_CBC_decrypt_buffer( actx,
                                (uint8_t*)&tmpCiperBuff[cnt * AES_BLOCKLEN],
                                AES_BLOCKLEN );
    }

    // Let make it erase wrong buffers.
    int clearLen = tmpCiperLen - retI;

    memset( &tmpCiperBuff[retI], 0, clearLen );

    origintext = tmpCiperBuff;

    return true;
}

#ifdef DEBUG
void debug_printkey( const char* prefix, const char* key )
{
    printf( "%s = '", prefix );
    for( unsigned cnt=0; cnt<32; cnt++ )
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
        memset( encryptiv, 0, 32 );

        for( unsigned cnt=0; cnt<32; cnt++ )
        {
           encryptiv[ cnt ] = encryptkey[ 31 - cnt ];
        }
    }

#ifdef DEBUG
    debug_printkey( "#DEBUG# EK", encryptkey );
    debug_printkey( "#DEBUG# IV", encryptiv );
#endif

}
