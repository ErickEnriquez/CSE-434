/*
 * tcp_server.c
 *
 *  Created on: Jun 17, 2018
 *      Author: duolu
 *
 *
 *  Compile this code using the command "gcc tcp_server.c -o tcp_server -lpthread".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

void *worker_thread(void *arg) {

	int ret;
	int connfd = (int)(unsigned long)arg;
	char recv_buffer[1024];

	printf("[connfd-%d] worker thread started.\n", connfd);

	while (1) {
		ret = recv(connfd, recv_buffer, sizeof(recv_buffer), 0);

		if (ret < 0) {
			printf("[connfd-%d] recv() error: %s.\n", connfd, strerror(errno));
			return NULL;
		} else if (ret == 0) {
			printf("[connfd-%d] connection finished\n", connfd);
			break;
		}

		ret = send(connfd, recv_buffer, strlen(recv_buffer) + 1, 0);
		if (ret < 0) {
			printf("send() error: %s.\n", strerror(errno));
			break;
		}


		printf("[connfd-%d] %s", connfd, recv_buffer);
	}

	printf("[connfd-%d] worker thread terminated.\n", connfd);

	return NULL;
}

int main(int argc, char *argv[]) {
	int ret;
	socklen_t len;
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		printf("socket() error: %s.\n", strerror(errno));
		return -1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(31000);

	ret = bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	if (ret < 0) {
		printf("bind() error: %s.\n", strerror(errno));
		return -1;
	}

	if (listen(listenfd, 10) < 0) {
		printf("listen() error: %s.\n", strerror(errno));
		return -1;
	}

	while (1) {
		printf("waiting for connection...\n");
		connfd = accept(listenfd, (struct sockaddr*) &client_addr, &len);

		if (connfd < 0) {
			printf("accept() error: %s.\n", strerror(errno));
			return -1;
		}
		printf("connection accept from %s.\n", inet_ntoa(client_addr.sin_addr));

		pthread_t tid;
		pthread_create(&tid, NULL, worker_thread, (void *)(unsigned long)connfd);

	}
	return 0;
}
