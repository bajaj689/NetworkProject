#include<stdio.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <stdlib.h> //to declare exit
#include <string.h> //to declare strlen, memset

#define MAX_MSG_COUNT 5
#define BUFFER_SIZE 256
//mqd_t mq_open(const char *name, int oflag); //oflags means operation flags
//mqd_t mq_open(const char *name, int oflag, mode_t mode,
//		struct mq_attr *attr);

void handleError(char* msg){

	perror(msg);
	exit(EXIT_FAILURE);

}


int main(int argc, char** argv){

	if(argc < 2){
		//error
		printf("Pls specify the msg queue name\n");
		exit(EXIT_FAILURE);
	}

	printf("argv[1] is %s\n",argv[1]);
	//1. Open/Create a mq
	mqd_t mq_fd = mq_open( argv[1], O_WRONLY | O_CREAT , 0, NULL); //If attr is NULL, then the queue is created with implementation-defined default attributes.
	if(mq_fd == -1)
		handleError("mq_open");

	//mq_getattr can be used to get the attr details
	struct mq_attr attrset;
	//int mq_getattr(mqd_t mqdes, struct mq_attr *attr);
	if(mq_getattr(mq_fd, &attrset) == -1)
		handleError("mq_getattr");

	printf("Queue attributes read are: Max count: %d, Max size: %d\n",attrset.mq_maxmsg, attrset.mq_msgsize);
	
	//2. Send msg to the queue
	char buffer[BUFFER_SIZE] = {0}; //Max allowed by is 8192
	int msg_count = MAX_MSG_COUNT;
	while(msg_count--){
	
		memset(buffer, 0, BUFFER_SIZE);
		printf("Enter the message to be queued(Max size: %d)\n",BUFFER_SIZE);
		//scanf("%s",buffer);
		fgets(buffer, BUFFER_SIZE, stdin); //To read from fd to buffer
		//unblocks when msg is available for reading on the queue 
		if(strcmp(buffer,"end")){

			printf("Lenght of message queued is %d\n",strlen(buffer)); //actual size of the message on the queue will still be mq_msgsize
			if(mq_send(mq_fd, buffer, strlen(buffer) + 1, 0) == -1)
				handleError("mq_send");
		
			continue;
		}

		break;	
	}
	
	//3. Close the queue
	if(mq_close(mq_fd) == -1)//closes the fd for the process //basically association of process to fd is deleted
		handleError("mq_close");

	//We dont unlink here : Unlinking remove the msq queue name itself// this causes receiving process's msg queue fd to point to deleted name.	
	exit(EXIT_SUCCESS);	


}
