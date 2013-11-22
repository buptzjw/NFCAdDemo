#include "xmlconfigfile.h"
#include <QDebug>

XMLConfigFile::XMLConfigFile()
{
}


bool XMLConfigFile::ReadMainConfigFile( QString &filename )
{
    //读取XML配置文件，包括PN532配置，terminalId，服务器端IP地址，查询时延
    QFile configFile( filename );
    if(!configFile.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }
    QDomDocument domDocument;
    if (!domDocument.setContent(&configFile, true)) {
        configFile.close();
        printf("setContent Error!\n");
        return false;
    }
    QDomElement root = domDocument.documentElement();
    QDomNodeList firstItemlist = root.childNodes();
    QDomElement firstItem = root.firstChildElement();

    QDomNodeList secondItemlist = firstItem.childNodes();
    QDomNode node0 = secondItemlist.at(0);
    QDomNode node1 = secondItemlist.at(1);
    QDomNode node2 = secondItemlist.at(2);
    QDomNode node3 = secondItemlist.at(3);
    QString pcDriver_qstr = node0.toElement().text();
    QString pcPort_qstr = node1.toElement().text();
    QString uiSpeed_qstr = node2.toElement().text();
    QString uiBusIndex_qstr = node3.toElement().text();

    QByteArray pcDriver_ba = pcDriver_qstr.toLatin1();
    QByteArray pcPort_ba = pcPort_qstr.toLatin1();
    char *pcDriver = pcDriver_ba.data();
    char *pcPort = pcPort_ba.data();
    int uiSpeed = uiSpeed_qstr.toInt();
    int uiBusIndex = uiBusIndex_qstr.toInt();

    QDomNode terIdNode = firstItemlist.at(1);
    QString terId_qstr = terIdNode.toElement().text();
    QByteArray terId_ba = terId_qstr.toLatin1();
    char *terId = terId_ba.data();

    QDomNode serverIPNode = firstItemlist.at(2);
    QString serverIP_qstr = serverIPNode.toElement().text();
    QByteArray serverIP_ba = serverIP_qstr.toLatin1();
    char *serverIP = serverIP_ba.data();

    QDomNode delayNode = firstItemlist.at(3);
    QString delay_qstr = delayNode.toElement().text();
    QByteArray delay_ba = delay_qstr.toLatin1();
    char *delay = delay_ba.data();

    QDomNode wNode = firstItemlist.at(4);
    QString w_qstr = wNode.toElement().text();
    QByteArray w_ba = w_qstr.toLatin1();
    char *width = w_ba.data();

    QDomNode hNode = firstItemlist.at(5);
    QString h_qstr = hNode.toElement().text();
    QByteArray h_ba = h_qstr.toLatin1();
    char *height = h_ba.data();

    int t_len = strlen(delay)+1;
    configInfo.delay =  (char*)malloc( t_len );
    memcpy(configInfo.delay,delay,strlen(delay) );
    configInfo.delay[t_len] = 0x00;

    t_len = strlen(serverIP)+1;
    configInfo.serverIP =   (char*)malloc(t_len  );
    memcpy(configInfo.serverIP,serverIP,strlen(serverIP) );
    configInfo.serverIP[t_len] = 0x00;

    t_len = strlen(terId)+1;
    configInfo.terminalId =  (char*)malloc( t_len );
    memcpy(configInfo.terminalId,terId,strlen(terId) );
    configInfo.terminalId[t_len]  = 0x00;

    t_len = strlen(pcDriver)+1;
    configInfo.pnddDevices.pcDriver = (char*)malloc( t_len );
    memcpy(configInfo.pnddDevices.pcDriver,pcDriver,strlen(pcDriver) );
    configInfo.pnddDevices.pcDriver[t_len] = 0;

    t_len = strlen(pcPort)+1;
    configInfo.pnddDevices.pcPort =  (char*)malloc( t_len);
    memcpy(configInfo.pnddDevices.pcPort,pcPort,strlen(pcPort) );
    configInfo.pnddDevices.pcPort[t_len] = 0;

    configInfo.pnddDevices.uiBusIndex =  uiBusIndex;

    configInfo.pnddDevices.uiSpeed =   uiSpeed;

    t_len = strlen(width)+1;
    configInfo.width =  (char*)malloc( t_len );
    memcpy(configInfo.width,width,strlen(width) );
    configInfo.width[t_len] =0;

     t_len = strlen(height)+1;
    configInfo.height =  (char*)malloc( t_len );
    memcpy(configInfo.height,height,strlen(height) );
     configInfo.height[t_len] =0;


    write(1,configInfo.terminalId,2);
    write(1,configInfo.pnddDevices.pcDriver,strlen(configInfo.pnddDevices.pcDriver));

    configFile.close();

    return true;

}


//读取广告的tag信息
int XMLConfigFile::loadXMLFile_tag( QString & NewxmlFilePath )
{
     QFile New_file(NewxmlFilePath);

    if(!New_file.open(QFile::ReadOnly | QFile::Text)) {
        return -1;
    }

    QDomDocument domDocument;
    if (!domDocument.setContent(&New_file, true)) {
        New_file.close();
        printf("setContent Error!\n");
        return -1;
    }

    QDomElement root = domDocument.documentElement();
    //firstItem : tag1,tag2,tag3
    QDomElement firstItem = root.firstChildElement();
    int i = 0;
    while(!firstItem.isNull()){
            QDomNodeList secondItemlist = firstItem.childNodes();
            QDomNode node0 = secondItemlist.at(0);
            QDomNode node1 = secondItemlist.at(1);
            fileNameFromXML_tag[i]=node0.toElement().text();

            if( !loadBintag( i, rootFilePath+fileNameFromXML_tag[i]) ){
                printf( "the tag %d is ERROR!!\n", i );
                return -1;
            }

            fileNameFromXML_video[i]=node1.toElement().text();
            firstItem = firstItem.nextSiblingElement();
            i++;
    }
    New_file.close();
    nADCount = i;
    return nADCount;
}

//读取广告的tag信息
bool XMLConfigFile::loadBintag( int num , QString FileName )
{
    QFile New_file(FileName);

    if(!New_file.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

    QTextStream stream( &New_file );
    //QStringList lines;
    QString line;
    while ( !stream.atEnd() ) {
        line += stream.readLine(); // 不包括“\n”的一行文本
        //qDebug()<< line;
    }
    New_file.close();
    unsigned char chH , chL , chDest;
    baTagInfo[num].clear();

    for(int i = 0; i < line.length(); i+=2)
    {
        chH = line.at(i).toLatin1();
        chL = line.at(i+1).toLatin1();
        chDest = (fun( chH ) << 4) |fun( chL );

        baTagInfo[num].append(chDest);
    }

    return true;
}



//把用字符表示的十六进制数转换为二进制表示
unsigned char XMLConfigFile::fun(unsigned char c)
{
    if('A' <=c && c <='F')
                return c - 'A' + 10;
     if('a' <=c && c <='f')
                return c - 'a' + 10;

    return c - '0' ;
}

void  XMLConfigFile::dumpConfigFile()
{
    qDebug()<< "Dump Config File\n";
    for( int i=0; i< getTagCount(); i++)
    {
        qDebug()<< i ;
        qDebug()<< getVideoFileName(i);
        qDebug()<< getTagFileName(i);
        qDebug()<< getTagInfo(i);
    }

}
