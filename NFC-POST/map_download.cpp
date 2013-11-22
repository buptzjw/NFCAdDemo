#include <QtCore/QCoreApplication>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <curl/curl.h>
#include <curl/easy.h>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QString>
#include <QDomDocument>
#include <QDomDocument>
#include <QXmlStreamReader>

#include "map_download.h"

FILE *fp;

size_t process_data(void *buffer, size_t size, size_t nmemb, void *user_p){
    FILE *fp = (FILE *)user_p;
    size_t return_size = fwrite(buffer, size, nmemb, fp);
    return return_size;
}

static CURLcode map_download(char *filename,char *url)
{
    CURLcode return_code;
    return_code = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != return_code){
        printf("init libcurl failed.");
        return (CURLcode)-1;
    }
    // 获取easy handle
    CURL *easy_handle = curl_easy_init();
    if (NULL == easy_handle)   {
        printf( "get a easy handle failed.") ;
        curl_global_cleanup();
        return (CURLcode)-1;
    }

    FILE *fp = fopen(filename, "wb");
    // 设置easy handle属性
    curl_easy_setopt(easy_handle,CURLOPT_URL, url);
    curl_easy_setopt(easy_handle,CURLOPT_WRITEFUNCTION, &process_data);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);
    //curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT_MS, 3000);// Set timeout by ms
    // 执行数据请求
    return_code=curl_easy_perform(easy_handle);
    // 释放资源
    fclose(fp);
    curl_easy_cleanup(easy_handle);
    curl_global_cleanup();
    return return_code;
}
/*
int main()
{
    CURLcode nRet;

    int i=0;
    nRet=map_download("/home/ubuntu/QtDownload/map.jpg","http://map.hytrip.net/photo/350/8010753.jpg");
    if (CURLE_OK != nRet){
        printf("Failed!");
    }
    printf("Success!\n");

    return 0;
}
*/
