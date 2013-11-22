#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif // HAVE_CONFIG_H

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

#include "rsa.h"
#include "chips/pn53x-internal.h"
#include "chips/pn53x.h"
#include "drivers.h"
#include "nfc-internal.h"
#include "nfc-utils.h"
#include "nfc/nfc.h"
#include "pn532_uart.h"

#include "nfc-ndef.h"
#include "nfc-emulation.h"

#include "nfc_emulate_card.h"

#include "nfc-emulate-forum-tag4.c"

#define SYMBOL_PARAM_fISO14443_4_PICC   0x20
/* C-ADPU offsets */
#define CLA  0
#define INS  1
#define P1   2
#define P2   3
#define LC   4
#define DATA 5
#define ISO144434A_RATS 0xE0
static char msg1[] = "nfc_reader thread is on! \n";
static char msg2[] = "nfc_emulator is on! \n";
static int len1,len2;
int number = 0;

size_t  szFound;
size_t  i,j,k,l;

size_t  sourceLength,destLength;
bool verbose = false;
bool    res;
byte_t pbrData[256];

char *plain;

byte_t cipherDataWithFrame[256];
byte_t cipherDataWithoutFrame[128];
byte_t cipherDataWithoutFrame_reverse[128];
byte_t QueryDataWithFrame[256];
byte_t QueryDataWithoutFrame[256];

unsigned char source[128];
unsigned char dest[128];
char rawtext[250] = "1234";
char *g_pubN = "AA030736A1480CC78576531EDF2D1153C18C22F1D0CAA5DBC7B52E3183B6BCC24F8853F7621F6E9473827DC32789E12F2D324BAE4C53A26F71ECF77C78914E107CC76B25507946F68A5D0FF9BD9D1EBB4234B2482E3824C2D3F55EA1D108DA2CF7F1B48D23FF7508DDCD4824E5E1D7C5B74FFB4A7F3492B3E0E3EEF3EA7B76F1";
char *g_pubKey = "10001";
char *g_priKey = "5F074248EEED6EF7374A643BA5537393F5637744E307E29D6E8B7F3E7DF8EE994E0B4DA3C15D005A4A166E655EFC5ABA1BF1390C3BEDA3A5D770350585FEF23AABCA9774C98AD60015926D4D843C82F52E52923A41F2387F7A54CBAD6416543549FED54ACE2D21F5B6B9B593549E57FEE48AA4FAFDF4C12B47EA34B4574DFAE1";

static nfc_device_t *pnd = NULL;

bool bEmulate = true;

void stop_polling (int sig)
{
    (void) sig;
    if (pnd)
        nfc_abort_command (pnd);
    else
        exit (EXIT_FAILURE);
}

int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int i;
    for (i = 0; i < nSrcLength; i += 2){
    // 输出高4位
        if ((*pSrc >= '0') && (*pSrc <= '9')){
            *pDst = (*pSrc - '0') << 4;
        }
        else{
            *pDst = (*pSrc - 'A' + 10) << 4;
        }
        pSrc++;
        // 输出低4位
        if ((*pSrc>='0') && (*pSrc<='9')){
            *pDst |= *pSrc - '0';
        }
        else{
            *pDst |= *pSrc - 'A' + 10;
        }
        pSrc++;
        pDst++;
    }
    // 返回目标数据长度
    return (nSrcLength / 2);
}

void nfc_reader(nfc_device_desc_t *pnddDevices, int times){
    //  进行NFC设备的连接
    size_t  szFound;
    size_t  i,j,k,l;

    size_t  sourceLength,destLength;
    bool verbose = false;
    bool    res;
    byte_t pbrData[256];

    int randData_int = 0;
    char *randData_char;
    char *rand_plain;
    randData_char = (char *)malloc(sizeof(char)*4);
    rand_plain = (char *)malloc(sizeof(char)*4);

    char *plain_receive;
    plain_receive = (char *)malloc(sizeof(char)*4);

    char *cipherResult_send_char;
    cipherResult_send_char = (char*)malloc(sizeof(char)*129);

    unsigned char *cipherResult_send_hex;
    cipherResult_send_hex = (unsigned char*)malloc(sizeof(char)*129);

    byte_t cipherDataWithFrame_receive[256];
    byte_t cipherDataWithoutFrame_receive[128];
    byte_t cipherDataWithoutFrame[128];
    byte_t QueryDataWithFrame[256];
    byte_t QueryDataWithoutFrame[256];

    unsigned char source[128];
    unsigned char dest[128];
    char rawtext[250] = "1234";
    //智能信息终端密钥对
    char *g_pubN = "AA030736A1480CC78576531EDF2D1153C18C22F1D0CAA5DBC7B52E3183B6BCC24F8853F7621F6E9473827DC32789E12F2D324BAE4C53A26F71ECF77C78914E107CC76B25507946F68A5D0FF9BD9D1EBB4234B2482E3824C2D3F55EA1D108DA2CF7F1B48D23FF7508DDCD4824E5E1D7C5B74FFB4A7F3492B3E0E3EEF3EA7B76F1";
    char *g_pubKey = "10001";
    char *g_priKey = "5F074248EEED6EF7374A643BA5537393F5637744E307E29D6E8B7F3E7DF8EE994E0B4DA3C15D005A4A166E655EFC5ABA1BF1390C3BEDA3A5D770350585FEF23AABCA9774C98AD60015926D4D843C82F52E52923A41F2387F7A54CBAD6416543549FED54ACE2D21F5B6B9B593549E57FEE48AA4FAFDF4C12B47EA34B4574DFAE1";
    //卡端密钥对
    char *c_pubN = "";
    char *c_pubKey = "";
    char *c_priKey = "";

    signal (SIGINT, stop_polling);

    pnd = pn532_uart_connect (pnddDevices);

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
    }
    int n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
    printf("receive %d\n",n);
//    printf("SAMConfiguration success !\n");
    //Handle Error
//    if(pbrData[0] != 0){
//        printf("SAMConfiguration pn532_uart_receive ERROR!");
//    }

    //List Passive Target
    memset(pbrData,0,sizeof(pbrData));
    byte_t pbtData2[] = { 0x4A, 0x01, 0x00};
    res = pn532_uart_send(pnd,pbtData2,sizeof(pbtData2));
    if(!res){
        printf ("List Passive Target pn532_uart_send ERROR!\n");
    }

    n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
    //printf("%d",n);
    int iv=0;
    while( n<=0 ){
        if(iv++ > times){
            goto theend;
        }
        pn532_uart_send(pnd,pbtData2,sizeof(pbtData2));
        n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
    }

    if(pbrData[0] != 0x01){
        printf("List Passive Target pn532_uart_receive ERROR!\n");
    }
//    printf("receive %d\n",n);
    printf("List Passive Target success ! \n");

    //Select APP on mobile phone
    memset(pbrData,0,sizeof(pbrData));
    //byte_t pbtData3[] = { 0x40, 0x01, 0x00, 0xA4, 0x04, 0x00, 0x0A, 0x62, 0x75, 0x70, 0x74, 0x2E, 0x64, 0x65,0x6d, 0x6f, 0x2e};
    byte_t pbtData3[] = { 0x40, 0x01, 0x00, 0xA4, 0x04, 0x00, 0x0D, 0x62, 0x75, 0x70, 0x74, 0x2E, 0x6E, 0x65, 0x77, 0x70, 0x75, 0x73,0x68,0x2e};
    res = pn532_uart_send (pnd,pbtData3,sizeof(pbtData3));
    if(!res){
        printf ("Select APP pn532_uart_send ERROR!");
    }
    //printf("%d",res);
    n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));
    if(n <= 0){
        printf ("Select APP pn532_uart_receive ERROR!");
    }


    if(pbrData[0] != 0x00){
        printf("Select APP Status Error! \n");
    }
    if((pbrData[1] == 0x90)&&(pbrData[2] == 0x00)){
        printf("receive %d\n",n);
        printf("Select APP success \n");

    }

    /*身份认证*/
    byte_t pbtData4[135];
    memset(pbtData4,0,sizeof(pbtData4));
    pbtData4[0] = 0x40;
    pbtData4[1] = 0x01;
    pbtData4[2] = 0x00;
    pbtData4[3] = 0x72;
    pbtData4[4] = 0x00;
    pbtData4[5] = 0x00;
    pbtData4[6] = 0x80;

    //random()
    srand((unsigned)time(NULL));

    for(i=0; i<4; i++){
        randData_int = rand()%255;
        sprintf(randData_char,"%x",randData_int);
        strcat(rand_plain,randData_char);
    }
    printf("the random plain is %s",rand_plain);

    //encrypt()
    cipherResult_send_char = rsa_encrypt(g_pubN,g_priKey,rand_plain);
    printf("cipherResult_send_char length : %d\n",strlen(cipherResult_send_char));
    printf("cipherResult_send_char : %s\n",cipherResult_send_char);

    //send the cipher data to card
    gsmString2Bytes(cipherResult_send_char,cipherResult_send_hex,256);
    for(i=0;i<128;i++){
       pbtData4[i+7] = cipherResult_send_hex[i];
    }
    printf("Identication send command with frame is : \n");
    for(i=0;i<135;i++){
        printf("%x ",pbtData4[i]);
    }

//    byte_t pbtData4[] = { 0x40, 0x01, 0x00, 0x72, 0x00, 0x00, 0x04, 0x31, 0x32, 0x33, 0x34};
//    byte_t pbtData4[] = { 0x40, 0x01, 0x00, 0x72, 0x00, 0x00, 0x04};

    pn532_uart_send (pnd,pbtData4,sizeof(pbtData4));

    //wait for seconds to receive card data
    /* code */
    memset(cipherDataWithFrame_receive,0,sizeof(cipherDataWithFrame_receive));
    n = pn532_uart_receive(pnd,cipherDataWithFrame_receive,sizeof(cipherDataWithFrame_receive));

    for(j = 0;j < 128;j++){
        cipherDataWithoutFrame_receive[j] = cipherDataWithFrame_receive[j+2];
    }

    //接收数据与应接收的数据顺序正好相反,将两数组结果颠倒.
    for(k = 0;k < 128;k++){
        cipherDataWithoutFrame[127-k] = cipherDataWithoutFrame_receive[k];
    }
    printf("received cipher data is :%s\n",cipherDataWithoutFrame);

    //decrypt()
    plain_receive = rsa_decrypt(c_pubN,c_pubKey,cipherDataWithoutFrame);
    printf("the decrypt result is :%s\n",plain_receive);
    if(strcmp(plain_receive,randData_char) != 0){
        printf("Identity Authentication Error !\n");
        return -1;
    }
    printf("Identity Authentication success!\n");

/*
    UINT_to_HEX(cipherDataWithoutFrame_reverse,128,cipherData);

    printf("cipherDataWithoutFrameleng:%d\n",sizeof(cipherDataWithoutFrame_receive));
    printf("cipherData:%s\n",cipherData);
    printf("cipherDataleng:%d\n",strlen(cipherData));
    //void ChangetoHex(unsigned char *strRec, int nLen,unsigned char *cipherData)

    printf("***\n");
    printf("the challenge data is :%s\n",rawtext);
    printf("***\n");

    printf("%s\n",cipherData);
    printf("***\n");

    printf("the decrypt result is :%s\n",plain_receive);
    if(strcmp(plain_receive,"1234") != 0){
        printf("Identity Authentication Error !\n");
        return -1;
    }
    else{
        printf("Identity Authentication success!\n");
    }
*/

    //Map search
    memset(QueryDataWithFrame,0,sizeof(QueryDataWithFrame));
    memset(QueryDataWithoutFrame,0,sizeof(QueryDataWithoutFrame));
    memset(source,0,sizeof(source));
    memset(dest,0,sizeof(dest));

    byte_t pbtData5[] = { 0x40, 0x01, 0x00, 0x76, 0x00, 0x00};
    //byte_t pbtData3[] = { 0x40, 0x01, 0x00, 0xA4, 0x04, 0x00, 0x0A, 0x62, 0x75, 0x70, 0x74, 0x2E, 0x64, 0x65,0x6d, 0x6f, 0x2e};
    pn532_uart_send (pnd,pbtData5,sizeof(pbtData5));
    n = pn532_uart_receive(pnd,QueryDataWithFrame,sizeof(QueryDataWithFrame));
    printf("QueryDataWithFrame length:%d \n",n);
    if(QueryDataWithFrame[0] != 0x00){
        printf("QueryData Error! \n");
        return -1;
    }

    for(l = 0;l < n-3;l++){
       QueryDataWithoutFrame[l] = QueryDataWithFrame[l+1];

    }

    sourceLength = QueryDataWithoutFrame[0];
    destLength = QueryDataWithoutFrame[sourceLength+1];
    for(l=1,k=0;l < QueryDataWithoutFrame[0]+1;l++,k++){
        source[k] = QueryDataWithoutFrame[l];
        if(source[k] > 0xff){
            source[k] = 0xffffffff-source[k];
        }
    }

    for(l=sourceLength+2,k=0;l < 2+sourceLength+destLength;l++,k++){
        dest[k] = QueryDataWithoutFrame[l];
        if(dest[k] > 0xff){
            dest[k] = 0xffffffff-dest[k];
        }
    }
    printf("The source is :");
    for(i=0;i<sourceLength;i++){
        printf("0x%x ",source[i]);
    }
    printf("\n");
    printf("The destination is :");
    for(i=0;i<destLength;i++){
        printf("0x%x ",dest[i]);
    }

//    return 0;

theend:

//    pn532_uart_disconnect(pnd);

//    free (pnddDevices);
    return 0;
}

void print1(){
    write(STDOUT_FILENO, msg1, len1);
}

void print2(){
    write(STDOUT_FILENO, msg2, len2);
}

//// timer设定的时间到了,进行相应的处理
//void prompt_info_thr(int signo)
//{
//    if(bEmulate)
//        stop_emulation(signo);
//}

//// 建立信号处理机制
//void init_sigaction(void)
//{
//    struct sigaction tact;
//    //信号到了要执行的任务处理函数为prompt_info
//    tact.sa_handler = prompt_info_thr;
//    tact.sa_flags = 0;
//    //初始化信号集
//    sigemptyset(&tact.sa_mask);
//    //建立信号处理机制
//    sigaction(SIGALRM, &tact, NULL);
//}

//void init_time()
//{
//    struct itimerval value;
//    //设定执行任务的时间间隔为2秒0微秒
//    value.it_value.tv_sec = 5;
//    value.it_value.tv_usec = 0;
//    //设定初始时间计数也为2秒0微秒
//    value.it_interval = value.it_value;
//    //设置计时器ITIMER_REAL
//    setitimer(ITIMER_REAL, &value, NULL);
//}

int main()
{
    const char *acLibnfcVersion = "libnfc-1.5.0";

    nfc_device_desc_t *pnddDevices ;

    if (!(pnddDevices = malloc(sizeof (*pnddDevices)))) {
        fprintf (stderr, "malloc() failed\n");
        exit (EXIT_FAILURE);
    }
//    strcpy( pnddDevices->acDevice , "ACS ACR 38U-CCID 00 00");
    pnddDevices->pcDriver = "PN532_UART";
    pnddDevices->pcPort = "/dev/ttyUSB0";
    pnddDevices->uiSpeed = 115200;
    pnddDevices->uiBusIndex = 115200;

//    init_sigaction();
//    init_time();



    while(1){
        bEmulate = true;
        nfc_card_emulator(pnddDevices);

        bEmulate = false;
        nfc_reader(pnddDevices,3);
    }

    free (pnddDevices);
    return 0;
}
