#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
// #include "list.h"
// #include "setup.h"
// #include "sender.h"
// #include "receiver.h"
#include "../headers/receiver.h"
#include "../headers/list.h"
#include "../headers/sender.h"
#include "../headers/setup.h"

static pthread_t networkInThread;
static pthread_t screenOutThread;

static pthread_mutex_t receiveListMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t receiveListCondition = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t receiveMessageMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t receiveMessageCondition = PTHREAD_COND_INITIALIZER;

static List* receiveList = NULL;
static char *pMessage = NULL;

static void* networkIn() 
{
	char messageRx[MSG_MAX_LEN] = {0};
	pthread_testcancel();
	while(1) {
		memset(messageRx, '\0', sizeof(messageRx)); // clear buffer
		int bytesRx = recvfrom(socketDescriptor, messageRx, MSG_MAX_LEN, 0, remoteResult->ai_addr, &remoteResult->ai_addrlen);
		if (bytesRx < 0) {
			perror("recvfrom error\n");
			exit(0);
			continue;
		} else {
			int terminateIdx = (bytesRx < MSG_MAX_LEN) ? bytesRx : MSG_MAX_LEN - 1;
			messageRx[terminateIdx] = 0;
		}
		pthread_mutex_lock(&receiveListMutex);
		{
			pMessage = (char*)malloc(sizeof(char) * MSG_MAX_LEN);
			memcpy(pMessage, messageRx, sizeof(messageRx));
			if (List_append(receiveList, pMessage) < 0) {
				printf("Could not add to list\n");
				printf("Suggestion: Exit manually\n");
				pthread_cond_wait(&receiveMessageCondition, &receiveListMutex);
			}
			pthread_cond_signal(&receiveListCondition);
		}
		pthread_mutex_unlock(&receiveListMutex);
		fflush(stdin);
	}	
	return NULL;
}

static void* screenOut()
{
	char* message = NULL;
	pthread_testcancel();
	while(1) {
		pthread_mutex_lock(&receiveListMutex);
		{
			pthread_cond_wait(&receiveListCondition, &receiveListMutex);
			printf("\nReceived:\n");
			while(List_count(receiveList) != 0) {
				List_first(receiveList);
				pMessage = List_remove(receiveList);
				if (pMessage == NULL) {
					printf("removed null\n");
					pthread_mutex_unlock(&receiveListMutex);
					free(pMessage);
					pMessage = NULL;
					continue;
				} else {
					message = pMessage;
					write(STDOUT_FILENO, message, strlen(message));
					fflush(stdout);
					if ((message[0] == '!' && strlen(message) == 2) || message[strlen(message)-1] == '!') {
						printf("\nscreenOut: Exiting program\n");
						printf("\n'!' detected: Exiting program\n");
						free(pMessage);
						pMessage = NULL;
						cancelSenderThreads();
						cancelReceiverThreads();
					}
					free(pMessage);
					pMessage = NULL;
					message = NULL;
				}
			}
			pthread_cond_signal(&receiveMessageCondition);
		}
		pthread_mutex_unlock(&receiveListMutex);
		fflush(stdout);
	}
	return NULL;
}

void receiverInit() 
{
	receiveList = List_create();
	if (pthread_create(&networkInThread, NULL, networkIn, NULL) != 0) {
		printf("Error creating networkIn thread\n");
		exit(1);
	}
	if (pthread_create(&screenOutThread, NULL, screenOut, NULL) != 0) {
		printf("Error creating screenOut thread\n");
		exit(1);
	}
}

static void complexTestFreeFn(void* pItem) 
{
	pItem = NULL;
	printf("pItem: %p\n", pItem);
}

void receiverClose()
{
	pthread_join(networkInThread, NULL);
	pthread_join(screenOutThread, NULL);

	free(pMessage);
	pMessage = NULL;
	List_free(receiveList, complexTestFreeFn);
	pthread_mutex_unlock(&receiveListMutex);

	// Cleanup memory
	pthread_mutex_destroy(&receiveMessageMutex);
	pthread_mutex_destroy(&receiveListMutex);
	pthread_cond_destroy(&receiveMessageCondition);
	pthread_cond_destroy(&receiveListCondition);
}

static void cancelReceiveThread() 
{
	if (pthread_cancel(networkInThread) != 0) {
		printf("networkInThread cancelled with -1\n");
	}
	printf("Cancelled networkInThread\n");
}

static void cancelScreenThread() 
{
	if (pthread_cancel(screenOutThread) != 0) {
		printf("screenOutThread cancelled with -1\n");
	}
	printf("Cancelled screenOutThread\n");
}

void cancelReceiverThreads() 
{
	cancelReceiveThread();
	cancelScreenThread();
}