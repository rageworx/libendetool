#ifndef __BASE64_H__
#define __BASE64_H__

#ifdef __cplusplus
extern "C" {
#endif

int base64_encode(const char *text, int numBytes, char **encodedText);
int base64_decode(const char *text, unsigned char *dst, int numBytes );

#ifdef __cplusplus
}
#endif /// of extern "C"

#endif // of __BASE64_H__
