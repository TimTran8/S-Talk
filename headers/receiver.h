#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#define MSG_MAX_LEN 512

void receiverInit();
void receiverClose();
void cancelReceiverThreads();

#endif