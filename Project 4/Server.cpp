
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <time.h>
#include <map> 
#include <utility>
#include <stdlib.h> 
#include <vector>
#include <sys/select.h>

using namespace std;

struct header {

    char magic1;
    char magic2;
    char opcode;
    char payload_len;

    uint32_t token;
    uint32_t msg_id;
};

const int h_size = sizeof(struct header);

// These are the constants indicating the states.
// CAUTION: These states have nothing to do with the states on the client.
#define STATE_OFFLINE          0
#define STATE_ONLINE           1
#define STATE_MSG_FORWARD      2
#define STATE_RETRIEVE         3
// Now you can define other states in a similar fashion.

// These are the events
// CAUTION: These events have nothing to do with the states on the client.
#define EVENT_NET_LOGIN                 80
#define EVENT_NET_POST                  81
#define EVENT_LOGIN                     83
#define EVENT_NET_LOGOUT                84
#define EVENT_NET_SUBSCRIBE             85
#define EVENT_NET_UNSUBSCRIBE           86
#define EVENT_NET_FORWARD_ACK           87
#define EVENT_NET_RETRIEVE              88
#define EVENT_GENEERATE_INVALID_MESSAGE 89
#define EVENT_RESET                     90
// Now you can define other events from the network.
//......
#define EVENT_NET_INVALID               255

// These are the constants indicating the opcodes.          //NUMBERS NEED TO BE SMALLET THAN 0x7F as that is 127 or the smallest number that can be held in 1 bit 
// CAUTION: These opcodes must agree on both sides.
#define OPCODE_RESET                    0x00
#define OPCODE_MUST_LOGIN_FIRST_ERROR   0x7C
#define OPCODE_LOGIN                    0x10
#define OPCODE_SUCCESSFUL_LOGIN_ACK     0x7E
#define OPCODE_FAILED_LOGIN_ACK         0x7F
#define OPCODE_POST                     0x30
#define OPCODE_POST_ACK                 0x7D
#define OPCODE_SUBSCRIBE_REQUEST        0x7B
#define OPCODE_UNSUBSCRIBE_REQUEST      0x7A
#define OPCODE_SUBSCRIBE_SUCCESS        0x79
#define OPCODE_SUBSCRIBE_FAILURE        0x78
#define OPCODE_UNSUBSCRIBE_SUCCESS      0x77
#define OPCODE_UNSUBSCRIBE_FAILURE      0x76
#define OPCODE_FORWARD                  0x75
#define OPCODE_FORWARD_ACK              0x74
#define OPCODE_RETRIEVE                 0x73
#define OPCODE_RETRIEVE_ACK             0x72
#define OPCODE_END_OF_RETRIEVE_ACK      0x71
#define OPCODE_LOGOUT                   0x70
#define OPCODE_LOGOUT_SUCCESS           0x6F
#define OPCODE_SPECIAL_USER             0x5D
#define OPCODE_INVALID                  0x11
/////////////////
/*

*/
//////////////////
// Now you can define other opcodes in a similar fashion.
//......
#define MAGIC_1                         'E'
#define MAGIC_2                         'E' 

void hard_code_sessions();
int extract_token_from_the_received_binary_msg(char*);
struct session * find_the_session_by_token(int);
int parse_the_event_from_the_datagram(char*);
int check_id_password(char*,char*);
uint32_t generate_a_random_token(struct session*);
struct session* find_this_client_in_the_session_array(char*);
bool remove_client_in_cs_map(char*,struct session*);
time_t right_now();
void print_map();
void check_liveliness();

// This is a data structure that holds important information on a session.
struct session {

    char client_id[32]; // Assume the client ID is less than 32 characters.
    struct sockaddr_in client_addr; // IP address and port of the client
                                    // for receiving messages from the 
                                    // server.
    time_t last_time; // The last time when the server receives a message
                      // from this client.
    uint32_t token;        // The token of this session.
    int state;        // The state of this session, 0 is "OFFLINE", etc.
    std::map<char*,session*> subscriptions  ;   //list of the subsciptions
    char* password;     //users passwords
    // TODO: You may need to add more information such as the subscription
    // list, password, etc.
};


std::vector<std::pair<char*,string> > messages_list;
struct registered_clients{
    struct session** clients;//list of the registered clients
};

map<uint32_t,session*> list;//list of all the sessions
// TODO: You may need to add more structures to hold global information
// such as all registered clients, the list of all posted messages, etc.
// Initially all sessions are in the OFFLINE state.

int main() {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int ret;
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    char send_buffer[1024];
    char recv_buffer[1024];
    int recv_len;
    socklen_t len;
    fd_set read_set;
    FD_ZERO(&read_set);
    int maxfd;

    // You may need to use a std::map to hold all the sessions to find a 
    // session given a token. I just use an array just for demonstration.
    // Assume we are dealing with at most 16 clients, and this array of
    // the session structure is essentially our user database.
    struct session session_array[16];

    // Now you need to load all users' information and fill this array.
    // Optionally, you can just hardcode each user.
    hard_code_sessions();// add a couple of sessions into the map container
  //  print_map();
    // This current_session is a variable temporarily hold the session upon
    // an event.
    struct session *current_session;
    int token;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    // The servaddr is the address and port number that the server will 
    // keep receiving from.
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(32000);

    bind(sockfd, 
         (struct sockaddr *) &serv_addr, 
         sizeof(serv_addr));

    // Same as that in the client code.
    struct header *ph_send = (struct header *)send_buffer;
    struct header *ph_recv = (struct header *)recv_buffer;
    maxfd = sockfd + 1;
    while (1) {

        // Note that the program will still block on recvfrom()
        // You may call select() only on this socket file descriptor with
        // a timeout, or set a timeout using the socket options./////////////
        FD_SET(sockfd, &read_set);
        struct timeval tv = {5, 0}; 
        select(maxfd, &read_set, NULL, NULL, &tv);
        if(FD_ISSET(sockfd,&read_set)){
//////////////////////////////////////////////////////////////////////////
        len = sizeof(cli_addr);
        recv_len = recvfrom(sockfd, // socket file descriptor
                 recv_buffer,       // receive buffer
                 sizeof(recv_buffer),  // number of bytes to be received
                 0,
                 (struct sockaddr *) &cli_addr,  // client address
                 &len);             // length of client address structure

        if (recv_len <= 0) {
            printf("recvfrom() error: %s.\n", strerror(errno));
            return -1;
        }




        // Now we know there is an event from the network
        // TODO: Figure out which event and process it according to the
        // current state of the session referred.

        
        int token = extract_token_from_the_received_binary_msg(recv_buffer);
        // This is the current session we are working with.
        struct session *cs = find_the_session_by_token(token);
        
        int event = parse_the_event_from_the_datagram(recv_buffer);

        // Record the last time that this session is active.
     //   cout << "EVENT IS " << event << endl;
        if (event == EVENT_LOGIN) {

            // For a login message, the current_session should be NULL and
            // the token is 0. For other messages, they should be valid.

            char *id_password = recv_buffer + h_size;

            char *delimiter = strchr(id_password, '&');
            char *password = delimiter + 1;
            *delimiter = 0; // Add a null terminator
            // Note that this null terminator can break the user ID
            // and the password without allocating other buffers.
            char *user_id = id_password;

            delimiter = strchr(password, '\n');
            *delimiter = 0; // Add a null terminator
            // Note that since we did not process it on the client side,
            // and since it is always typed by a user, there must be a
            // trailing new line. We just write a null terminator on this
            // place to terminate the password string.




            // The server need to reply a msg anyway, and this reply msg
            // contains only the header
            ph_send->magic1 = MAGIC_1;
            ph_send->magic2 = MAGIC_2;
            ph_send->payload_len = 0;
            ph_send->msg_id = 0;
            

            int login_success = check_id_password(user_id, password);
            if (login_success > 0) {

                // This means the login is successful.

                
                ph_send->opcode = OPCODE_SUCCESSFUL_LOGIN_ACK;

                cs = find_this_client_in_the_session_array(user_id);
                ph_send->token = generate_a_random_token(cs);

                
                cs->state = STATE_ONLINE;
                cs->token = ph_send->token;
                cs->last_time = time(NULL);
                cs->client_addr = cli_addr;

            } else {
              ph_send->opcode = OPCODE_FAILED_LOGIN_ACK;
                ph_send->token = 0;

            }
            
            sendto(sockfd, send_buffer, h_size, 0, 
                (struct sockaddr *) &cli_addr, sizeof(cli_addr));



        }else if(event == EVENT_NET_POST){
            // The server need to reply a msg anyway, and this reply msg
            // contains only the header
            ph_send->magic1 = MAGIC_1;
            ph_send->magic2 = MAGIC_2;
            ph_send->payload_len = 0;
            ph_send->msg_id = 0; // will probably change just placeholder rn
            if(cs!= NULL){
                if(cs->state == STATE_ONLINE){
                    ph_send->opcode = OPCODE_POST_ACK;
                    cs->last_time = right_now();
                }
                else {   
                ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
                }
            }
            else {  
                ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
            }
            sendto(sockfd, send_buffer, h_size, 0, 
                (struct sockaddr *) &cli_addr, sizeof(cli_addr));
            /////////////////////////////////////////////////////
            if(ph_send->opcode == OPCODE_POST_ACK){//if we actually sent a message
               bool in_list = false;
               struct session* target;
              for(std::map<uint32_t, session *>::iterator it  = list.begin();it != list.end(); it++ ) {//go through each item in list
                for(std::map<char*,session*>::iterator iterator = it->second->subscriptions.begin();//go through all subscribers
                iterator != it->second->subscriptions.end(); iterator++){
                
                    if(strncmp(iterator->second->client_id,cs->client_id,strlen(iterator->second->client_id)) == 0){
                     
                          if(it->second->state == STATE_ONLINE){ 
                              
                               char *text = recv_buffer + h_size;
                               char *payload = send_buffer + h_size;
                               target = it->second;
                   
                     // This formatting the "<client_a>some_text" in the payload
                    // of the forward msg, and hence, the client does not need
                    // to format it, i.e., the client can just print it out.
                                snprintf(payload, sizeof(send_buffer) - h_size, "<%s>%s",
                                cs->client_id, text);
                                int m = strlen(payload);

                    // "target" is the session structure of the target client.
                                target->state = STATE_MSG_FORWARD;

                                ph_send->magic1 = MAGIC_1;
                                ph_send->magic2 = MAGIC_2;
                                ph_send->opcode = OPCODE_FORWARD;
                                ph_send->payload_len = m;
                                ph_send->msg_id = 0; // Note that I didn't use msg_id here.

                                sendto(sockfd, send_buffer, h_size + m, 0, 
                                        (struct sockaddr *) &target->client_addr, 
                                        sizeof(target->client_addr));
                       
                        }
                    }
                }

            }


                char *text = recv_buffer + h_size;
                char *payload = send_buffer + h_size;
                 snprintf(payload, sizeof(send_buffer) - h_size, "<%s>%s",
                                cs->client_id, text);
                                int m = strlen(payload);
                string message(payload);

                messages_list.push_back(make_pair(cs->client_id,message));//store the message in the global list
               // cout << "adding " << message << endl;
          }  ///////////////////////////////////////////////////////
        }
        else if(event == EVENT_NET_LOGOUT){
            ph_send->magic1 = MAGIC_1;
            ph_send->magic2 = MAGIC_2;
            ph_send->payload_len = 0;
            ph_send->msg_id = 0; // will probably change just placeholder rn
            if(cs != NULL){
                if(cs->state == STATE_ONLINE){
                    cs->state  = STATE_OFFLINE;
                    cs->last_time = right_now();
                    ph_send->opcode = OPCODE_LOGOUT_SUCCESS;
                }
                else{
                    ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
                }
                sendto(sockfd, send_buffer, h_size, 0, 
                (struct sockaddr *) &cli_addr, sizeof(cli_addr));
            }
        }
        else if(event == EVENT_NET_SUBSCRIBE){
            ph_send->magic1 = MAGIC_1;
            ph_send->magic2 = MAGIC_2;
            ph_send->payload_len = 0;
            ph_send->msg_id = 0; // will probably change just placeholder rn

            if(cs!= NULL){
                ph_send->token = cs->token;
                if(cs->state == STATE_ONLINE){
                    char *client_target = recv_buffer + h_size;
                    struct session* client_target_session =  find_this_client_in_the_session_array(client_target);

                    if(client_target_session != NULL ){
                        std::pair<char*,session*>pair;
                        pair.first = client_target_session->client_id;
                        pair.second = client_target_session;
                        cs->subscriptions.insert(pair);
                        ph_send->opcode = OPCODE_SUBSCRIBE_SUCCESS;
                    }
                    else{
                        ph_send->opcode = OPCODE_SUBSCRIBE_FAILURE;
                    }
                }
                else{//if we are not in online state
                    ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
                    cs->state = STATE_OFFLINE;
                }
                
            }
            else{
                ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
            }
             sendto(sockfd, send_buffer, h_size, 0, 
                (struct sockaddr *) &cli_addr, sizeof(cli_addr));
        } 
        else if(event == EVENT_NET_UNSUBSCRIBE ){
            ph_send->magic1 = MAGIC_1;
            ph_send->magic2 = MAGIC_2;
            ph_send->payload_len = 0;
            ph_send->msg_id = 0; 
            if(cs != NULL){
                if(cs->state == STATE_ONLINE){
                     char *client_target = recv_buffer + h_size;
                     bool result = remove_client_in_cs_map(client_target,cs);
                     if(result == true){//if we could remove successfully
                        ph_send->opcode = OPCODE_UNSUBSCRIBE_SUCCESS;
                     }
                     else{
                         ph_send->opcode = OPCODE_UNSUBSCRIBE_FAILURE;
                     }
                }
                else{
                    ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
                }
            }
            else{
                ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
            }
            sendto(sockfd, send_buffer, h_size, 0, 
                (struct sockaddr *) &cli_addr, sizeof(cli_addr));
        }
        else if(event == EVENT_NET_FORWARD_ACK){
            if(cs != NULL)
                cs->state = STATE_ONLINE;//reset the state
        }
        else if(event == EVENT_NET_RETRIEVE){
            if(cs != NULL){
                 ph_send->magic1 = MAGIC_1;
                 ph_send->magic2 = MAGIC_2;
                 ph_send->payload_len = 0;
                 ph_send->msg_id = 0;
                if (cs->state == STATE_ONLINE){
                    char* string_number = recv_buffer + h_size;
                    int num = atoi(string_number);
                //    cout << num <<endl;
                //    cout << messages_list.size() << endl;
                    for(std::vector<std::pair<char*,string> >::reverse_iterator iterator = messages_list.rbegin(); iterator != messages_list.rend(); iterator++){//loop through list
              //          cout << "message" << iterator->second << endl;
                        for(std::map<char*, session *>::iterator it  = cs->subscriptions.begin();
                             it != cs->subscriptions.end(); it++ ){
                  //               cout << "checking" << it->second->client_id <<endl;
                                if(strncmp(it->first,iterator->first,strlen(it->first))== 0){
                                    if(num > 0){
                    //                    cout <<"MATCH\n";
                                        ph_send->opcode = OPCODE_RETRIEVE_ACK;
                                       int m  = iterator->second.size();
                                        memcpy(send_buffer+h_size, iterator->second.c_str(),iterator->second.size());
                                        sendto(sockfd, send_buffer, h_size + m, 0, 
                                        (struct sockaddr *) &cs->client_addr, 
                                        sizeof(cs->client_addr));
                                        num--;
                                        
                                    }
                                }
                            }
                        }
                         if(num == 0){
                        ph_send->opcode = OPCODE_END_OF_RETRIEVE_ACK;
                        sendto(sockfd, send_buffer, h_size, 0, 
                                (struct sockaddr *) &cs->client_addr, 
                                sizeof(cs->client_addr));

                        }

                }
                else{
                    ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
                     sendto(sockfd, send_buffer, h_size, 0, 
                                (struct sockaddr *) &cs->client_addr, 
                                sizeof(cs->client_addr));
                               // cout  << "SENT RESET\n";
                }
            }
            else{
                ph_send->magic1 = MAGIC_1;
                 ph_send->magic2 = MAGIC_2;
                 ph_send->payload_len = 0;
                 ph_send->msg_id = 0;
                 ph_send->opcode = OPCODE_MUST_LOGIN_FIRST_ERROR;
                     sendto(sockfd, send_buffer, h_size, 0, 
                    (struct sockaddr *) &cli_addr, sizeof(cli_addr));
                                cout  << "SENT RESET HERE\n";
            }
        }
        else if(event == EVENT_GENEERATE_INVALID_MESSAGE){
            if(cs!= NULL){
            cs->last_time = right_now();
            ph_send->magic1 = MAGIC_1;
            ph_send->magic2 = MAGIC_2;
            ph_send->payload_len = 0;
            ph_send->msg_id = 0;
            ph_send->opcode = OPCODE_INVALID; // INVALID CODE WHICH WILL BE CAUGHT BY USER
            sendto(sockfd, send_buffer, h_size, 0, 
                (struct sockaddr *) &cli_addr, sizeof(cli_addr));
            
            }
        }
        else if (event == EVENT_RESET){
            if(cs != NULL){
                cs->state = STATE_OFFLINE;
                cout << "CLIENT " << cs->client_id<< " GENERATED RESET\n";
            }
        }
        else{
            if(cs != NULL){
            cs->state = STATE_OFFLINE; //KICK USER OFF SESSION
            cs->last_time = right_now();
            ph_send->magic1 = MAGIC_1;
            ph_send->magic2 = MAGIC_2;
            ph_send->payload_len = 0;
            ph_send->msg_id = 0;
            ph_send->opcode = OPCODE_RESET;
            sendto(sockfd, send_buffer, h_size, 0, 
                (struct sockaddr *) &cli_addr, sizeof(cli_addr));
            cout << cs->client_id << " SESSION RESET BY SERVER " << endl;
            }

        }
        
        
        
        check_liveliness();

        // Now you may check the time of clients, i.e., scan all sessions. 
        // For each session, if the current time has passed 5 minutes plus 
        // the last time of the session, the session expires.
        // TODO: check session liveliness

        bzero(recv_buffer,1024);//clear the buffer every time
        }
        else{//if we timeout we check liveliness
          
            check_liveliness();
        }
    } // This is the end of the while loop

    return 0;
} // This is the end of main()

int extract_token_from_the_received_binary_msg(char* message){
    if(message[2] == OPCODE_LOGIN){
        return 0;
    }
    else{
       int token = message[4];//get the value of the token
       return token;
    }
}
//Function goes through and finds the session that matches the token
struct session * find_the_session_by_token(int token){
    if(token == 0){
        return NULL;
    }
    else {
        for(std::map<uint32_t, session *>::iterator it = list.begin(); it != list.end();it++){
            if(it->first == token){
                return it->second;
            }
        }
    }
    cout<<"THERE HAS BEEN AN ERROR LOOKING FOR  A SESSION\n";
    return NULL;
}
//Function takes the received message and returns the event that is occuring
int parse_the_event_from_the_datagram(char* datagram){
    if(datagram[2] == OPCODE_LOGIN){//if we have a login request
        return EVENT_LOGIN;
    } 
    else if(datagram[2] == OPCODE_POST){
        return EVENT_NET_POST;
    }
    else if(datagram[2] == OPCODE_LOGOUT){
        return EVENT_NET_LOGOUT;
    }
    else if(datagram[2] == OPCODE_SUBSCRIBE_REQUEST)
        return EVENT_NET_SUBSCRIBE;
    else if(datagram[2] == OPCODE_UNSUBSCRIBE_REQUEST)
        return EVENT_NET_UNSUBSCRIBE;
    else if(datagram[2] == OPCODE_FORWARD_ACK)
        return EVENT_NET_FORWARD_ACK;
    else if(datagram[2] == OPCODE_RETRIEVE)
        return EVENT_NET_RETRIEVE;
    else if(datagram[2] == OPCODE_SPECIAL_USER)
        return EVENT_GENEERATE_INVALID_MESSAGE;
    else if(datagram[2] == OPCODE_RESET)
        return EVENT_RESET;
    return -1;
}
//Function checks the list of its members and returns if it exists
int check_id_password(char* user,char* password){
    
     for(std::map<uint32_t, session *>::iterator it = list.begin() ; it != list.end() ;it++){
      if(strncmp(it->second->client_id,user,strlen(it->second->client_id)) == 0 && strncmp(it->second->password,password,strlen(password)) == 0){
          return 1;
      }
    }
    return -1;
}
//This function finds the current session , removes it from the list then generates a new key for it , adds it back and returns the token
uint32_t generate_a_random_token(struct session* current){
    srand(time(NULL));//get a seed
    uint32_t token = rand() % 127;
   struct session* temp;
   std::map<uint32_t, session *>::iterator iterator;
     for(std::map<uint32_t, session *>::iterator it = list.begin() ; it != list.end() ;it++){
       if(strncmp(it->second->client_id,current->client_id,strlen(it->second->client_id)) == 0 ){//if we find the session
           temp = it->second;//store the session
           iterator = it;
       }
    }
    list.erase(iterator);
   std::map<uint32_t, session *>::iterator it;
   it = list.find(token);//search for token
   while(it != list.end()){//if we have a token in list alreadt generate a new one and try again
        token = rand() % 127 + 1;
        it = list.find(token);
   }
   std::pair<uint32_t,session*> pair;
   pair.first = token;
   pair.second = temp;
   list.insert(pair);
   return token;

}
//Finds client in std::map
struct session* find_this_client_in_the_session_array(char* user){
    for(std::map<uint32_t, session *>::iterator it = list.begin() ; it != list.end() ;it++){
        if(strncmp(it->second->client_id,user,strlen(it->second->client_id)) == 0 ){
          return it->second;
      }
    }
    return NULL;
}
time_t right_now(){
    return time(NULL);
}

/*Hard code a few sessions */
void hard_code_sessions(){
    
    session* temp = new session;
    temp->password = new char[100];//initialize the password
    strlcpy(temp->password,"test",sizeof("test"));
    temp->state = STATE_OFFLINE;
    temp->token = 1;
    strlcpy(temp->client_id,"test",sizeof("test"));
    pair<uint32_t,session*> pair;
    pair.first = temp->token;
    pair.second = temp;
    list.insert(pair);
    ///////////////////////////////////////////////////////////////
   session* s2 = new session;
    s2->password = new char[100];
    strlcpy(s2->password,"1234",sizeof("1234"));
    s2->state = STATE_OFFLINE;
    s2->token = 2;
    strlcpy(s2->client_id,"Erick",sizeof("Erick"));
    std::pair<uint32_t,session*> p2;
    p2.first = s2->token;
    p2.second = s2;
    list.insert(p2);
    ////////////////////////////////////////////////////////////////
    session* s3 = new session;
    s3->password = new char[100];
    strlcpy(s3->password,"password",sizeof("password"));
    s3->state = STATE_OFFLINE;
    s3->token = 3;
    strlcpy(s3->client_id,"Siriah",sizeof("Siriah"));
    std::pair<uint32_t,session*>p3;
    p3.first = s3->token;
    p3.second = s3;
    list.insert(p3);

}

/*utility function that prints map contents*/
void print_map(){
    cout << "LIST SIZE = " <<list.size() << endl;
    for(std::map<uint32_t, session *>::iterator it = list.begin() ; it != list.end() ;it++){
        cout << "item token " << it->first <<endl;
        cout<< "item value = " << it->second->client_id << endl;
        cout << "Password = " << it->second->password<<endl;
    }
}

bool remove_client_in_cs_map(char* client, struct session* cs){
   
   std::map<char*, session *>::iterator iterator = cs->subscriptions.end();
    for(std::map<char*, session *>::iterator it  = cs->subscriptions.begin();
    it != cs->subscriptions.end(); it++ ){
        if(strncmp(it->second->client_id,client,strlen(it->second->client_id)) == 0){
            iterator = it;
        }
    }
    if(iterator != cs->subscriptions.end()){
        cs->subscriptions.erase(iterator);
        return true;
    }
    return false;;
}

void check_liveliness(){
    time_t current_time = time(NULL);
    for(std::map<uint32_t, session *>::iterator it = list.begin() ; it != list.end() ;it++){
        if(current_time - it->second->last_time  > 60 && it->second->state != STATE_OFFLINE){
            it->second->state = STATE_OFFLINE;
        }
    }
}
