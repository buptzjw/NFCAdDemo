#include "nfcthread.h"

extern "C"
{
    #include "nfc-main.h"
}

NFCThread::NFCThread()
{
}

//启动nfc线程
void NFCThread::run()
{
     nfc_start();
}
