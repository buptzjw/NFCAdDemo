#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <QString>
#include "video.h"

QProcess *video_process =new QProcess;

//启动播放video进程
void Video::startVideo(QString fileName_video){

    /*
    QString fileName_video="/home/ubuntu/TagUpdate/";
    fileName_video.append(videoName);*/


    QString command ="mplayer "  +fileName_video +  "  -slave -quiet -softvol ";

    video_process->setProcessChannelMode(QProcess::MergedChannels);

    video_process->start(command);
    while(!video_process->waitForFinished()){
//        sleep(0.1);
    }
}


