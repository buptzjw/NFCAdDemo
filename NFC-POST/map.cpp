#include <QtGui/QApplication>
#include <QtGui/QGraphicsView>
#include <signal.h>
#include <sys/time.h>
#include "main.h"
#include "video.h"
#include "map.h"

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <QString>
#include "dynamic-load.h"
#include "image.h"
#include "nfc/nfc-types.h"

extern "C"
{
    #include "nfc-main.h"
    #include "chips/pn53x-internal.h"
    #include "chips/pn53x.h"
    #include "drivers.h"
    #include "nfc-internal.h"
    #include "nfc-utils.h"
    #include "nfc/nfc.h"
    #include "pn532_uart.h"
    #include "nfc/nfc-types.h"
    #include "log.h"
    #include "myconf.h"
    ConfigInfo configInfo;
    #include "pwm.h"
}

#define APDU_MAXLENGTH 254

int mapCount=0;
int mappid;
FILE *fp;

static nfc_device_t *pnd = NULL;
nfc_device_desc_t *pnddDevices_1 ;

//char *map_char; //下载地图图片的URL地址
//char *text_char;    //发送到手机端的数据

QString command;

class MapThread : public QThread
{
//     Q_OBJECT
public:
protected:
    void run();
};

void MapThread::run()
{
     startMap();
}

void stop_polling (int sig)
{
    (void) sig;
    if (pnd)
        nfc_abort_command (pnd);
    else
        exit (EXIT_FAILURE);
}



//启动线程进行地图展示
int showmap( char *source,int srclen, char *dest,int deslen,int mode,bool bMapStarted){
    if (!(pnddDevices_1 = (nfc_device_desc_t *)malloc(sizeof (*pnddDevices_1))))  {
        fprintf (stderr, "pnddDevices malloc() failed\n");
        exit (EXIT_FAILURE);
    }
    pnddDevices_1->pcDriver = configInfo.pnddDevices.pcDriver;
    pnddDevices_1->pcPort = configInfo.pnddDevices.pcPort;
    pnddDevices_1->uiSpeed = configInfo.pnddDevices.uiSpeed;
    pnddDevices_1->uiBusIndex = configInfo.pnddDevices.uiBusIndex;

    char *command_str;
    command_str = (char *)malloc(sizeof(char)*256);
    memset(command_str,0,sizeof(command_str));
    strcat(command_str,"/home/ubuntu/TagUpdate/map/map ");
    strncat(command_str,source,srclen);
    strcat(command_str," ");
    strncat(command_str,dest,deslen);
    strcat(command_str," ");



    //add by tl 20130826-1940
    if(mode==1){
        strcat(command_str,"1");
    }
    else{
        strcat(command_str,"0");
    }
    TraceLog("%s",command_str);
    printf("command_str : %s\n",command_str);
    printf("mode = %d\n",mode);
    command = QString(QLatin1String(command_str));

    //map thread
    if(source[0]!=0x30){//在获取广告信息时，不会进行地图查询;

        TraceLog("map.cpp:before bMapStarted:%d",bMapStarted);
        if(!bMapStarted){

            MapThread *thread_map=new MapThread;
            thread_map->start();
        }
        bMapStarted = true;
        TraceLog("map.cpp:after bMapStarted:%d",bMapStarted);
        sleep(20);
        /*
        FILE *fp_temp;
        long filesize = 0;
        do{
            sleep(1);
            fp_temp=fopen("/home/ubuntu/TagUpdate/map/map_ndef.txt","rb");
            if( fp_temp == NULL)
                continue;
            fseek( fp_temp,0,SEEK_SET);
            fseek( fp_temp,0,SEEK_END);
            filesize = ftell(fp_temp);
            fclose(fp_temp);
        }while (filesize < 4 );
        */

        //add by tl 20130808-1715
        //根据showmap函数传进来的参数，如果为swp则执行下面的程序,否则执行双界面卡的写入操作;
        if(mode == 1){
            //swp
            unsigned char *map_text;
            map_text = (unsigned char *)malloc(sizeof(unsigned char)*2560);
            int len=read_file("/home/ubuntu/TagUpdate/map/map_ndef.txt",map_text,2560);

            copyNdef( len,map_text);
            FILE *fp1=fopen("/home/ubuntu/TagUpdate/map/map_ndef.txt","w");
            if(fp1==NULL){
                TraceLog("%s","clear file error");
            }
            fclose(fp1);
            TraceLog("%s","clear map_ndef.txt");
            free(map_text);
        }
        else{
            //使用nfc_reader中的update指令来向卡中写入数据
            TraceLog("%s","begin write apdu data");

//            TraceLog("%s","pwmStart");
//            pwmStart();


            byte_t pbrData[256];
            bool    res;
            int times = 3;
            int i;

            signal (SIGINT, stop_polling);

            pnd = pn532_uart_connect (pnddDevices_1);

            if (pnd == NULL) {
                TraceLog("%s","Unable to connect to NFC device.");
                exit (EXIT_FAILURE);
            }
            TraceLog("%s",pnd->acName);
            //SAMConfiguration
            memset(pbrData,0,sizeof(pbrData));
            byte_t pbtData1[] = { 0x14, 0x01, 0x00, 0x00};
            res = pn532_uart_send(pnd,pbtData1,sizeof(pbtData1));
            if(!res){
                TraceLog("%s","SAMConfiguration pn532_uart_send ERROR!");
                goto theend;
            }
            int n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));

            //List Passive Target
            memset(pbrData,0,sizeof(pbrData));
            byte_t pbtData2[] = { 0x4A, 0x01, 0x00};
            res = pn532_uart_send(pnd,pbtData2,sizeof(pbtData2));
            if(!res){
                TraceLog("%s","List Passive Target pn532_uart_send ERROR!");
                goto theend;
            }

            n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
            int iv=0;
            while( n<=0 ){
                if(iv++ > times){
                    TraceLog("%s","List Target Error.");
                    return -1;
                }
                pn532_uart_send(pnd,pbtData2,sizeof(pbtData2));
                n = pn532_uart_receive (pnd,pbrData,sizeof(pbrData));
            }

            TraceLog("%s","List Passive Target result is ");
            TraceLog("%s",pbrData);

            if(pbrData[0] != 0x01){
                TraceLog("%s","List Passive Target pn532_uart_receive ERROR!");
                return -1;
            }


            TraceLog("%s","List Passive Target success.");

            //Select APP on mobile phone 1
            memset(pbrData,0,sizeof(pbrData));
            byte_t pbtData3[] = { 0x40, 0x01, 0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
            res = pn532_uart_send (pnd,pbtData3,sizeof(pbtData3));
            if(!res){
                TraceLog("%s","1:Select APP pn532_uart_send ERROR!");
                goto theend;
            }
            n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));

            TraceLog("%s","Select APP 1 result is ");
            TraceLog("%s",pbrData);

            if(n < 0){
                TraceLog("%s","1:Select APP pn532_uart_receive ERROR!");
                goto theend;
            }
            if(pbrData[0] != 0x00){
                TraceLog("%s","1:data ERROR!");
                goto theend;
            }
            //Select APP on mobile phone 2
            memset(pbrData,0,sizeof(pbrData));
            byte_t pbtData4[] = { 0x40, 0x01, 0x00, 0xA4, 0x00, 0x00, 0x02, 0xDF, 0x03};
            res = pn532_uart_send (pnd,pbtData4,sizeof(pbtData4));
            if(!res){
                TraceLog("%s","2:Select APP pn532_uart_send ERROR!");
                goto theend;
            }
            n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));

            TraceLog("%s","Select APP 2 result is ");
            TraceLog("%s",pbrData);

            if(n < 0){
                TraceLog("%s","2:Select APP pn532_uart_receive ERROR!");
                goto theend;
            }
            if(pbrData[0] != 0x00){
                TraceLog("%s","2:data ERROR!");
                goto theend;
            }
            //Select APP on mobile phone 3
            memset(pbrData,0,sizeof(pbrData));
            byte_t pbtData5[] = { 0x40, 0x01, 0x00, 0xA4, 0x00, 0x00, 0x02, 0x00, 0x18};
            res = pn532_uart_send (pnd,pbtData5,sizeof(pbtData5));
            if(!res){
                TraceLog("%s","3:Select APP pn532_uart_send ERROR!");
                goto theend;
            }
            n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));

            TraceLog("%s","Select APP 3 result is ");
            TraceLog("%s",pbrData);
            if(n < 0){
                TraceLog("%s","3:Select APP pn532_uart_receive ERROR!");
                goto theend;
            }
            if(pbrData[0] != 0x00){
                TraceLog("%s","3:data ERROR!");
                goto theend;
            }

            //Write data into dual interface card;
            TraceLog("%s","Write data into dual interface card");
            memset(pbrData,0,sizeof(pbrData));
            byte_t pbtData6_temp[254] = {0};

            int len = read_file("/home/ubuntu/TagUpdate/map/map_apdu.txt",pbtData6_temp,APDU_MAXLENGTH);

            TraceLog("%s","read file end");

            //apdu格式的数据一次最多能发256byte的数据
            byte_t pbtData6[256] = {0};
            pbtData6[0] = 0x40;
            pbtData6[1] = 0x01;

            memcpy(pbtData6+2,pbtData6_temp,len  );


            for(i=0;i<len;i++){
                pbtData6[i+2] = pbtData6_temp[i];
            }

            pn532_uart_send (pnd,pbtData6,sizeof(pbtData6));
            n = pn532_uart_receive(pnd,pbrData,sizeof(pbrData));

            TraceLog("%s","Write data into dual interface card result is ");
            TraceLog("%s",pbrData);

            if(n <= 0){
                TraceLog("%s","apdu:can't write apdu into dual interface card.");
                goto theend;
            }
            TraceLog("%s","apdu:write data success.");

            pwm();
            TraceLog("%s","map.cpp:pwm");

            if( pnddDevices_1 != NULL )
                free(pnddDevices_1);
            TraceLog("%s","free pnddDevices_1");
            return 0;
        }

    }
    theend:
    return 1;
}

//进行地图展示相关处理
void startMap(){
     ChangeMapOn(1);
     TraceLog("%s","map.cpp:map thread start");
     QProcess *map_process=new QProcess;
     map_process->setProcessChannelMode(QProcess::MergedChannels);
//     command ="/home/ubuntu/TagUpdate/map/map ";
     map_process->start(command);
     while(!map_process->waitForFinished()){
         sleep(0.1);
     }
     TraceLog("%s","map.cpp:map thread end");

     ChangeMapOn(0);
     show_map_end();
}



