#include<stdio.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <unistd.h>
#include <stdlib.h>
//mqd_t mq_open(const char *name, int oflag); //oflags means operation flags
//mqd_t mq_open(const char *name, int oflag, mode_t mode,
//		struct mq_attr *attr);

#define PERMISSIONS 0660
#define BUFFER_SIZE 256

void handleError(char* msg){

	perror(msg);
	exit(EXIT_FAILURE);

}


int main(int argc, char** argv){

	if(argc < 2){
		//error
		printf("Pls specify the msg queue name in the format </queueName> \n");
		exit(EXIT_FAILURE);
	}

	printf("argv[1] is %s\n",argv[1]);

#if 0
	struct mq_attr {
               long mq_flags;       /* Flags: 0 or O_NONBLOCK */
               long mq_maxmsg;      /* Max. # of messages on queue */
               long mq_msgsize;     /* Max. message size (bytes) */
               long mq_curmsgs;     /* # of messages currently in queue */
           };
#endif

	//Check files in  /proc/sys/fs/mqueue/ for all values
	//Set the attributes
	struct mq_attr attrset;
	attrset.mq_flags = 0;
	attrset.mq_maxmsg = 10;
	attrset.mq_msgsize = 256;
	attrset.mq_curmsgs = 0;

	//1. Open/Create a mq
	mqd_t mq_fd = mq_open( argv[1], O_RDONLY | O_CREAT, PERMISSIONS, &attrset);
	if(mq_fd == -1)
		handleError("mq_open");

	printf("Queue created/opened successfully with fd %d\n",mq_fd);
	//2. Receive msg to the queue
	char buffer[BUFFER_SIZE] = {0}; //Max allowed by is 8192
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(mq_fd, &readfds);
	
		

	while(1){


		printf("Blocked on select() sys call\n");
		if(select(mq_fd + 1, &readfds, NULL, NULL, NULL) == -1)
			handleError("select");

		if(FD_ISSET(mq_fd,&readfds))
		{
			if(mq_receive(mq_fd, buffer, BUFFER_SIZE, NULL) == -1)
				handleError("mq_receive");

			printf("Msg read from the queue is %s\n",buffer);
		}
	}


	printf("Closing the fd\n");
	//3. Close the queue
	if(mq_close(mq_fd) == -1)//closes the fd
		handleError("mq_close");
	
	if(mq_unlink(argv[1]) == -1) //removes the name
		handleError("mq_unlink");

	exit(EXIT_SUCCESS);	
}
