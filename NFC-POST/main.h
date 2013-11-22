#ifndef MAIN_H
#define MAIN_H
#endif

#include <nfc/nfc-types.h>

#ifdef _cplusplus
extern "C" void show_map( char *source,int srclen, char *dest,int deslen,int mode);
extern "C" void show_map_end();
//extern "C" {
//    typedef struct{
//        nfc_device_desc_t pnddDevices;
//        char *terminalId;
//        char *serverIP;
//        int delay;
//    }ConfigInfo;
//    extern ConfigInfo configInfo;
//}


//#include <nfc/nfc-types.h>

#endif




void start_video_main(int TAG_NUM);
int loadXMLFile_tag();
void copyNdef(int len,unsigned char *indata);
 // MAIN_H
