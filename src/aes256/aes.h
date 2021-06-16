#ifndef _AES_H_
#define _AES_H_

/*
    -- variable key length each 16, 24, 32 AES variant source,
    -- (C)2021 Raphael K,
*/

#include <stdint.h>

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// CTR enables encryption in counter-mode.
// ECB enables the basic ECB 16-byte block algorithm. All can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.

#ifndef CBC
  #define CBC 1
#endif

#ifndef ECB
  #define ECB 1
#endif

#ifndef CTR
  #define CTR 1
#endif

//Default is AES256
#define AES256 1

//Block length in bytes AES is 128b block only
#define AES_BLOCKLEN        16

//Key length types, each 256, 192 and 128
typedef enum {
    AES_256     = 0,
    AES_192,
    AES_128
}AEStype;

#define AES_KEYLEN_32       32
#define AES_KEYLEN_24       24
#define AES_KEYLEN_16       16
#define AES_KEYEXPLEN_32    240
#define AES_KEYEXPLEN_24    208
#define AES_KEYEXPLEN_16    176

#define AES_KEYLEN_MAX      AES_KEYLEN_32
#define AES_KEYEXPLEN_MAX   AES_KEYEXPLEN_32

#if defined(AES256) && (AES256 == 1)
    #define AES_KEYLEN      AES_KEYLEN_32
    #define AES_keyExpSize  AES_KEYEXPLEN_32
#elif defined(AES192) && (AES192 == 1)
    #define AES_KEYLEN      AES_KEYLEN_24
    #define AES_keyExpSize  AES_KEYEXPLEN_24
#else
    #define AES_KEYLEN      AES_KEYLEN_16 
    #define AES_keyExpSize  AES_KEYEXPLEN_16
#endif

struct AES_ctx
{
    uint32_t    RoundKeyLength;
    uint32_t    CipherRounds;
    uint8_t     RoundKey[AES_KEYLEN_MAX];
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
    uint8_t     Iv[AES_BLOCKLEN];
#endif
};

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key, AEStype atype);
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv, AEStype atype);
void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv);
#endif

#if defined(ECB) && (ECB == 1)
// buffer size is exactly AES_BLOCKLEN bytes; 
// you need only AES_init_ctx as IV is not used in ECB 
// NB: ECB is considered insecure for most uses
void AES_ECB_encrypt(const struct AES_ctx* ctx, uint8_t* buf);
void AES_ECB_decrypt(const struct AES_ctx* ctx, uint8_t* buf);

#endif // #if defined(ECB) && (ECB == !)


#if defined(CBC) && (CBC == 1)
// buffer size MUST be mutile of AES_BLOCKLEN;
// Suggest https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx via AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 
void AES_CBC_encrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);
void AES_CBC_decrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);

#endif // #if defined(CBC) && (CBC == 1)


#if defined(CTR) && (CTR == 1)

// Same function for encrypting as for decrypting. 
// IV is incremented for every block, and used after encryption as XOR-compliment for output
// Suggesting https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx with AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 
void AES_CTR_xcrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, uint32_t length);

#endif // #if defined(CTR) && (CTR == 1)


#endif //_AES_H_
