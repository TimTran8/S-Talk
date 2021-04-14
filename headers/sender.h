#ifndef _SENDER_
#define _SENDER_
#define MSG_MAX_LEN 512

void senderInit() ;
void senderClose();
void cancelSenderThreads();

#endif