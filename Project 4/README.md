# CSE 434 Assignment 4


##  Description

This is a client-server applications that was created to practice more advanced socket programming as well as working with a publisher - subscriber type of design pattern

## Reqirements
 Compile the client as well as the server on a POSIX machine using the g++ compiler with the following commands
 ```
 g++ -o Client Client.cpp
 ```
 ```
 g++ -o Server Server.cpp
 ```
 Then using 2 seperate windows run both the server program first then the client

 ```bash
 ./Server
 ```
 then
 ```
 ./Client
 ```
 

## Usage

 #### On the Client program you can use the following commands
 ```
 login#client_a&password
 ```

 where **client_a** is the client ID and **password** is the password of this client

 Server will respond with one of 2 messages

 1) login_ack#successful 

 2) login_ack#failed 
   
 #### Onced Logged in you can use any of the following 
 ```
 subscribe#client_b 
 ```
 This subscribes you to the **client_b** if the client exists this is similar to 'following a user' similarly you can type

 ```
 unsubscribe#client_b
 ```
 this unsubscribes you from the client assuming they exist and you were previously subscribed to them

 the different responses that you can receive on the server side are for these commands are
 
 1) subscribe_ack#successful
 2) subscribe_ack#failed
 3) unsubscribe_ack#successful​
 4) ​unsubscribe_ack#failed​

`Note: Any of the following commands done before a successful login will return a error#must_login_first from the server`

A  command you can do from the client side is 
```
post#some_text
```
where **some_text** is posted to the server and if any clients are subscribers to this person as "signed in" they will receive a real time notification

server response for the post client are
1) post_ack#successful
2) post_ack_failed

The subscribers will receive the following 

```
<client_a>some_text
```

where **client_a** is the client ID of the person who posted the text

As a subscriber you can type

```
retrieve#n
```
Where n is  the n most recent messages that were posted from clients that the user is subscribed to which will return 

```
<client_a>some_text
```
which is the same as the real time message feed

Finally you can type the line

```
logout#
```

to instruct the client to logout , it will send a logout message to the sever which will return

1) logout_ack#successful

once logged out client will not receive any messages from the server. 

Finally something to note is the server periodically checks the 'liveliness' of the clients that are currently logged in. If they don't type a command within the time period specified then the server automatically logs the client out and they will have to login again to use the commands



# Issues
 When Compiling ensure that you don't compile with the C++ 11 flag as we get a destination unreachable error