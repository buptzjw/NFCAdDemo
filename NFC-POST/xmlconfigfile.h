#ifndef XMLCONFIGFILE_H
#define XMLCONFIGFILE_H

#include <QDomDocument>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QFile>
#include <QtGui/QApplication>
#include <QTextStream>
#include <QStringList>
#include <QChar>

extern "C"
{
    #include "myconf.h"
    extern ConfigInfo configInfo;
}
#define MAX_TAG_NUM 100

class XMLConfigFile
{
public:
    XMLConfigFile();

    int nADCount;

    int getTagCount() {return nADCount;};
    QString getVideoFileName( int i ) { return fileNameFromXML_video[i]; };
    QString getTagFileName( int i ) { return fileNameFromXML_tag[i]; };
    QByteArray getTagInfo( int i ) { return baTagInfo[i]; };

    QString fileNameFromXML_video[MAX_TAG_NUM];
    QString fileNameFromXML_tag[MAX_TAG_NUM];
    QByteArray baTagInfo[MAX_TAG_NUM];

    QString rootFilePath;//= "/home/ubuntu/TagUpdate/"

    void setRootFilePath( QString s){rootFilePath=s;};
    QString getRootFilePath(){ return rootFilePath; };

    bool ReadMainConfigFile( QString &filename );
    int loadXMLFile_tag(QString &NewxmlFilePath );
    bool loadBintag( int num  , QString  FileName );
    unsigned char fun(unsigned char ch );

    void dumpConfigFile();
};

#endif // XMLCONFIGFILE_H
