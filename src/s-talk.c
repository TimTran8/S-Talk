#include <stdio.h>
#include <stdlib.h>
// #include "list.h"
// #include "setup.h"
// #include "sender.h"
// #include "receiver.h"
#include "../headers/receiver.h"
#include "../headers/list.h"
#include "../headers/sender.h"
#include "../headers/setup.h"

int main(int argc, char** args)
{
	if (argc != 4) { // requires 3 arguments: my port, remote machine name, remote port num
		fprintf(stderr,"./s-talk [my port number] [remote machine name] [remote port number]\n");
		exit(1);
	}
	printf("Local Port: %s\n", args[1]);
	printf("Remote Machine: %s\n", args[2]);
	printf("Remote Port: %s\n", args[3]);

	setupNetwork(args[1], args[2], args[3]);

	senderInit();
	receiverInit();

	senderClose();
	receiverClose();
	closeNetwork();

	printf("Finished\n");
	return 0;
}