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

const char*  LZMAT_COMPRESS_HEADER = "LZMT";

typedef struct
{
    unsigned int srcsize;
    char*        buffer;
}CompBlk;

///////////////////////////////////////////////////////////////

EnDeTool::EnDeTool()
 : origintext(NULL),
   encrypttext(NULL),
   cryptcontext(NULL),
   isencoded(false),
   doingcompress(false)
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

void EnDeTool::compress( bool enabled )
{
	doingcompress = enabled;
}

int  EnDeTool::encodebinary( const char* src, unsigned srcsize, char* &out )
{
    if ( ( src == NULL ) || ( srcsize == 0 ) )
        return -1;

    generateiv();
    AES_ctx* actx = (AES_ctx*)cryptcontext;
    AES_init_ctx_iv( actx,
                     (const uint8_t*)encryptkey ,
                     (const uint8_t*)encryptiv );

    int tmpCiperLen  = srcsize;

    if ( tmpCiperLen > AES_BLOCKLEN )
    {
        tmpCiperLen += AES_BLOCKLEN - ( tmpCiperLen % AES_BLOCKLEN );
    }
    else
    {
        // Let minimal 16 bytes
        tmpCiperLen = AES_BLOCKLEN;
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

    CompBlk* pcb = new CompBlk;
    if ( pcb == NULL )
        return 0;

    pcb->srcsize = srcsize;
    pcb->buffer = new char[ tmpCiperLen + 1 ];
    memset( pcb->buffer, 0, tmpCiperLen + 1 );
    memcpy( pcb->buffer, src, srcsize );

    int encloop = tmpCiperLen / AES_BLOCKLEN;
    if ( encloop == 0 )
    {
        encloop = 1;
    }

    for ( int cnt=0; cnt<encloop; cnt++ )
    {
        AES_CBC_encrypt_buffer( actx,
                                (uint8_t*)&pcb->buffer[ cnt*AES_BLOCKLEN ],
                                AES_BLOCKLEN );
    }

	if ( doingcompress == true )
	{
		unsigned comsz = compressbuffer( pcb->buffer, tmpCiperLen );
        
        out = new char[ comsz + 4 ];
        if ( out != NULL )
        {
            memcpy( &out[0], &pcb->srcsize, 4 );
            memcpy( &out[4], pcb->buffer, comsz );
            delete[] pcb->buffer;
            delete pcb;

            return comsz + 4;
        }
        else
        {
            // Error case.
            return 0;
        }
	}

    out = pcb->buffer;
    delete pcb;

    return tmpCiperLen;
}

int  EnDeTool::decodebinary( const char* src, unsigned srcsize, char* &out )
{
    if ( ( src == NULL ) || ( srcsize < AES_BLOCKLEN ) )
        return -1;

	char*           decbuff     = (char*)src;
	unsigned        decbuffsz   = srcsize - 4;
	bool            need2free   = false;
    unsigned int    realsz      = 0;

	// checks is it compressed ..
	if ( strncmp( &src[4], LZMAT_COMPRESS_HEADER, 4 ) == 0 )
	{
        realsz = *(unsigned int*)src;
		decbuff = new char[ srcsize - 4 ];

		if ( decbuff == NULL )
			return -1;

		memcpy( decbuff, &src[4], srcsize - 4 );
		decbuffsz = decompressbuffer( decbuff, srcsize - 4 );

		if ( decbuffsz < AES_BLOCKLEN )
			return -2;

		need2free = true;
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

    out = new char[ decbuffsz + 1 ];
    memset( out, 0, decbuffsz + 1 );
    memcpy( out, decbuff, decbuffsz );

	if ( need2free == true )
	{
		delete[] decbuff;
	}

    int decloop = decbuffsz / AES_BLOCKLEN;

    if ( decloop == 0 )
    {
        decloop = 1;
    }

    for (int cnt=0; cnt<decloop; cnt++ )
    {
        AES_CBC_decrypt_buffer( actx,
                                (uint8_t*)&out[cnt * AES_BLOCKLEN],
                                AES_BLOCKLEN );
    }

    if ( realsz>0 )
        return realsz;

    return decbuffsz;
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

		if ( doingcompress == true )
		{
			unsigned comsz = compressbuffer( tmpCiperBuff, tmpCiperLen );
			if ( comsz > 0 )
			{
				tmpCiperLen = comsz;
			}
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
		unsigned decsz = decompressbuffer( tmpCiperBuff, tmpCiperLen );
		if ( decsz > 0 )
		{
			tmpCiperLen = decsz;
			retI = decsz;
		}
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
                    memcpy( switchbuff + 0, LZMAT_COMPRESS_HEADER, 4 );
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
        if ( strncmp( buff, LZMAT_COMPRESS_HEADER, 4 ) == 0 )
        {
            //Check original size.
            MP_U32 olen = 0;
            memcpy( &olen, buff + 4, 4 );

            if ( olen > 0 )
            {
                MP_U8* rebuff = (MP_U8*)buff + 8;
                MP_U32 rebufflen = blen - 8;

                MP_U8* debuff = new MP_U8[ olen ];
                if ( debuff != NULL )
                {
                    int retcode = lzmat_decode( debuff, &olen,
                                                rebuff, rebufflen );

                    if ( retcode == LZMAT_STATUS_OK )
                    {
                        delete[] buff;
                        buff = (char*)debuff;

                        return olen;
                    }
                }
            }
        }
    }

    return 0;
}
