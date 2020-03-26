/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  Thursday 26 March 2020 07:19:24  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Ayush Bajaj 
 *         Company:  FH Südwestfalen, Iserlohn
 *
 * =====================================================================================
 */

#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<string.h>

#define SOCK_NAME "/tmp/DemoSocket"
#define MAX_BACKLOG_CONN_REQS 21 
#define MAX_BUFFER_SIZE 128

void handle_error(char* msg){

 do { 
        
        perror(msg);
        exit(EXIT_FAILURE);
    
    
}while(0);

}

int main(){

    //System calls(made by this process to linux kernel) -- socket, bind, listen, accept, read, write etc

    //Delete the master socket if still being used by the server process that terminated with failure, signal etc 
    //Prevents bind failure: addr already in user
    unlink(SOCK_NAME);

   
    //Service Runs
    errno=0; 

    //Create Master Socket
    int master_conn_socket;
    master_conn_socket = socket(AF_UNIX, SOCK_STREAM, 0);    
    if(master_conn_socket == -1)
        handle_error("socket call failed");
    printf("Master socket created\n");

#if 0
    #define UNIX_PATH_MAX    108

    struct sockaddr_un {
        sa_family_t sun_family;               /* AF_UNIX */
        char        sun_path[UNIX_PATH_MAX];  /* pathname */
    };
#endif

    //struct sockaddr_un name; 
    struct sockaddr_un myAddr;
    memset(&myAddr, 0, sizeof(struct sockaddr_un));   /*Clear the structure*/

    myAddr.sun_family = AF_UNIX;
    strncpy(myAddr.sun_path, SOCK_NAME, sizeof(myAddr.sun_path) - 1);

    printf("sizeof(myAddr.sun_path) is %d\n",sizeof(myAddr.sun_path));

    //Instruction to the OS to send any msg destined for a socket with name /tmp/demosocket to this process
    if(bind(master_conn_socket, (const struct sockaddr *) &myAddr, sizeof(struct sockaddr_un)) == -1) //why typecast?
        handle_error("bind failed");
    
    printf("Bind success\n");
    
    //Like opening the shutter of some service shop //Shop is now open to receive customers
    //marks the master socket as passive/or simply as listener(used to accept incoming connections)

    int rt = listen(master_conn_socket, MAX_BACKLOG_CONN_REQS);
    if(rt == -1)
           handle_error("listen failed");

    printf("Listen success...Server is now listening for connection requests\n");

    char databuf[MAX_BUFFER_SIZE];

    while(1){
        
        //fd is just a positive integer
        int client_socket_fd;

        //blocking call, corresponding client API call is connect()
        printf("waiting to accept connection request from client\n");
        client_socket_fd = accept(master_conn_socket, NULL, NULL);     
        if(client_socket_fd == -1)
            handle_error("accept failed");
           
        printf("connection accepted from client, fd assigned is %d\n",client_socket_fd);
    
    
        while(1){   
           
           
         memset(databuf, 0 ,sizeof(databuf));
        //How much can be read in a single operation is specified by the variable size_t SSIZE_MAX
        //So, the count must not exceed SSIZE_MAX
        //read count bytes from fd to buffer
        printf("waiting for service request(data from the client)\n");

        int count = read(client_socket_fd, databuf, MAX_BUFFER_SIZE);
        //0 indicates EOF, -1 means error
        if(count == -1)
               handle_error("read failed");
  
        

        printf("Count of bytes read is %d, and actual data read is %s\n",count,databuf);
        //write to fd
        //memset(databuf, 0, MAX_BUFFER_SIZE);
        int count_w = write(client_socket_fd, databuf, MAX_BUFFER_SIZE);
        if(count_w == -1)
            handle_error("write failed");
        
        
        if(strcasecmp(databuf, "end") == 0){
            printf("Close request received from client\n");      
            break;
        }

        }

        //close the client data socket
        close(client_socket_fd);

    }

    //Close the master socket
    close(master_conn_socket);
    
    //Server must free all the resources used by it on termination
    
    unlink(SOCK_NAME);
    exit(EXIT_SUCCESS); 



    return 0;
        
    
    
}
