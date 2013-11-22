#ifndef NFCMAIN_H
#define NFCMAIN_H

 // NFCMAIN_H
#include "nfc/nfc-types.h"
int nfc_start();

int show_map( char *source,int srclen, char *dest,int deslen,int mode,bool bMapStarted);
void ChangeMapOn(int On);
int gsmString2Bytes(const char *pSrc, int nSrcLength, unsigned char* pDst);
void dualInterfaceUpload(char *website);

#endif
