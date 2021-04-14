#ifndef _S_TALK_
#define _S_TALK_

#include <netdb.h>

int socketDescriptor;
struct addrinfo sinLocal;
struct addrinfo sinRemote;
struct addrinfo *localResult;
struct addrinfo *remoteResult;

void setupLocal();
void setupRemote();
void setupNetwork();
void setupThreads();
void closeNetwork();

#endif