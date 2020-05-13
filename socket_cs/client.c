/*
 * =====================================================================================
 *
 *       Filename:  client.c
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
#define MAX_BUFFER_SIZE 128

void handle_error(char* msg){

 do { 
        
        perror(msg);
        exit(EXIT_FAILURE);
    
    
}while(0);

}

int main(){

	//Service Runs
	errno=0; 

	//Create Master Socket==========================================================================
	int master_conn_socket;
	master_conn_socket = socket(AF_UNIX, SOCK_STREAM, 0);    
	if(master_conn_socket == -1)
		handle_error("socket call failed");
	printf("Master socket created\n");

	struct sockaddr_un myAddr;
	memset(&myAddr, 0, sizeof(struct sockaddr_un));   /*Clear the structure*/
	myAddr.sun_family = AF_UNIX;
	strncpy(myAddr.sun_path, SOCK_NAME, sizeof(myAddr.sun_path) - 1);

	printf("myAddr.sun_path is %s\n",myAddr.sun_path);

	//Make a connection request to the server ======================================================
	int ret = connect(master_conn_socket, (const struct sockaddr *) &myAddr, sizeof(struct sockaddr_un));
	if(ret == -1)
		handle_error("connect failed...server is down");


	int num = 0;
	char databuf[MAX_BUFFER_SIZE];
	do{

		printf("Enter the number:");
		scanf("%d",&num);

		//printf("Number read is %d\n",num);
		//Send data to server ============================================================================
		///printf("Sending data to server\n");
		int count_w = write(master_conn_socket, &num ,sizeof(int));
		if(count_w == -1)
			handle_error("write failed");

		//printf("No.of bytes sent is %d\n", count_w);
		//printf("num is %d\n",num);

	}while(num);


	printf("waiting for server result\n");
	//Read response from server =======================================================================
	//memset(databuf, 0, MAX_BUFFER_SIZE);
	int res = 0;
	int count = read(master_conn_socket, &res, sizeof(int));
	if(count == -1)
		handle_error("read failed");
	//printf("count of bytes read is %d and  data received from server is %s\n",count, databuf);
	printf("count of bytes read is %d and  data received from server is %d\n",count, res);
	printf("Closing the connection\n");
	//Close the master socket
	close(master_conn_socket);
	//Server must free all the resources used by it on termination

	exit(EXIT_SUCCESS); 

	return 0;
}
