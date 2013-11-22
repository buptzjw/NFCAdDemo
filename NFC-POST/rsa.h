#ifndef __RSA_H___
#define __RSA_H___

char * rsa_encrypt(char *pubN, char *priKey, char *plain);
char * rsa_decrypt(char *pubN, char *pubKey, char *cipher);
int InverseMod2power(int n, int s);
void modmul(unsigned char *X, unsigned char *Y, unsigned char *N, int size);
void modpow(unsigned char *base, unsigned char *exponent, unsigned char *modulus,unsigned char *result, int size);
void HEX_to_UINT(char *hex, unsigned char *uint);
void UINT_to_HEX(unsigned char *uint, int size, char *hex);
int compare(unsigned char *X, unsigned char *Y, int size);

#endif
