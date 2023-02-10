#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <poll.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/un.h>
#include <sys/socket.h>

#include <atomic>

#include "io.hpp"

#define INPUT_CANCEL_ORDER 'C'
#define INPUT_BUY_ORDER 'B'
#define INPUT_SELL_ORDER 'S'

static char* line_buffer;
static size_t line_buffer_size = 0;
static std::atomic<bool> main_is_exiting = 0;

static void* poll_thread(void* fdptr)
{
	struct pollfd pfd {};
	pfd.fd = (int) (long) fdptr;
	pfd.events = 0;

	while(!main_is_exiting)
	{
		if(poll(&pfd, 1, -1) == -1)
		{
			perror("poll");
			_exit(1);
		}
		if(main_is_exiting)
		{
			break;
		}
		if(pfd.revents & (POLLERR | POLLHUP))
		{
			fprintf(stderr, "Connection closed by server\n");
			_exit(0);
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s <path of socket to connect to> < <input>\n", argv[0]);
		return 1;
	}

	int clientfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(clientfd == -1)
	{
		perror("socket");
		return 1;
	}

	{
		struct sockaddr_un sockaddr {};
		sockaddr.sun_family = AF_UNIX;
		strncpy(sockaddr.sun_path, argv[1], sizeof(sockaddr.sun_path) - 1);
		if(connect(clientfd, (const struct sockaddr*) &sockaddr, sizeof(sockaddr)) != 0)
		{
			perror("connect");
			return 1;
		}
	}

	FILE* client = fdopen(clientfd, "r+");
	setbuf(client, NULL);

	pthread_t poll_thread_handle;
	if(pthread_create(&poll_thread_handle, NULL, poll_thread, (void*) (long) clientfd) < 0)
	{
		fprintf(stderr, "Failed to create poll thread\n");
		return 1;
	}

	while(1)
	{
		ClientCommand input {};

		ssize_t line_length = getline(&line_buffer, &line_buffer_size, stdin);
		if(line_length == -1)
			break;

		switch(line_buffer[0])
		{
			case '#':
			case '\n': continue;
			case INPUT_CANCEL_ORDER:
				input.type = input_cancel;
				if(sscanf(line_buffer + 1, " %u", &input.order_id) != 1)
				{
					fprintf(stderr, "Invalid cancel order: %s\n", line_buffer);
					return 1;
				}
				break;
			case INPUT_BUY_ORDER: input.type = input_buy; goto new_order;
			case INPUT_SELL_ORDER:
				input.type = input_sell;
			new_order:
				if(sscanf(line_buffer + 1, " %u %8s %u %u", &input.order_id, input.instrument, &input.price, &input.count) != 4)
				{
					fprintf(stderr, "Invalid new order: %s\n", line_buffer);
					return 1;
				}
				break;
			default: fprintf(stderr, "Invalid command '%c'\n", line_buffer[0]); return 1;
		}

		if(fwrite(&input, 1, sizeof(input), client) != sizeof(input))
		{
			fprintf(stderr, "Failed to write command\n");
			return 1;
		}
	}

	main_is_exiting = 1;
	fclose(client);

	return ferror(stderr) ? 1 : 0;
}
