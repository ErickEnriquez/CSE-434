
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

FILE* fp;

int check_before_send(char*);//function will check to see if the format is correct before sending
char* create_header(char*);//this messsage will return the header of a file
void parse_ack(char*);//this will parse the acknowledgment and print message correctly

int main(int argc, char *argv[]) {
    int ret;
    int sockfd = 0;
    char send_buffer[1024];
    char header_buffer[8];//stores the header created
    char size_of_file[4];//send the size of the file on its own
    char temp[1024];//temporary buffer
    int FSIZE =0;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
	  printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    // Note that this is the server address that the client will connect to.
    // We do not care about the source IP address and port number. 

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(31000);

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

        // These two lines allow the client to "gracefully exit" if the
        // user type "exit".
        
        if (strncmp(send_buffer, "exit", strlen("exit")) == 0)
		break;

            int opcode = check_before_send(send_buffer);

          if(opcode != -1){// if we have a valid command
            //if we get a valid request then loop here
            bzero(header_buffer,sizeof(header_buffer));//clear the header buffer
            memcpy(header_buffer,create_header(send_buffer),8);//copy the data
            if(strncmp(send_buffer,"upload$",strlen("upload$")) == 0){//get the name of the file 
                strcpy(temp,send_buffer + strlen("upload$"));//store the file name
                temp[strlen(temp)-1] = '\0';
            }
            else if (strncmp(send_buffer,"download$",strlen("download$")) == 0){
                strcpy(temp,send_buffer + strlen("download$"));
                temp[strlen(temp)-1] = '\0';
            }


            

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if(opcode == 1){
            fp = fopen(temp,"rb");
            if(fp == NULL){
                printf("ERROR OPENING FILE");
            }
            fseek(fp,0,SEEK_END);//go the the end of the file
            int filesize = ftell(fp);//get the size of the file
            fseek(fp,0,SEEK_SET);//return to the top of the file in order to ensure we read correctly
            int data_read = 0;
            int data_sent = 0;
            int result = 0;
                memcpy(send_buffer,header_buffer,8);//copy yhe headwe buffer into the send buffer
            result = send(sockfd,send_buffer,sizeof(send_buffer),0);//send th header
            if(result < 0){
                printf("ERROR SENDING DATA");
                return -1;
            }
            int converted_number = htonl(filesize);
            result = send(sockfd,&converted_number,sizeof(converted_number),0);//send filesize
            result = send(sockfd,temp,sizeof(temp),0);//send the file_name
            data_read = fread(send_buffer,1,1024,fp);
            data_sent = send(sockfd,send_buffer,sizeof(send_buffer),0);//send the first 1024 bytes of file
            while(data_read != filesize){
                data_read = data_read + fread(send_buffer,1,1024,fp);
                data_sent = send(sockfd,send_buffer,sizeof(send_buffer),0);
            }
            printf("Done Sending\n");
             printf("WAITING FOR ACK\n");
             result = recv(sockfd,send_buffer,sizeof(send_buffer),0);//get the ackowledgment
            parse_ack(send_buffer);;//parse the acknowledgement
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if(opcode == 2){//if we have a dowload command
                int result = 0;
                fp = fopen(temp,"wb");//open up a file 
                if(fp == NULL){
                    printf("ERROR OPEING FILE");
                }
                memcpy(send_buffer,header_buffer,8);//place the header
                result = send(sockfd,send_buffer,sizeof(send_buffer),0);//send the header
                result = send(sockfd,temp,sizeof(temp),0);//send the file_name you want
                int TEMP=0;
                recv(sockfd,&TEMP,sizeof(TEMP),0);//get the number of bytes
                 int FILESIZE = ntohl(TEMP);
                printf("SIZE OF FILE DOWNLOADED IS %d\n" ,FILESIZE);

                int data_recieved = 0;
                while(data_recieved< FILESIZE){
                    data_recieved = data_recieved + recv(sockfd,send_buffer,sizeof(send_buffer),0);//store how much data is being recieved
                    if(data_recieved < FILESIZE){//whole buffer is part of file
                        fwrite(send_buffer,1,1024,fp);
                    }
                    else if(data_recieved > FILESIZE){//has garbage data
                        int difference = data_recieved - FILESIZE;
                        fwrite(send_buffer,1,1024-difference,fp);
                    }
                }
                printf("download_ack$_%s_successfully!\n",temp);

                
                
            }

         }
         fclose(fp);//close the file
           
            
    }
    
    close(sockfd);

    return 0;
}



int check_before_send(char* message){
    if(strncmp(message , "upload$" ,strlen("upload$")) == 0){
        printf("UPLOADING FILE\n");
        return 1;
    }
    else if (strncmp(message,"download$",strlen("download$")) == 0){
        printf("DOWNLOADING FILE\n");
        return 2;
    }
    else{
        printf("Unrecognized format\n");
        return -1;
    }
}

char* create_header(char* message){
    char* header = (char*)malloc(1024);//make a buffer to hold the header
    char file_name[1024];
    strncpy(header,"EE",strlen("EE"));//attach the magic numbers
    if(strncmp(message,"upload$",strlen("upload$")) == 0){//if user sends an upload request
            strncpy(header+2,"P",1);//Q has value of 80
            strcpy(file_name,message+strlen("upload$"));
    }
    else if(strncmp(message,"download$",strlen("download$")) == 0){//if user sends a download request
            strncpy(header+2,"R",1);//R has value of 82
             strcpy(file_name,message+strlen("download$"));
    }

//////////////////////////////////////////////////////////////////////////////////////////////////
    if(strncmp(message,"upload$",strlen("upload$")) == 0){//if we have an upload request
    file_name[strlen(file_name)-1] = '\0';//removes the extra newline character from the file_name
    fp = fopen(file_name,"rb");
    if(fp == NULL){
        printf("ERROR OPENING FILE\n");
        return NULL;
    }
    fseek(fp,0,SEEK_END);//go the the end of the file
    unsigned int filesize = ftell(fp);//get the size of the file
        char a[4];
        a[0] = filesize & 0xff;
        a[1] = (filesize>>8) & 0xff;
        a[2] = (filesize>>8) & 0xff;
        a[3] = (filesize>>8) & 0xff;
        strncpy(header+4,a,4);
    fseek(fp,0,SEEK_SET);//return to the top of the file in order to ensure we read correctly
    char file_name_size = (int)strlen(file_name);//get the size of the filename
    strncpy(header+3,&(file_name_size),1);//store the size of the file
    header[8] = '\0';//add the end of line character back in
    fclose(fp);//close the file
    }
    else if(strncmp(message,"download$",strlen("download$")) == 0){
        char File_name_size = (int)strlen(file_name);
        strncpy(header+3,&(File_name_size),1);//inlcude the size of the file
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////
    return header;
}

void parse_ack(char* message){//parse the message a server sent us
    int opcode  = message[2];//get the opcode
    if(opcode == 81){//if we have an acknowledgment 
        printf("upload$ %s _ack_success\n",message+4);
    }
    else if(opcode == 83){//if we have a download success ack

    }
}
