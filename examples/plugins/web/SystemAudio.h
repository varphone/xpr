#ifndef _WEB_AUDIO_H_
#define _WEB_AUDIO_H_

#include "Config.h"

void SystemAudioInit(void);

int GET_PSIA_System_Audio_Channels(struct mg_connection* conn,
                                   const struct mg_request_info* request_info);

int PUT_PSIA_System_Audio_Channels(struct mg_connection* conn,
                                      const struct mg_request_info* request_info, XML_Parser p);
#endif /* _WEB_AUDIO_H_ */
