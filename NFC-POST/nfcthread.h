#ifndef NFCTHREAD_H
#define NFCTHREAD_H

#include <QThread>

class NFCThread : public QThread
{
public:
    NFCThread();

    void run();
};

#endif // NFCTHREAD_H
