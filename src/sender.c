#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "list.h"
#include "setup.h"
#include "sender.h"
#include "receiver.h"

static pthread_t screenInThread;
static pthread_t networkOutThread;

static pthread_mutex_t sendListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t messageSendMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sendListCondition = PTHREAD_COND_INITIALIZER;
static pthread_cond_t messageSendCondition = PTHREAD_COND_INITIALIZER;

static void cancelNetworkOutThread();
static void cancelScreenInThread();

static List* sendList = NULL;
static char *pMessage = NULL;

static void* screenIn() 
{
	char message[MSG_MAX_LEN] = {'\0'};
	int readVal = 0;
	int terminateIdx = 0;
	char *pExclamation = NULL;
	pthread_testcancel();
	while(1) {
		memset(&message, '\0', sizeof(message)); // clear buffer
		readVal = read(0, message, sizeof(message));
		if (readVal == 0) {
			printf("Received EOF, only receiving messages now\n");
			// break;
			cancelScreenInThread();
		}
		else if (readVal < 0) {
			printf("Error reading\n");
			continue;
		}
		if ((pExclamation = strstr(message, "\n!\n"))) { // middle of file
			terminateIdx = pExclamation - message;
			terminateIdx += 2;
			message[terminateIdx] = '\0';
		} 
		else if (message[0] == '!' && message[1] == '\n') { // beginning of file
			terminateIdx++;
			message[terminateIdx] = '\0';
		}
		else {
			terminateIdx = (strlen(message) < MSG_MAX_LEN) ? strlen(message) : MSG_MAX_LEN - 1;
			message[terminateIdx] = '\0';
		}
		pthread_mutex_lock(&sendListMutex);
		{
			pMessage = (char*)malloc(sizeof(char)*MSG_MAX_LEN);
			memcpy(pMessage, message, sizeof(message));
			if (List_add(sendList, pMessage) < 0) { 
				printf("Couldn't add to list\n");
				printf("Suggestion: Exit manually\n");
				pthread_cond_wait(&messageSendCondition, &sendListMutex);
			}
			pthread_cond_signal(&sendListCondition);
		}
		pthread_mutex_unlock(&sendListMutex);
	}
	return NULL;
}

static void* networkOut() 
{
	char *msg = NULL;
	int msgLen = 0;
	pthread_testcancel();
	while(1) {
		pthread_mutex_lock(&sendListMutex);
		{
			pthread_cond_wait(&sendListCondition, &sendListMutex);
			while (List_count(sendList) != 0) {
				List_first(sendList);
				pMessage = List_remove(sendList);
				if (pMessage == NULL) {
					printf("removed null\n");
					pthread_mutex_unlock(&sendListMutex);
					free(pMessage);
					pMessage = NULL;
					continue;
				} else {
					msg = pMessage;
					msgLen = strlen(msg);
					int numBytes = sendto(socketDescriptor, msg, msgLen, 0, remoteResult->ai_addr, remoteResult->ai_addrlen);
					if (numBytes < 0) {
						printf("Error sending message\n");
					}
					if ((msg[0] == '!' && strlen(msg) == 2) || msg[strlen(msg)-1] == '!' || (msg[0] == '!' && msg[1] == '\n')) {
						printf("\n'!' detected: Exiting program\n");
						free(pMessage);
						pMessage = NULL;
						cancelReceiverThreads();
						cancelSenderThreads();
						break;
					}
					free(pMessage);
					pMessage = NULL;
					msg = NULL;
				}
			}
			pthread_cond_signal(&messageSendCondition);
		}
		pthread_mutex_unlock(&sendListMutex);
	}
	return NULL;
}

void senderInit() 
{
	sendList = List_create();
	if (pthread_create(&screenInThread, NULL, screenIn, NULL) != 0) {
		printf("Error creating screenIn thread\n");
		exit(1);
	}
	if (pthread_create(&networkOutThread, NULL, networkOut, NULL) != 0) {
		printf("Error creating networkOut thread\n");
		exit(1);
	}
}

static void cancelScreenInThread() 
{
	if (pthread_cancel(screenInThread) != 0) {
		printf("screenInThread cancelled with -1\n");
	}
	printf("Cancelled screenInThread\n");
}

static void cancelNetworkOutThread() 
{
	if (pthread_cancel(networkOutThread) != 0) {
		printf("networkOutThread cancelled with -1\n");
	}
	printf("Cancelled networkOutThread\n");
}

void cancelSenderThreads() 
{
	cancelScreenInThread();
	cancelNetworkOutThread();
}

static void complexTestFreeFn(void* pItem) 
{
	pItem = NULL; // free items in list
	printf("pItem: %p\n", pItem);
}

void senderClose() 
{
	if (pthread_join(screenInThread, NULL) != 0) {
		printf("Error joining to main thread\n");
		exit(1);
	}
	if (pthread_join(networkOutThread, NULL) != 0) {
		printf("Error joining to main thread\n");
		exit(1);
	}
	free(pMessage);
	pMessage = NULL;

	List_free(sendList, complexTestFreeFn);
	pthread_mutex_unlock(&sendListMutex);
	pthread_mutex_unlock(&messageSendMutex);
	
	// Cleanup memory
	pthread_mutex_destroy(&sendListMutex);
	pthread_mutex_destroy(&messageSendMutex);
	pthread_cond_destroy(&sendListCondition);
	pthread_cond_destroy(&messageSendCondition);
}