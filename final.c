#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>  
#include <signal.h>

/***************************************************************************
 *  CIS 452 Project 2
 *  Token Ring Communication
 *
 *  This program is a token ring simmulation. The user enters a string
 *  and the program sends the string from process one to process x which
 *  is specified by the user. THe maximum processes that can be requested
 *  works according to MAX which is currently set to 100.
 *
 *  @author: Kehlsey Lewis
 *  @version: Fall 2018
 ****************************************************************************/ 


void createRing(int num, int total, int* pipeList[]);

#define READ 0 //for reading end of pipe
#define WRITE 1 //for writing end of pipe
#define MAX 100 //maximum number of processes to be specified

void sigUsrHandler(int signNum);//funtion for graceful parent process termination
void sigChildHandler(int signNum);//funtion for graceful child process termination

int main(void) 
{ 
	//for parent process termination
	signal(SIGINT, sigUsrHandler);
	int size;
	int status;

	//user specifies the size of the ring
	printf("Enter the size of the token ring: ");
	scanf("%d", &size);

	//creates array to hold pipes according to ring size
	int* pipes[size];

	//initializing all pipes
	for (int i = 0; i < size; ++i){
		int Apipe[2];
		pipes[i] = Apipe;
		if(pipe(pipes[i]) == -1)
			printf("Error Piping %d", i);
	}

	//loops the process of sending a string
	while (1){
		fflush(stdout);
		fflush(stdin);
		char input[MAX]; //reinitializes input each time to clear

		printf("Enter a message to send: \n");
		scanf("%s", input);

		write(pipes[1][WRITE], input, strlen(input) + 1 );//sends to first pipe
		createRing(0, size, pipes); //creates token ring and sends message
		wait(&status); //wait for ring to complete
	}
	return 0;
}

/*  
 *  This is a recursive calling function that sends a message
 *  down the line of processes. Count keeps track of which process is
 *  currently being used and total is so the function knows when to stop.
 *  the pipe list holds all of the pipes for reading and writing.
 *
 */ 

void createRing(int count, int total, int* pipeList[]){
	signal(SIGINT, sigUsrHandler); //for parent process termination
	
	sleep(3);//sleep for three seconds for watching purposes

	if(count < total){ //stop recursive call once total is reached
		pid_t  pid;
		int status;
		char parentMes[MAX];//string to hold the parent process read from pipe
		char msg[MAX]; //string to hold the child process read from pipe

		//forks a new process
		if ((pid = fork()) < 0){     
			printf("fork failure\n");
			exit(1);
		}

     	//child process
		else if(pid == 0) {	
			signal(SIGINT, sigChildHandler); //for child process termination
			createRing(count+1, total, pipeList);//recursive call for next process

			if(count == total-1){ //once last process is reached

				//reads from current pipe into msg variable
				read(pipeList[count][READ], msg, sizeof(msg) + 1 );
				printf("Last p%d (%ld) received: %s. \n", count+1, (long) getpid(), msg);
			 }
			 exit(0); //exits once child is done
		}

    	//parent process
		else{
			signal(SIGINT, sigChildHandler); //for parent termination

			if(count != total-1){

				//reads from current pipe into parentmes variable			
				read(pipeList[count][READ], parentMes, sizeof(parentmes));
				printf("%s was received by p%d (%ld).\n", parentmes, count+1, (long) getpid());
				
				//writes to the next pipe
				write(pipeList[count+1][WRITE], parentmes, strlen(parentmes) + 1 );
			}		
		}	
		wait(&status);
	}
}
 /* 
 *  Used for graceful termination by a parent process
 */ 
void sigUsrHandler(int signNum) { 
	if (signNum == SIGINT){
		printf("\nControl-c registered.\n");
		printf("Exiting...\n");
		exit(0);
	}
}

 /* 
 *  Used for graceful termination by a child process
 */ 
void sigChildHandler(int signNum) { 
	exit(0);
}