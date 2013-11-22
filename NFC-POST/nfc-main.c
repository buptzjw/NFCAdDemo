#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "main.h"

#include "myconf.h"
extern ConfigInfo configInfo;

//#include "map.h"

#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <string.h>
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <signal.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include "nfc-main.h"
#include "rsa.h"
#include "chips/pn53x-internal.h"
#include "chips/pn53x.h"
#include "drivers.h"
#include "nfc-internal.h"
#include "nfc-utils.h"
#include "nfc/nfc.h"
#include "pn532_uart.h"
#include "nfc/nfc-types.h"
#include "dynamic-load.h"
#include "nfc-ndef.h"
#include "nfc/nfc-emulation.h"
#include "nfc_emulate_card.h"
#include "nfc-emulate-forum-tag4.c"
#include "nfc-emulation.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <log.h>
#include "ndeffile.h"
#include "pwm.h"

//#include "webtransaction.h"


#define SYMBOL_PARAM_fISO14443_4_PICC   0x20
/* C-ADPU offsets */
#define CLA  0
#define INS  1
#define P1   2
#define P2   3
#define LC   4
#define DATA 5
#define ISO144434A_RATS 0xE0

static nfc_device_t *pnd=NULL;
nfc_device_desc_t *pnddDevices;


//size_t  i,j,k,l;
//size_t  sourceLength,destLength;
//bool verbose = false;
//bool    res;
//byte_t pbrData[256];

//byte_t cipherDataWithFrame[256];
//byte_t cipherDataWithoutFrame[128];
//byte_t cipherDataWithoutFrame_reverse[128];
//byte_t QueryDataWithFrame[256];
//byte_t QueryDataWithoutFrame[256];

static  bool bMapOn;

void stop_polling (int sig)
{
    (void) sig;
    if (pnd)
        nfc_abort_command (pnd);
    else
        exit (EXIT_FAILURE);
}



int gsmString2Bytes(const char *pSrc, int nSrcLength, unsigned char* pDst)
{

    int i;
    for (i = 0; i < nSrcLength; i++){
    // 输出高4位
    unsigned char high=*pSrc>>4&0x0f;
    unsigned char low=*pSrc&0x0f;
        if ((high >= 0) && (high <= 9)){
            *pDst = high+'0';
        }

        else{
            *pDst = high + 'A' - 10;
        }
        pDst++;
        // 输出低4位
        if ((low >= 0) && (low <= 9)){
            *pDst = low +'0';
        }
        else{
            *pDst = low + 'A' - 10;
        }
        pSrc++;
        pDst++;
    }
    // 返回目标数据长度
    return (nSrcLength * 2);
}

void nfc_reader(nfc_device_desc_t *pnddDevices, int times){

    char c_pubN[128];
    char c_pubKey[3];
    memset(c_pubN,0,sizeof(c_pubN));
    memset(c_pubKey,0,sizeof(c_pubKey));

    size_t  i,j,k,l;

    size_t  sourceLength,destLength;

    bool    res;
    byte_t pbrData[256];
    byte_t QueryDataWithFrame[256];

    int mode;


//    char *g_pubN = "AA030736A1480CC78576531EDF2D1153C18C22F1D0CAA5DBC7B52E3183B6BCC24F8853F7621F6E9473827DC32789E12F2D324BAE4C53A26F71ECF77C78914E107CC76B25507946F68A5D0FF9BD9D1EBB4234B2482E3824C2D3F55EA1D108DA2CF7F1B48D23FF7508DDCD4824E5E1D7C5B74FFB4A7F3492B3E0E3EEF3EA7B76F1";
//    char *g_pubKey = "10001";
//    char *g_priKey = "5F074248EEED6EF7374A643BA5537393F5637744E307E29D6E8B7F3E7DF8EE994E0B4DA3C15D005A4A166E655EFC5ABA1BF1390C3BEDA3A5D770350585FEF23AABCA9774C98AD60015926D4D843C82F52E52923A41F2387F7A54CBAD6416543549FED54ACE2D21F5B6B9B593549E57FEE48AA4FAFDF4C12B47EA34B4574DFAE1";

//    bool verbose = false;
//    byte_t pbrDatabuf[256];
//    char *randData_char;
//    char *plain;
//    randData_char = (char *)malloc(sizeof(char)*4);
//    plain = (char *)malloc(sizeof(char)*4);

//    char *plain_receive;
//    plain_receive = (char *)malloc(sizeof(char)*4);

//    char *cipherResult_send;
//    cipherResult_send = (char*)malloc(sizeof(char)*129);

//    char *cipherResult_send_hex;
//    cipherResult_send_hex = (char *)malloc(sizeof(char)*260);

//    unsigned char *cipherResult_send_hex;
//    cipherResult_send_hex = (unsigned char*)malloc(sizeof(char)*129);

//    byte_t cipherDataWithFrame_receive[256];
//    byte_t cipherDataWithoutFrame_receive[128];
//    byte_t cipherDataWithoutFrame[128];
//    byte_t QueryDataWithoutFrame[256];

    signal (SIGINT, stop_polling);
//  进行NFC设备的连接
    pnd = pn532_uart_connect (pnddDevices);

    if (pnd == NULL) {
        TraceLog("%s","Unable to connect to NFC device.");
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
        TraceLog("%s","SAMConfiguration pn532_uart_send ERROR!");
        printf ("SAMConfiguration pn532_uart_send ERROR!");
    }
    int n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));

    if(n < 0){
        printf("SAMConfiguration receive %d\n",n);
        goto theend;
    }

    //List Passive Target
    memset(pbrData,0,sizeof(pbrData));
    byte_t pbtData2[] = { 0x4A, 0x01, 0x00};
    res = pn532_uart_send(pnd,pbtData2,sizeof(pbtData2));
    if(!res){
        TraceLog("%s","List Passive Target pn532_uart_send ERROR!");
        printf ("List Passive Target pn532_uart_send ERROR!\n");
    }

    n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
    printf("List Passive Target%d",n);
    int iv=0;
    while( n<=0 ){
        if(iv++ > times){
            goto theend;
        }

        pn532_uart_send(pnd,pbtData2,sizeof(pbtData2));
        n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
//        sleep(0.5);
    }

    if(pbrData[0] != 0x01){
        TraceLog("%s","List Passive Target pn532_uart_receive ERROR!");
        printf("List Passive Target pn532_uart_receive ERROR!\n");
        goto theend;
    }
    printf("List Passive Target success ! \n");

    unsigned char content[256];
    unsigned char mapsrc[256];
    unsigned char mapdes[256];

    byte_t mapsrcHEX[256];
    byte_t mapdesHEX[256];
    byte_t mapconHEX[256];

    memset(mapsrcHEX,0,sizeof(mapsrcHEX));
    memset(mapdesHEX,0,sizeof(mapdesHEX));
    memset(content,0,sizeof(content));

    //区分swp卡和双界面卡
    if((pbrData[15] == 0x90)||(pbrData[15] == 0x42)||(pbrData[15] == 0x48)||(pbrData[15] == 0xA1)){
        mode = 1;
        TraceLog("%s","use swp card.");
        printf("use swp card\n");
        //Select APP on mobile phone
        memset(pbrData,0,sizeof(pbrData));
        //byte_t pbtData3[] = { 0x40, 0x01, 0x00, 0xA4, 0x04, 0x00, 0x0A, 0x62, 0x75, 0x70, 0x74, 0x2E, 0x64, 0x65,0x6d, 0x6f, 0x2e};
        byte_t pbtData3[] = { 0x40, 0x01, 0x00, 0xA4, 0x04, 0x00, 0x0D, 0x62, 0x75, 0x70, 0x74, 0x2E, 0x6E, 0x65, 0x77, 0x70, 0x75, 0x73,0x68,0x2e};
        res = pn532_uart_send (pnd,pbtData3,sizeof(pbtData3));
        if(!res){
            TraceLog("%s","Select APP pn532_uart_send ERROR!");
            printf ("Select APP pn532_uart_send ERROR!");
            goto theend;
        }
        //printf("%d",res);
        n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));
        if(n <= 0){
            TraceLog("%s","Select APP pn532_uart_receive ERROR!");
            printf ("Select APP pn532_uart_receive ERROR!");
            goto theend;
        }

        if(pbrData[0] != 0x00){
            TraceLog("%s","Select APP Status Error!");
            printf("Select APP Status Error! \n");
            goto theend;
        }
        //Map search
        memset(QueryDataWithFrame,0,sizeof(QueryDataWithFrame));
        //test mobile
        byte_t pbtData5[] = { 0x40, 0x01, 0x00, 0x76, 0x00, 0x00,0x1E};

        pn532_uart_send (pnd,pbtData5,sizeof(pbtData5));
        n = pn532_uart_receive(pnd,QueryDataWithFrame,sizeof(QueryDataWithFrame));
        printf("QueryDataWithFrame length:%d \n",n);
        if(QueryDataWithFrame[0] != 0x00){
            TraceLog("%s","QueryData Error!");
            printf("QueryData Error! \n");
            goto theend;
        }

        for(l = 0;l < n-3;l++){
           content[l] = QueryDataWithFrame[l+1];
           if(n>50){
               goto theend;
           }
        }

        sourceLength = content[0];
        destLength = content[sourceLength+1];

        for(l=0;l<sourceLength+1;l++){
            mapsrc[l]=content[l+1];
        }
        for(l=0;l<destLength+1;l++){
            mapdes[l]=content[l+2+sourceLength];
        }
    }
    else{
        TraceLog("%s","use dual interface card.");
        printf("use dual interface card\n");
        mode = 0;
        //Select APP on mobile phone 1
        memset(pbrData,0,sizeof(pbrData));
        byte_t pbtData3[] = { 0x40, 0x01, 0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
        res = pn532_uart_send (pnd,pbtData3,sizeof(pbtData3));
        if(!res){
            TraceLog("%s","Select APP1 pn532_uart_send ERROR!");
            printf ("Select APP1 pn532_uart_send ERROR!");
            goto theend;
        }
        n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));
        if(n <= 0){
            TraceLog("%s","Select APP1 pn532_uart_receive ERROR!");
            printf ("Select APP1 pn532_uart_receive ERROR!");
            goto theend;
        }
        if(pbrData[0] != 0x00){
            TraceLog("%s","Select APP1 on mobile phone 1 Error!");
            printf("Select APP1 on mobile phone 1 Error! \n");
            goto theend;
        }
        //Select APP on mobile phone 2
        memset(pbrData,0,sizeof(pbrData));

        byte_t pbtData4[] = { 0x40, 0x01, 0x00, 0xA4, 0x00, 0x00, 0x02, 0xDF, 0x03};
        res = pn532_uart_send (pnd,pbtData4,sizeof(pbtData4));
        if(!res){
            TraceLog("%s","Select APP2 pn532_uart_send ERROR!");
            printf ("Select APP2 pn532_uart_send ERROR!");
            goto theend;
        }
        n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));
        if(n <= 0){
            TraceLog("%s","Select APP2 pn532_uart_receive ERROR!");
            printf ("Select APP2 pn532_uart_receive ERROR!");
            goto theend;
        }
        if(pbrData[0] != 0x00){
            TraceLog("%s","Select APP on mobile phone 2 Error!");
            printf("Select APP on mobile phone 2 Error! \n");
            goto theend;
        }
        //Select APP on mobile phone 3
        memset(pbrData,0,sizeof(pbrData));
        byte_t pbtData5[] = { 0x40, 0x01, 0x00, 0xA4, 0x00, 0x00, 0x02, 0xEF, 0x02};
        res = pn532_uart_send (pnd,pbtData5,sizeof(pbtData5));
        if(!res){
            TraceLog("%s","Select APP3 pn532_uart_send ERROR!");
            printf ("Select APP3 pn532_uart_send ERROR!");
            goto theend;
        }
        n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));
        if(n <= 0){
            TraceLog("%s","SSelect APP3 pn532_uart_receive ERROR!");
            printf ("Select APP3 pn532_uart_receive ERROR!");
            goto theend;
        }
        if(pbrData[0] != 0x00){
            TraceLog("%s","Select APP on mobile phone 3 Error!");
            printf("Select APP on mobile phone 3 Error! \n");
            goto theend;
        }

        //Map search
        memset(QueryDataWithFrame,0,sizeof(QueryDataWithFrame));
        //test mobile
        byte_t pbtData6[] = { 0x40, 0x01, 0x00, 0xB0, 0x00, 0x00,0x20};
        pn532_uart_send (pnd,pbtData6,sizeof(pbtData6));
        n = pn532_uart_receive(pnd,QueryDataWithFrame,sizeof(QueryDataWithFrame));
        printf("\nQueryDataWithFrame length:%d \n",n);

        if(QueryDataWithFrame[0] != 0x00){
            printf("QueryData Error! \n");
            goto theend;
        }

        for(l = 0;l < n-3;l++){
           content[l] = QueryDataWithFrame[l+3];
        }

        destLength = content[0];
        sourceLength = content[destLength+2];
        for(l=0;l<destLength;l++){
            mapdes[l]=content[l+1];
        }
        for(l=0;l<sourceLength;l++){
            mapsrc[l]=content[l+3+destLength];
        }

    }

    int conLen = sourceLength+destLength+2;
    int mapsrcln = gsmString2Bytes(mapsrc,sourceLength,mapsrcHEX);
    int mapdesln = gsmString2Bytes(mapdes,destLength,mapdesHEX);
    int mapconln = gsmString2Bytes(content,conLen,mapconHEX);

    write(STDOUT_FILENO, mapsrcHEX, mapsrcln);
    write(STDOUT_FILENO, mapdesHEX, mapdesln);
    write(STDOUT_FILENO, mapconHEX, mapconln);
    bool bMapStarted = false;
    if((mapsrcln==0)||(mapdesln==0)){
        TraceLog("%s","source or dest value is null.");
    }
    else{
        //download map
        if((mapsrcHEX[0]!=0x30)&&(mapsrcln!=0)&&(mapdesln!=0)&&(!bMapStarted)){
            TraceLog("nfc-main.c:bMapStarted:%d",bMapStarted);
            int res = show_map(mapsrcHEX,mapsrcln,mapdesHEX,mapdesln,mode,bMapStarted);//启动地图，关闭video
            if(res==0){
                bMapStarted = true;
            }
//            sleep(0.1);
            TraceLog("%s","show_map is end.");
        }

        //如果mode=1  swp card 时，向管理平台发送日志
        if((mapsrcHEX[0]!=0x30)&&(mode == 1)){
            char *iccid = load_user_info(pnd);
            byte_t iccidHEX[20];
            int iccidln=gsmString2Bytes(iccid,10,iccidHEX);
            if( iccidHEX[0]=='0'){
                goto theend;
            }
            char *reader_website;
            reader_website = (char *)malloc(sizeof(char)*256);
            memset(reader_website,0,sizeof(reader_website));
            strcat(reader_website,configInfo.serverIP);
            strcat(reader_website,"/NFC/TerminalAction?terminalId=");
            strcat(reader_website,configInfo.terminalId);
            strcat(reader_website,"&type=02&content=");
    //        strcat(reader_website,"http://192.168.1.101:8080/NFC/TerminalAction?terminalId=01&type=02&content=");
    //        strncat(reader_website,mapsrcHEX,mapsrcln);
    //        strncat(reader_website,mapdesHEX,mapdesln);
            strncat(reader_website,mapconHEX,mapconln);
            strcat(reader_website,"&iccid=");
            strncat(reader_website,iccidHEX,iccidln);
            int len_reader_website = strlen(reader_website);
            write(STDOUT_FILENO, reader_website, len_reader_website);
            TraceLog("%s",reader_website);

            WebTransUpload(reader_website);
            sleep(0.1);

            free(reader_website);
        }
        else if((mapsrc[0]==0x00)&&(mapsrc[1]==0x2a)&&(mapsrc[2]==0x00)&&(mapsrc[3]==0x23)){ //*#
            char *dual_website;
            char *tagNumber;
            tagNumber = (char *)malloc(sizeof(char)*32);
            memset(tagNumber,0,sizeof(tagNumber));
            sprintf(tagNumber,"%d",tagCount+1);
            dual_website = (char *)malloc(sizeof(char)*256);
            memset(dual_website,0,sizeof(dual_website));
            strcat(dual_website,configInfo.serverIP);
            strcat(dual_website,"/NFC/TerminalAction?terminalId=");
            strcat(dual_website,configInfo.terminalId);
            strcat(dual_website,"&type=03&content=");
            strcat(dual_website,tagNumber);
            TraceLog("%s",dual_website);
            dualInterfaceUpload(dual_website);

            pwm();
            TraceLog("%s","*#:pwm");

            free(tagNumber);
            free(dual_website);
        }
        else if((mode==0)&&(mapsrc[0]!=0x00)){  //daul
            char *dual_website;
            dual_website = (char *)malloc(sizeof(char)*256);
            memset(dual_website,0,sizeof(dual_website));
            strcat(dual_website,configInfo.serverIP);
            strcat(dual_website,"/NFC/TerminalAction?terminalId=");
            strcat(dual_website,configInfo.terminalId);
            strcat(dual_website,"&type=04&content=");
            strncat(dual_website,mapconHEX,mapconln);

            dualInterfaceUpload(dual_website);
            TraceLog("%s",dual_website);
            free(dual_website);
        }
    }

    theend:
//    free(randData_char);
//    free(plain);
//    free(plain_receive);
//    free(cipherResult_send);
    pn532_uart_disconnect (pnd);
}

void ChangeMapOn(int On){
    if(On==1){
        bMapOn=true;
    }
    else{
        bMapOn=false;
    }
}

void dualInterfaceUpload(char *website){

    CURL *curl_dualUpload;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_dualUpload = curl_easy_init();
    curl_easy_setopt(curl_dualUpload, CURLOPT_URL,website);
    curl_easy_perform(curl_dualUpload);
    curl_easy_cleanup(curl_dualUpload);
}

void WebTransUpload( char * webURL )
{

    CURL *curl_dualUpload;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_dualUpload = curl_easy_init();
    curl_easy_setopt(curl_dualUpload, CURLOPT_URL,webURL);
    curl_easy_perform(curl_dualUpload);
    curl_easy_cleanup(curl_dualUpload);
}

int nfc_start()
{
    bMapOn = false;
    if (!(pnddDevices = malloc(sizeof (*pnddDevices)))) {
        fprintf (stderr, "pnddDevices malloc() failed\n");
        exit (EXIT_FAILURE);
    }
    pnddDevices->pcDriver = configInfo.pnddDevices.pcDriver;
    pnddDevices->pcPort = configInfo.pnddDevices.pcPort;
    pnddDevices->uiSpeed = configInfo.pnddDevices.uiSpeed;
    pnddDevices->uiBusIndex = configInfo.pnddDevices.uiBusIndex;

    while(1){
        if(!bMapOn){
            TraceLog("%s","program in nfc_reader");
            nfc_reader(pnddDevices,3);
            sleep(1);
        }
        TraceLog("%s","program in nfc_card_emulator");
        nfc_card_emulator(pnddDevices);
        sleep(1);
    }
    free (pnddDevices);
    return 0;
}
