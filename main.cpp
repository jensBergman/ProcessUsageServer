/* 
 * File:   main.cpp
 * Author: jesse
 *
 * Created on den 31 december 2015, 13:01
 */

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// necessary networking headers
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// avoid zombie processes
#include <signal.h> 

using namespace std;

void dostuff(int);

/*
 * Server that start a new process for each connection
 */
int main(int argc, char** argv) {
    // The signal is ignored and the code execution will continue even if not meaningful. 
    __sighandler_t prev_handler = signal(SIGCHLD,SIG_IGN);
    if(prev_handler == SIG_ERR){cout << "unable to raise signal!" << endl; return 0;}
    // file descriptors
    int sockfd, newsockfd;
    int port = 9999;
    int pid; // process id
    // stores the address size of the client
    socklen_t client_length;
    
    // internet addresses
    struct sockaddr_in server_addr, client_addr;
    
    /*
     * AF_INET = Internet domain
     * SOCK_STREAM = TCP socket
     * 0 = let the operating choose the most appropiate protocol (i.e. 3rd arg = protocol)
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cout << "error opening socket!" << endl;
        return 0;
    }cout << "Socket opened!" << endl;
    
    /*
     * bzero() sets all values in a buffer to zeros
     * 1st arg: buffer
     * 2nd arg: buffer length
     */
    bzero((char *) &server_addr, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET; // network domain
    server_addr.sin_addr.s_addr = INADDR_ANY; // IP addr of machine
    server_addr.sin_port = htons(port); // define port number in network byte order
    
    if (bind(sockfd, (struct sockaddr *) &server_addr,sizeof(server_addr)) < 0){
        cout << "Failed to bind socket" << endl;
        close(sockfd);
        return 0;
    }cout << "Socket bound!" << endl;
    
    /*
     * The listen system call allows the process to listen on the socket for connections. 
     * The first argument is the socket file descriptor, and the second is the size of the backlog queue,
     *  i.e., the number of connections that can be waiting while the process is handling a particular connection. 
     * This should be set to 5, the maximum size permitted by most systems. 
     * If the first argument is a valid socket, this call cannot fail, and so the code doesn't check for errors. 
     */
    listen(sockfd,5);
    cout << "Waiting for clients ..." << endl;
    client_length = sizeof(client_addr);
    
    // accept many connections simultaneously
    while(true){
        // process is blocket until client connects, then it wakes up the process.
        newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_length);
        if(newsockfd < 0){
            cout << "Error when accepting client connection" << endl;
            close(sockfd);
            return 0;
        }cout << "Client connection accepted!" << endl;
        
        pid = fork();
        if(pid < 0){ cout << "error on fork" << endl; }
        else if(pid == 0){
            close(sockfd);
            dostuff(newsockfd);
            return 0;
        }
        else{
            close(newsockfd);
        }

        
    }
    close(sockfd);
    
    return 0;
}

void dostuff (int newsockfd)
{
    
    // contains the number of characters read or written
    int stream_length;
    // read-buffer
    char buffer[256];
    
    // read from client ------- //

    bzero(buffer, 256);
    // read stream from client
    stream_length = read(newsockfd, buffer, 255);

    if(stream_length < 0){
        cout << "Error reading from socket" << endl;
        close(newsockfd);
        return;
    }

    cout << "Client said: " << buffer << endl;

    // write to client ----------------//
    stream_length = write(newsockfd, "I got your message!", 19);
    if(stream_length < 0){
        cout << "Error, failed to write to client" << endl;
        close(newsockfd);
        return;
    }cout << "Responded to client!" << endl;

    close(newsockfd);
}