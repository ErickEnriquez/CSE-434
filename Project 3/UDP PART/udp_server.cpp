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
#include<string>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <time.h>

int count = 0 ;
using namespace std;
char* extract_message(char*);
char* create_acknowlegment(char *);
string create_line_to_write(time_t,struct sockaddr_in,char* message);

char temp[1024];

int main(int argc, char *argv[]) {
    int count = 0 ;
    FILE *fp;
    fp=fopen("log.txt","wr");
    time_t current_time;
    
    char messages[1024][1024];
    int length ;//length of the message
	int sockfd;                     //the socket file descriptor
	struct sockaddr_in serv_addr;   //server ip address
	struct sockaddr_in client_addr; //client ip address
	char recv_buffer[1024];
    char* acknowledgment = (char*)malloc(1024);
	socklen_t len = sizeof(client_addr);

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

    for (;;) {
        current_time = time(NULL);
        printf("Waiting on port 32000\n");
      char message[1024];
    	memset(recv_buffer, 0, sizeof(recv_buffer));
        memset(message,0,strlen(message));
        memset(acknowledgment,0,strlen(acknowledgment));//clear the buffer every time
        recvfrom(sockfd,
                 recv_buffer,
                 sizeof(recv_buffer),
                 0,
                 (struct sockaddr *)&client_addr,
                 &len);
        
        strcpy(message,extract_message(recv_buffer));//extract the message and put it into the acknowlegdment
        strcpy(acknowledgment,create_acknowlegment(message));
        if(strncmp(acknowledgment,"EE4",3)==0 && count >0)//if we need to send the previous message
            strcpy(acknowledgment+3,messages[count-1]);//copy the previous message 
        strcpy(messages[count],message);//store the message
        count++;
        //-------------------------------------------------
        message[strlen(message)-1] = '\0';//cut the newline character to store in logfile
            fprintf(fp,"%s [%s , %d] %s",message,inet_ntoa(client_addr.sin_addr),(int)ntohs(client_addr.sin_port),ctime(&current_time));
            fflush(fp);
        //---------------------------------------------------
        
       if( sendto(sockfd,acknowledgment,strlen(acknowledgment),0,(struct sockaddr *)&client_addr,(socklen_t)sizeof(client_addr)) < 0 )
           perror("SENDTO()");
        
        
       

    }
    close(sockfd);
    fclose(fp);
    return 0;
}

char* extract_message(char* message){
    memset(temp,0,sizeof(temp));
    int result = strncmp("post#",message + 3,5);
    if (result == 0 ){
       strncpy(temp,message + 3, strlen(message) - 3);//take the original part of the message
    }
    else{
        
        if (strncmp(message+2,"3",1) == 0){
            strncpy(temp,message+3,strlen(message) - 3);
        }
    }


    return temp;
}

char* create_acknowlegment(char* message){
   memset(temp,0,sizeof(temp));
    if(strncmp("post#",message,5) == 0)
        strcpy(temp,"EE2");
    else if(strncmp("retrieve#",message,9) == 0)
        strcpy(temp,"EE4");
    return temp;
}

