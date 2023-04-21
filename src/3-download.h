#ifndef __DOWNLOAD_H__
#define __DOWNLOAD_H__
#include "global.h"
#include "SPIFFS.h"

class Download {
	public:
    bool download(Stream *port);
};
#endif