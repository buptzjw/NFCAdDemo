#ifndef VIDEO_H
#define VIDEO_H

#endif // VIDEO_H
#include <QProcess>
class Video :public QObject
{
Q_OBJECT

//public:
//explicit Video(QObject *parent = 0);
//~Video();

public slots:      
    void startVideo(QString videoName);
//    void stopVideo();
//    void BeforeShowMapStop();
};
