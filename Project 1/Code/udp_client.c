/*
 * udp_client.c
 *
 *  Created on: Jun 17, 2018
 *      Author: duolu
 *
 *
 *  Compile this code using the command "gcc udp_client.c -o udp_client".
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
    int sockfd = 0;
    char send_buffer[1024];
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
	printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr
        = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(32000);

    ret = connect(sockfd,
                  (struct sockaddr *) &serv_addr,
                  sizeof(serv_addr));
    if (ret < 0) {
	printf("connect() error: %s.\n", strerror(errno));
        return -1;
    }

    while (1) {

        fgets(send_buffer,
              sizeof(send_buffer),
              stdin);

        sendto(sockfd,
               send_buffer,
               strlen(send_buffer),
               0,
               (struct sockaddr *) &serv_addr,
               sizeof(serv_addr));

	}

    close(sockfd);

    return 0;
}
