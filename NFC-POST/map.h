#ifndef MAP_H
#define MAP_H
#include <QThread>
#include "nfc/nfc-types.h"

void show_map_end();
int showmap( char *source,int srclen, char *dest,int deslen,int mode,bool bMapStarted);
int start_map();
void stop_map();
int xml_analysis(char *filename);
void startMap();


//void ChangeMapOn(bool On);

#endif


