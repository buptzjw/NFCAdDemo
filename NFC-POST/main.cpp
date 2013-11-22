#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "video.h"
#include "map.h"
#include "main.h"
#include <QThread>
#include "dynamic-load.h"
#include "log.h"

#include "nfcthread.h"
#include "xmlconfigfile.h"

//#include "nfc-types.h"

#include <QDomDocument>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QFile>
#include <QtGui/QApplication>
#include <QDebug>

extern "C"
{
    #include "nfc-main.h"
    #include "ndeffile.h"
    #include "myconf.h"
    extern ConfigInfo configInfo;
    //    #include "nfc-types.h"
}

#define MAXLEN 10000
#define MAX_TAG_NUM 100

QString fileNameFromXML_video[MAX_TAG_NUM];
QString fileNameFromXML_tag[MAX_TAG_NUM];

bool isVideoOn;

XMLConfigFile g_ConfFile;

//封装NDEF信息
void copyNdef(int len,unsigned char *indata){
    int i;
    ndef_file[0]=(char)(len>>8);
    ndef_file[1]=(char)len;
    for(i=0;i<len;i++){
        ndef_file[i+2]=indata[i];
    }
}

//map展示完，启动video
 void show_map_end(){
    isVideoOn=true;
}

 //停止video,启动map
 int show_map( char *source,int srclen, char *dest,int deslen,int mode,bool bMapStarted){
    isVideoOn=false;
    if(!bMapStarted){
        int res = showmap(source,srclen,dest,deslen,mode,bMapStarted);
//        TraceLog("/home/ubuntu/TagUpdate/log/log.txt","showmap is end.");
        if(res==0){
            return 0;
        }
    }
    else
        return 1;
}



int main(){
    QString rootFilePath = "/home/buptzjw/test/srcUpdate/";

    //读取XML配置文件，包括PN532配置，terminalId，服务器端IP地址，查询时延
    QString sConfigFileName =  rootFilePath + "config.xml";

    g_ConfFile.setRootFilePath(rootFilePath);



    if ( !g_ConfFile.ReadMainConfigFile(sConfigFileName )){
        printf("open config.xml failed!! no file or error ");
        return -1;
    }

    //读取配置文件,返回tag数量
    QString infoFilePath =  rootFilePath + "info.xml";
    int TAG_NUM= g_ConfFile.loadXMLFile_tag( infoFilePath );
    if(TAG_NUM<=0){
        printf( "info.xml:请检查配置文件！");
        exit(0);
    }
    g_ConfFile.dumpConfigFile();


    NFCThread *thread_nfc=new NFCThread;
    thread_nfc->start();


    //start_video_main(TAG_NUM);
    isVideoOn=true;
    Video video;

    while(true){
        for( int i=0;i<TAG_NUM;i++){
            if(isVideoOn){
                QByteArray tag_ba = g_ConfFile.getTagInfo(i);
                unsigned char * text = (unsigned char *)tag_ba.data();
                int len = tag_ba.length();
                ndef_file[0]=(char)(len>>8);
                ndef_file[1]=(char)len;
                memcpy(ndef_file+2,text,len );
            }
            video.startVideo( rootFilePath + g_ConfFile.getVideoFileName(i));
        }
    }

}


