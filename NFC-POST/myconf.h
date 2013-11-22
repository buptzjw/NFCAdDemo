#ifndef MYCONF_H
#define MYCONF_H
#include <nfc/nfc-types.h>



typedef struct{
    nfc_device_desc_t pnddDevices;
    char *terminalId;
    char *serverIP;
    char *delay;
    char *width;
    char *height;
}ConfigInfo;

extern ConfigInfo configInfo;



#endif // MYCONF_H
