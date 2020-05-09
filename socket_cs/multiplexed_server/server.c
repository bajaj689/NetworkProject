#include<stdio.h>
#include<sys/types.h>          /* See NOTES */
#include<sys/socket.h>
#include<stdlib.h>
#include<sys/un.h>
#include<unistd.h>

#include<errno.h>
#include<string.h>



#define SOCKET_PATH "/tmp/MasterUDSSocket"
#define MAX_BACKLOG 50
#define BUFFER_SIZE 128
#define MAX_CLIENTS_TO_SERVE 31

char buffer[BUFFER_SIZE];
int result[MAX_CLIENTS_TO_SERVE];
int backup_fd_set[MAX_CLIENTS_TO_SERVE];

static void init_backup_fd_set(){
	int i = 0;
	for(; i < MAX_CLIENTS_TO_SERVE; i++){
		backup_fd_set[i] = -1;
	}
}

static void add_to_backup_fd_set(int fd){

	int i = 0;
	for(; i < MAX_CLIENTS_TO_SERVE; i++){
		if(backup_fd_set[i] == -1){
			backup_fd_set[i] = fd;
			printf("backup_fd_set[i] is %d\n",backup_fd_set[i]);
			break;
		}
	}
}

static void remove_from_backup_fd_set(int fd){

	int i = 0;
	for(; i < MAX_CLIENTS_TO_SERVE; i++){
		
		if(backup_fd_set[i] == fd){
			backup_fd_set[i] = -1;
			break;
		}
	
	}
}

static void refresh_fd_set(fd_set* readfd_set_ptr){
	
	FD_ZERO(readfd_set_ptr);
	int i = 0;
	for(; i < MAX_CLIENTS_TO_SERVE; i++){
	
		if(backup_fd_set[i] != -1)
			FD_SET(backup_fd_set[i],readfd_set_ptr);

	}

}

static int getmaxfd(){


	int max = -1;
	int i = 0;
	for(; i < MAX_CLIENTS_TO_SERVE; i++){
		
		if(backup_fd_set[i] > max)
			max = backup_fd_set[i];
	}

	return max;
}


void handleError(char* msg){

	perror(msg);
	exit(EXIT_FAILURE);

}


int main(){


//Server starts
	
	unlink(SOCKET_PATH);

	//int socket(int domain, int type, int protocol);
	int master_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(master_fd == -1)
	      handleError("socket");

	int ret = -1;
	struct sockaddr_un myAddr;
	memset(&myAddr, 0, sizeof(struct sockaddr_un));/*Clear the structure*/
#if 0
#define UNIX_PATH_MAX    108

    struct sockaddr_un {
        sa_family_t sun_family;               /* AF_UNIX */
        char        sun_path[UNIX_PATH_MAX];  /* pathname */
    };
#endif

	myAddr.sun_family = AF_UNIX;
	strncpy(myAddr.sun_path, SOCKET_PATH, sizeof(myAddr.sun_path) - 1); 
	myAddr.sun_path[sizeof(myAddr.sun_path) - 1] = '\0';

	if(bind(master_fd, ( struct sockaddr *)&myAddr, sizeof(struct sockaddr_un)) == -1)
		handleError("bind");


	if(listen(master_fd, MAX_BACKLOG) == -1)
		handleError("listen");

	
	//=============================//
	fd_set readfds;
	FD_ZERO(&readfds);/* Clear the fd_set structure*/
	
	init_backup_fd_set();	
	add_to_backup_fd_set(master_fd);	

	while(1){

	refresh_fd_set(&readfds);

	printf("select: Watching all fds, getmaxfd(): [%d]\n",getmaxfd());

	if(select(getmaxfd() + 1, &readfds, NULL, NULL, NULL) == -1)  //Watch the fds in readfds //watch all fds from 0 to the maxfd and not FD_SETSIZE
		handleError("select");

	//unblocks when one or more fds get ready
	if(FD_ISSET(master_fd, &readfds))
	{
		
		printf("New connection recieved recvd, accept the connection\n");
		//invoke accept system call
		int new_client_fd = accept(master_fd, NULL, NULL);
	
		if(new_client_fd == -1)	
			handleError("accept");

		printf("Connection accepted from client, fd assigned is %d\n",new_client_fd);	
		add_to_backup_fd_set(new_client_fd);
		
	}
	else if(FD_ISSET(0, &readfds)){  //fd 0 activated means input available on stdin
		
		//input received from stdin
		memset(buffer, 0, BUFFER_SIZE);
		if(read(0, buffer, BUFFER_SIZE) == -1)
			handleError("stdin read");
		printf("Data read from stdin is %s\n",buffer);
	
	}
	else
	{
		
		//One of the client fds was activated //Identify which one
		int i = 0;
		for(; i < MAX_CLIENTS_TO_SERVE ; i++){
		
			if(FD_ISSET(backup_fd_set[i], &readfds)){

				int client_fd = backup_fd_set[i];
				int data = 0;
				//data received //read it
				if(read(client_fd, &data, sizeof(int)) == -1)
					handleError("read");
					
				printf("CLIENT : %d , DATA : %d \n",client_fd, data);

				if(data == 0)	//Return the sum to the client and close the connection and remove the client fd from fd_set
				{
					printf("Final result is %d\n", result[i]);	
					if(write(client_fd, result+i, sizeof(int)) == -1)
						handleError("write");
					
					close(client_fd);
					result[i] = 0;
					remove_from_backup_fd_set(client_fd);
					continue; //go to select and block	
					
				}
				else
				{
				
					result[i] += data;
					printf("Interim result is %d\n", result[i]);	
					break;
					
				}	
			}
		}
	}/* go to select and block*/

	}//

	close(master_fd);
	unlink(SOCKET_PATH);
	remove_from_backup_fd_set(master_fd);

	exit(EXIT_SUCCESS);
	
}
