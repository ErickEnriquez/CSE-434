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

int check_message(char* message);//this function checks if the user enters a valid posst or retreive
char* attach_header(char*);
char* extract_ack(char*);


int main(int argc, char *argv[]) {

int ret;
int socketfd;
int buffer_size = 205;
char message [buffer_size];//size of message =200 + 5 for the post#
struct sockaddr_in serv_addr ,client_addr;
socklen_t len = sizeof(client_addr);

socketfd = socket(AF_INET,SOCK_DGRAM,0);
if(socketfd < 0){
    printf("Error with socket() %s \n" , strerror(errno));
    return -1;
}

memset(&client_addr,0,sizeof(client_addr));
client_addr.sin_family =AF_INET;
client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
client_addr.sin_port = htons(0);

if(bind(socketfd,(struct sockaddr *)&client_addr,sizeof(client_addr)) <0){
    perror("BIND FAIL:");
    return -1;
}

memset(&serv_addr,0,sizeof(serv_addr));//zeros out the memory at server addr
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//ip address
serv_addr.sin_port = htons(32000);//port number 


while (1){
    fgets(message,buffer_size,stdin);//get the input
    int val = check_message(message); 
    if(val != -1){
        char* out_message=attach_header(message);
        sendto(socketfd , out_message , strlen(out_message) , 0 , (struct sockaddr *)&serv_addr , sizeof(serv_addr) );
        memset(message,0,sizeof(message));
        recvfrom(socketfd,message,sizeof(message),0,(struct sockaddr *)&serv_addr,&len);
        extract_ack(message);
        }   
    }
    close(socketfd);
    return 0;
}


int check_message(char* message){
    char post_string[6] = {"post#"};
    int result  = strncmp(post_string,message,5);
    if (result == 0 && strlen(message) <=205 )
        return 1; //if 1 then we have a valid post command
    else {
        char retrieve[10] = "retrieve#";
        int x = strncmp(retrieve,message,9);
        if( x == 0 && strlen(message) <= 209){
            return 2;
        }
        else{
            printf("Error: Unrecognized command format\n");
            return -1;
        }
    }
}

char* attach_header(char* message){
    char* out_message = (char*)malloc(213);
    int x = check_message(message);
    if (x == -1){
         return NULL;
    }
    else{
        strcpy(out_message,"EE");//attach first 2 bytes of header
        if (x == 1){//if post
            strcpy(out_message + 2 , "1");
        }
        else if(x == 2){
            strcpy(out_message + 2 , "3");//attach 3 opcode for retrieve
        }
       /*  int length = strlen(message);
        unsigned char bytes[1] = {0};//stores the length of the string
        memcpy(bytes,&length,1);//copy the length into 1 single unsigned byte
        memcpy(out_message+3,&bytes[0],1);//copy that byte into header*/
        strcpy(out_message+3,message);
        /* CONVERT LENGTH BACK TO AN INTEGER
        unsigned char text = out_message[3];
        int number = (int)text;
        printf("extracting %d\n", number);
        */
       
    }
    return out_message;
}

//still need the retrieve functionality
char * extract_ack(char* ack){
    if(strcmp(ack,"EE2") == 0){
        printf("post_ack#successful\n");
    }
    else if(strncmp(ack,"EE4",3)==0)
        printf("%s",ack+3);
    return NULL;
}
