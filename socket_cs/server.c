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

#define SOCK_NAME "/tmp/MasterSocketfd"
#define MAX_BACKLOG_CONN_REQS 3 
#define MAX_BUFFER_SIZE 128

void handle_error(char* msg){

    do { 

        perror(msg);
        exit(EXIT_FAILURE);


    }while(0);

}

int main(){

    //System calls(made by this process to linux kernel) -- socket, bind, listen, accept, read, write, accept etc

    //Delete the master socket if still being used by the server process that terminated with failure, signal etc 
    //Prevents bind failure: addr already in use
    unlink(SOCK_NAME); //Instead use virtual namespace

    //Service Runs
    errno=0; 

    //Create Master Socket //This is just an endpoint for communication //It can be used as server(by doing bind, accept, listen) or as client(by doing connect)
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

    struct sockaddr_un myAddr;
    memset(&myAddr, 0, sizeof(struct sockaddr_un));   /*Clear the structure*/

    myAddr.sun_family = AF_UNIX;
    strncpy(myAddr.sun_path, SOCK_NAME, sizeof(myAddr.sun_path) - 1);

    printf("Master socket name is %s\n",myAddr.sun_path);

    //Instruction to the OS to send any msg destined for a socket with name /tmp/demosocket to this process
    if(bind(master_conn_socket, (const struct sockaddr *) &myAddr, sizeof(struct sockaddr_un)) == -1) //why typecast?
        handle_error("bind failed");

    printf("Bind success\n");

    //Like opening the shutter of some service shop //Shop is now open to receive customers
    //marks the master socket as passive/or simply as listener(used to accept incoming connections)

    int rt = listen(master_conn_socket, MAX_BACKLOG_CONN_REQS);
    if(rt == -1)
        handle_error("listen failed");

    printf("Listen success\n");
    //Listen success...Coverts socket to passsive from active. It cannot initiate connection requests. It can only listen for connection requests
    char databuf[MAX_BUFFER_SIZE];

    while(1){

        //fd is just a positive integer
        int client_socket_fd;

        //blocking call, corresponding client API call is connect()
        printf("waiting to accept connection request from client\n");
        client_socket_fd = accept(master_conn_socket, NULL, NULL);//Who is next in the queue? 
        if(client_socket_fd == -1)
            handle_error("accept failed");

        printf("connection accepted from client, fd assigned is %d\n",client_socket_fd);

	//Handling the connection
	int result = 0;
        int temp_num = 0;
	int countOfBytesRead = 0;
        while(1){   


            //memset(databuf, 0 ,sizeof(databuf));
            //How much can be read in a single operation is specified by the variable size_t SSIZE_MAX
            //So, the count must not exceed SSIZE_MAX
            //read count bytes from fd to buffer
            printf("Waiting for service request(data from the client)\n");

            //int count = read(client_socket_fd, databuf, MAX_BUFFER_SIZE);
            countOfBytesRead = read(client_socket_fd, &temp_num, sizeof(int));
            //0 indicates EOF, -1 means error
            if(countOfBytesRead == -1){

                handle_error("read failed");
		close(client_socket_fd);
		close(master_conn_socket);

	    }

	    if(countOfBytesRead == 0){ //client exited
		
		close(client_socket_fd);	
		result = 0;
		break;
	    }	
            //printf("Count of bytes read is %d, and actual data read is %s\n",count,databuf);
            printf("Count of bytes read is %d and data read is %d\n",countOfBytesRead, temp_num);

	    //memcpy(&temp_num, databuf, sizeof(int));
	    result += temp_num;
	    if( temp_num == 0 )
		break;

        }

	if(countOfBytesRead == 0)
		continue; //go to accept() and block again

	//ssize_t write(int fd, const void *buf, size_t count);
	//Send result back to client
	printf("Final result is %d\n",result);
	int ret = write(client_socket_fd, &result, sizeof(int));
	if(ret == -1)
		handle_error("write failed");
	printf("No. of bytes written is %d\n",ret);

        printf("OUT OF INNER WHILE LOOP\n");
        //close the client data socket
        close(client_socket_fd);

    }

    //Close the master socket
    close(master_conn_socket);  //Abstract socket path names are automatically removed on closing the socket. If the physical path name is used for socket, then its necessary to unlink it
    //To create an abstract binding, we specify the first byte of the sun_path field as a null byte (\0)

    //Server must free all the resources used by it on termination

    unlink(SOCK_NAME);
    exit(EXIT_SUCCESS); 



    return 0;



}
