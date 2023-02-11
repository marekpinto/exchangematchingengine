// This file contains main() as well as the logic setting up the I/O.
// There should be no need to modify this file.

#include <stdio.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "io.hpp"
#include "engine.hpp"

static int listenfd = -1;
static char* socketpath = NULL;

static void handle_exit_signal(int signum)
{
	(void) signum;
	exit(0);
}

static void exit_cleanup(void)
{
	if(listenfd == -1)
		return;

	close(listenfd);
	if(socketpath)
		unlink(socketpath);
}

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s <socket path>\n", argv[0]);
		return 1;
	}

	socketpath = argv[1];
	listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(listenfd == -1)
	{
		perror("socket");
		return 1;
	}

	{
		struct sockaddr_un sockaddr {};
		sockaddr.sun_family = AF_UNIX;
		strncpy(sockaddr.sun_path, argv[1], sizeof(sockaddr.sun_path) - 1);
		if(bind(listenfd, (const struct sockaddr*) &sockaddr, sizeof(sockaddr)) != 0)
		{
			perror("bind");
			return 1;
		}
	}

	atexit(exit_cleanup);
	signal(SIGINT, handle_exit_signal);
	signal(SIGTERM, handle_exit_signal);

	if(listen(listenfd, 8) != 0)
	{
		perror("listen");
		return 1;
	}

	auto engine = new Engine();
	while(true)
	{
		int connfd = accept(listenfd, NULL, NULL);
		if(connfd == -1)
		{
			perror("accept");
			return 1;
		}

		engine->accept(ClientConnection(connfd));
	}

	return 0;
}
