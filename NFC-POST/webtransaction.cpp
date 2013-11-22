#include "webtransaction.h"

extern "C"
{
    #include <curl/curl.h>
    #include <curl/easy.h>
}

WebTransaction::WebTransaction()
{
}


void WebTransaction::TransactionUpload(char *weburl){

    CURL *curl_dualUpload;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_dualUpload = curl_easy_init();
    curl_easy_setopt(curl_dualUpload, CURLOPT_URL , weburl);
    curl_easy_perform(curl_dualUpload);
    curl_easy_cleanup(curl_dualUpload);
}
