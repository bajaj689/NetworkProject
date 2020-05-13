

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define SHM_PERMISSIONS 0660
#define SHM_OBJ_SIZE 256
//man shm_overview
//
//
void handleError(char * msg){

	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv){

	//int shm_open(const char *name, int oflag, mode_t mode); //mode is set while creating the shm object
	//1.Open/Create a SHM kernel object

	if(argc < 2){

		printf("Please enter the name of the SHM in format </name>\n");
		exit(EXIT_FAILURE);

	}


	int shm_fd = shm_open(argv[1], O_CREAT | O_RDONLY,  SHM_PERMISSIONS); //size zero if newly created
	if(shm_fd == -1)
	{
		handleError("shm_open");
	}

	printf("SHM create/open success\n");

	//2.Extend the VAS using mmap

	//void *mmap(void *addr, size_t length, int prot, int flags,
	//                int fd, off_t offset);

	void* shm_addr = NULL;
	if((shm_addr = mmap(NULL, SHM_OBJ_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
	{
		handleError("mmap");
	}

	//use fstat to get details about the shm object
	printf("mmap success: VAS extended\n");

	//4.Access(Read/Write)
	char data[SHM_OBJ_SIZE] = {0};
	memcpy(data, shm_addr, SHM_OBJ_SIZE);

	printf("Data read from SHM is : %s\n",data);

	if(munmap(shm_addr,SHM_OBJ_SIZE) == -1)
		handleError("munmap");
	//deletes the mappings for the specified address range, and causes further references to addresses within the range  to  generate  invalid memory  references.  The region is also automatically unmapped when the process is terminated.  On the other hand, closing the file descriptor does not unmap the region.

	//5.Close the SHM kernel object
	if(close(shm_fd) == -1)
		handleError("close");

	return 0;

}

