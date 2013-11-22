/*-
 * Public platform independent Near Field Communication (NFC) library
 * 
 * Copyright (C) 2011, Romuald Conty, Romain Tartière
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

 /**
 * @file nfc-emulate.c
 * @brief Provide a small API to ease emulation in libnfc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <unistd.h>
#include <nfc/nfc.h>
#include <nfc/nfc-emulation.h>

#include "iso7816.h"

#include "ndeffile.h"
#include "nfc/nfc-types.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <nfc-main.h>
#include "log.h"

bool bCanBeStop = true;

nfc_device_t* g_pnd;
char *iccid_str;

//static nfc_device_t *pnd = NULL;
//unsigned char *iccid;
//char iccid[10] = {};


//从卡端读取用户的iccid信息
char *load_user_info(nfc_device_t *pnd)
{
    char iccid[10] = {0};
    nfc_device_desc_t *pnddDevices;
    bool    res;
    byte_t pbrData[256];
    int i;
    iccid_str = (char *)malloc(sizeof(char)*10);
    memset(iccid_str,0,sizeof(iccid_str));

    if (pnd == NULL) {
        printf("Unable to connect to NFC device.");
        exit (EXIT_FAILURE);
    }

    printf ("Connected to NFC reader: %s\n", pnd->acName);
    printf ("NFC reader is on! \n");

    //SAMConfiguration
    memset(pbrData,0,sizeof(pbrData));
    byte_t pbtData1[] = { 0x14, 0x01, 0x00, 0x00};
    res = pn532_uart_send(pnd,pbtData1,sizeof(pbtData1));
    if(!res){
        printf ("SAMConfiguration pn532_uart_send ERROR!");
        return NULL;
    }

    int n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
    printf("SAMConfiguration receive %d\n",n);

    //List Passive Target
    memset(pbrData,0,sizeof(pbrData));
    byte_t pbtData2[] = { 0x4A, 0x01, 0x00};
    res = pn532_uart_send(pnd,pbtData2,sizeof(pbtData2));
    if(!res){
        TraceLog("%s","List Passive Target pn532_uart_send ERROR!");
        return NULL;
    }
    n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
    if(n <= 0){
        TraceLog("%s","List Passive Target:n<0");
        return NULL;
    }

    //Select APP on mobile phone
    memset(pbrData,0,sizeof(pbrData));
    byte_t pbtData3[] = { 0x40, 0x01, 0x00, 0xA4, 0x04, 0x00, 0x0D, 0x62, 0x75, 0x70, 0x74, 0x2E, 0x6E, 0x65, 0x77, 0x70, 0x75, 0x73,0x68,0x2e};
    res = pn532_uart_send (pnd,pbtData3,sizeof(pbtData3));
    if(!res){
        TraceLog("%s","Select APP pn532_uart_send ERROR!");
        printf ("Select APP pn532_uart_send ERROR!");
        return NULL;
    }
    //printf("%d",res);
    n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));
    if(n <= 0){
        TraceLog("%s","Select APP pn532_uart_receive ERROR!");
        printf ("Select APP pn532_uart_receive ERROR!");
        return NULL;
    }
    if(pbrData[0] != 0x00){
        TraceLog("%s","Select APP Status Error!");
        printf("Select APP Status Error! \n");
        return NULL;
    }
    if((pbrData[1] == 0x90)&&(pbrData[2] == 0x00)){
        TraceLog("%s","Select APP success.");
        printf("Select APP success \n");
    }

    //load the ICCID from card
    memset(pbrData,0,sizeof(pbrData));
    byte_t pbtData4[] = { 0x40, 0x01, 0x00, 0x73, 0x00, 0x00, 0x0A};
    res = pn532_uart_send (pnd,pbtData4,sizeof(pbtData4));
    if(!res){
        TraceLog("%s","load the ICCID from card pn532_uart_send ERROR!");
        printf ("load the ICCID from card pn532_uart_send ERROR!");
        return NULL;
    }
    //printf("%d",res);
    n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));
    if(n <= 0){
        TraceLog("%s","load the ICCID from card pn532_uart_receive ERROR!");
        printf ("load the ICCID from card pn532_uart_receive ERROR!");
        return NULL;
    }
    if(pbrData[0] == 0x00){
        for(i=0;i<10;i++){
            iccid[i] = pbrData[i+1];
        }
        for(i=0;i<10;i++){
            *(iccid_str+i) = iccid[i];
        }
    }
    return iccid_str;
}

int
nfc_emulate_target (nfc_device_t* pnd, struct nfc_emulator *emulator)
{
//    int i=0;
    byte_t abtRx[ISO7816_SHORT_R_APDU_MAX_LEN];
    size_t szRx = sizeof(abtRx);
    byte_t abtTx[ISO7816_SHORT_C_APDU_MAX_LEN];
    int res = 0;
    //  init_sigaction();
    //  init_time(10);
    g_pnd = pnd;
    bCanBeStop = true;
    if (!nfc_target_init (pnd, emulator->target, abtRx, &szRx)) {
        return -1;
    }
    bCanBeStop = false;
    g_pnd = NULL;

    while (res >= 0) {
        res = emulator->state_machine->io (emulator, abtRx, szRx, abtTx, sizeof (abtTx));
        if (res > 0) {
            //TraceLog("%s","abtTx:");
           // TraceLog("%s",(const char *)abtTx);
            //printf("@@@");
            if (!nfc_target_send_bytes(pnd, abtTx, res)) {
                return -1;
            }
        }
        if (res >= 0) {
            //TraceLog("%s","abtRx:");
            //TraceLog("%s",(const char *)abtRx);
            //printf("%%%%");
            if (!nfc_target_receive_bytes(pnd, abtRx, &szRx)) {
                return -1;
            }
        }
    }
    return 0;
}
