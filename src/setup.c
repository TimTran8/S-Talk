#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "../headers/setup.h"

void closeNetwork()
{
	freeaddrinfo(localResult); // free the linked-list
	freeaddrinfo(remoteResult); // free the linked-list
	shutdown(socketDescriptor, SHUT_RDWR);
	close(socketDescriptor);
}

void setupLocal(char *localPort)
{
	struct addrinfo *p = NULL;
	memset(&localResult, 0, sizeof(localResult));
	sinLocal.ai_family = AF_INET;
	sinLocal.ai_socktype = SOCK_DGRAM;
	sinLocal.ai_flags = AI_PASSIVE;
	int rv = getaddrinfo(NULL, localPort, &sinLocal, &localResult);
	if (rv != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	for (p = localResult; p != NULL; p = p->ai_next)
	{
		if ((socketDescriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("socket error");
			closeNetwork();
			continue;
		}
		if (bind(socketDescriptor, p->ai_addr, p->ai_addrlen) == -1) {
			close(socketDescriptor);
			perror("bind error");
			closeNetwork();
			exit(1);
		}
		break;
	}
	localResult = p; // Assign res to bound pointer
}

void setupRemote(char *remotePort, char *address)
{
	struct addrinfo *p = NULL;
	memset(&remoteResult, 0, sizeof(remoteResult));
	sinRemote.ai_family = AF_INET;
	sinRemote.ai_socktype = SOCK_DGRAM;
	int rv = getaddrinfo(address, remotePort, &sinRemote, &remoteResult); // rv = return value
	if (rv != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	//This part prints out the IP address of the remote user given their remote machine name for debugging purposes
	for (p = remoteResult; p != NULL; p = p->ai_next)
	{
		if (p->ai_addr->sa_family == AF_INET)
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)p->ai_addr;
			char *ip = inet_ntoa(sin->sin_addr);
			printf("IP ADDRESS OF REMOTE: %s\n", ip);
		}
	}
}

void setupNetwork(char *localPort, char *address, char *remotePort)
{
	int local = atoi(localPort);
	int remote = atoi(remotePort);
	if (local > 65535 || remote > 65535 || local <= 1024 || remote <= 1024) {
		printf("Invalid port numbers\n");
		exit(1);
	}
	setupLocal(localPort);
	setupRemote(remotePort, address);
	printf("Finished setting up\n");
}