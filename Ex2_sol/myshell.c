#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>


void print_cwd();
int check_pipe(int count,char** arglist);
void pipe_s(int index, char** arglist);


int prepare(void){
	print_cwd();
	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	return 0;
}


int process_arglist(int count, char** arglist){
	int index;
	index = check_pipe(count,arglist);//check if has a |
	if(index > 0){// if has, we sent the arglist and the pipe's index to func pipe_s()
		pipe_s(index, arglist);    
	}else if(strcmp(arglist[count-1],"&")==0){//check if has a &
		arglist[count-1]=NULL;
		pid_t pid = fork();
		if (pid == -1) {
    		perror("Failed forking");
       		exit(EXIT_FAILURE);
		}else if(pid == 0){
			if(execvp(arglist[0],arglist)!=0){
				perror("Failed forking");
       			exit(EXIT_FAILURE);
			}
		}
	}else{//if we are here so we didn't & or |
		pid_t pid = fork();
		if (pid == -1) {
    		perror("Failed forking");
       		exit(EXIT_FAILURE);
		}else if(pid == 0){
			signal(SIGINT,SIG_DFL);
			if(execvp(arglist[0],arglist)!=0){
				perror("Failed forking");
       			exit(EXIT_FAILURE);
			}
		} else {
        	int status;
			waitpid(pid,&status,0);
        }
	}
	print_cwd();
    return 1;
}	


void pipe_s(int index, char** arglist){
		int s;
		int pipe_fd[2]; 
 		pid_t pid1,pid2;
 		if (pipe(pipe_fd)==-1){
 			perror("pipe");
 			exit(EXIT_FAILURE);
 			}
 		if ((pid1=fork()) == 0){
 				arglist[index]=NULL;
 				close(1);
 				dup2(pipe_fd[1],1);
 				execvp(arglist[0],arglist);
 		}else{		
 			close(pipe_fd[1]);
 			if  ((pid2=fork())==0){
 			close(0);
 			dup2(pipe_fd[0],0);
 			execvp(arglist[index+1],&arglist[index+1]);
 			}
 		} 
 		wait(&s);wait(&s);
}


void print_cwd(){
	char cwd[1024];
	printf("\e[1;32m%s\033[0m:\033[1;31m~%s\033[0m$ ",getenv("USER"),getcwd(cwd,sizeof(cwd)));
}


int check_pipe(int count,char** arglist){
	for(int i=0;i<count;i++){//check if we have a pipe in arglist
		if(strcmp(arglist[i],"|")==0){
			return i;
		}
	}
	return 0;
}


int finalize(void){
	 
	return 0;
}
