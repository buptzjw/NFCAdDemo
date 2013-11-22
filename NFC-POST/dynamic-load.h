#ifndef DYNAMICLOAD_H
#define DYNAMICLOAD_H

//unsigned char * read_file(const char *filename,int maxlen);
int read_file(const char *filename,unsigned char *hex, int maxlen);
void UINT_to_HEX(unsigned char *uint, int size, char *hex);
void HEX_to_UINT(char *hex, unsigned char *uint);
void ToHex(const char *src,int srclen,unsigned char *hex );
//void ToHex(const unsigned char *src,int srclen , unsigned char* hex);
//unsigned char* ToHex(const char *src,int srclen);
//unsigned char fun(char c);
#endif // DYNAMICLOAD_H
