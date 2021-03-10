
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// This line must be included if you want to use multithreading.
// Besides, use "gcc ./tcp_receive.c -lpthread -o tcp_receive" to compile
// your code. "-lpthread" means link against the pthread library.
#include <pthread.h>


char global_buffer[1024];//global buffer 

char * create_ack(char*, int);


// This the "main" function of each worker thread. All worker thread runs
// the same function. This function must take only one argument of type 
// "void *" and return a value of type "void *". 
void *worker_thread(void *arg) {
    FILE* fp;//file pointer
    int ret;
    int connfd = (int) (long)arg;
    char recv_buffer[1024];
    char ack_buffer[1024];
    int FSIZE = 0;

    printf("[%d] worker thread started.\n", connfd);
    while (1) {
        ret = recv(connfd, 
                    recv_buffer, 
                    sizeof(recv_buffer), 
                    0);

        if (ret < 0) {
            // Input / output error.
		printf("[%d] recv() error: %s.\n", connfd, strerror(errno));
        	return NULL;
        } else if (ret == 0) {
            // The connection is terminated by the other end.
		printf("[%d] connection lost\n", connfd);
        	break;
        }
       
        int opcode = recv_buffer[2];//store the opcode














////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        if(opcode == 80){
        int file_name_size = recv_buffer[3];//get the size of the file
        //number of bytes the file will be
        char* filename  = (char*)malloc(file_name_size+1);
        recv(connfd,&FSIZE,sizeof(FSIZE),0);
        int FILE_SIZE = ntohl(FSIZE);//store the filesize
        recv(connfd,recv_buffer,sizeof(recv_buffer),0);//call the recv again to get the file name
        strncpy(filename,recv_buffer,file_name_size);
      //  printf("name of file is %s\n",filename);
        filename[strlen(filename)] = '\0';
        int data_recieved =0;
        strcpy(ack_buffer,filename),//store the name of the file for acknowledgment
        fp = fopen(filename,"wb");//open the file
        if (fp == NULL){
            printf("ERROR OPENING FILE\n");
            return NULL;
        }
        while(data_recieved < FILE_SIZE){
            data_recieved = data_recieved +  recv(connfd,recv_buffer,sizeof(recv_buffer),0);
            if(data_recieved < FILE_SIZE){//if the whole buffer belongs to the file
                fwrite(recv_buffer,1,1024,fp);//write the whole file to the buffer
            }
            else if(data_recieved >FILE_SIZE){//if this is last buffer and we only need to read partial
                int difference  = data_recieved - FILE_SIZE;
                fwrite(recv_buffer,1,1024-difference,fp);
            }
          // printf("%d BYTES RECIVED\n",data_recieved);
        }
        printf("OUT OF LOOP\n");
        fclose(fp);
        strcpy(ack_buffer,create_ack(ack_buffer,opcode));
        ret = send(connfd,ack_buffer,sizeof(ack_buffer),0);       
        printf("%d BYTES SENT BACK TO THE CLIENT\n",ret);
        }
        else if(opcode == 82){//if we get a download request
            int FILE_NAME_SIZE =recv_buffer[3];
            char * filename = (char*)malloc(FILE_NAME_SIZE + 1);
            recv(connfd,recv_buffer,sizeof(recv_buffer),0);//get the name of the file
            strncpy(filename,recv_buffer,FILE_NAME_SIZE);//copy the name into filename 
            filename[strlen(filename)] = '\0';//addd a terminating character
            printf("FILE WE WANT TO RETRIEVE IS %s\n",filename);
            fp = fopen(filename,"rb");
            if(fp == NULL){
                printf("ERROR OPENING FILE\n");
                return NULL;
            }
            fseek(fp,0,SEEK_END);//go the the end of the file
            int FSIZE = ftell(fp);//get the size of the file
            fseek(fp,0,SEEK_SET);//return to the top of the file in order to ensure we read correctly
            int converted_number = htonl(FSIZE);
            printf("SIZE OF FILE is %d\n",FSIZE);
            ret = send(connfd,&converted_number,sizeof(converted_number),0);//send the converted number
            printf("%d bytes send\n",ret);
            int data_sent = 0;
            while(data_sent != FSIZE){
                data_sent = data_sent + fread(recv_buffer,1,1024,fp);
                send(connfd,recv_buffer,sizeof(recv_buffer),0);
            }
            printf("DONE SENDING\n");



            fclose(fp);//close the file

        }






        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    printf("[%d] worker thread terminated.\n", connfd);
    return NULL;
}


// The main thread, which only accepts new connections. Connection socket
// is handled by the worker thread.
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

    ret = bind(listenfd, (struct sockaddr*) 
               &serv_addr, sizeof(serv_addr));
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
        connfd = accept(listenfd, 
                 (struct sockaddr*) &client_addr, 
                 &len);

        if(connfd < 0) {
		printf("accept() error: %s.\n", strerror(errno));
        	return -1;
        }
	  printf("conn accept - %s.\n", inet_ntoa(client_addr.sin_addr));

	  pthread_t tid;
	  pthread_create(&tid, NULL, worker_thread, (void *)(long)connfd);

    }
    return 0;
}





char* create_ack(char* filename , int opcode){
    if(opcode ==  80){//if opcode is upload
        strcpy(global_buffer,"EEQQ");
        strcpy(global_buffer+4,filename);
    }   
    else if(opcode == 82){//if opcode is download

    }
    return global_buffer;
}
