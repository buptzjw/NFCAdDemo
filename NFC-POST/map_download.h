#ifndef MAP_DOWNLOAD_H
#define MAP_DOWNLOAD_H

#include <curl/curl.h>
#include <curl/easy.h>
static CURLcode map_download(char *filename,char *url);
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

#endif // MAP_DOWNLOAD_H



