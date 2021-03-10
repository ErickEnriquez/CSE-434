/*
 * udp_server.c
 *
 *  Created on: Jun 17, 2018
 *      Author: duolu
 *
 *
 *  Compile this code using the command "gcc udp_server.c -o udp_server".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

	int ret;

	int sockfd;
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	char recv_buffer[1024];
	socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(32000);

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;

    bind(sockfd,
         (struct sockaddr *) &serv_addr,
         sizeof(serv_addr));

    while (1) {

    	memset(recv_buffer, 0, sizeof(recv_buffer));
        recvfrom(sockfd,
                 recv_buffer,
                 sizeof(recv_buffer),
                 0,
                 (struct sockaddr *)&client_addr,
                 &len);

        printf("[%s] %s",
               inet_ntoa(client_addr.sin_addr),
               recv_buffer);
    }
    close(sockfd);
    return 0;
}
