#include <cstdio>
#include <enet/enet.h>

int testNetwork(){
	if (enet_initialize () != 0)
	{
		fprintf (stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	printf("ENet initialized successfully\n");
	atexit (enet_deinitialize);
	return 0;
}
