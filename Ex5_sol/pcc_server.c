#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>

unsigned long int pcc_total[95];
_Bool isCWorking = false;
_Bool isHWorking = false;

void user_handler(){
	if(!isCWorking){
		for(int i=0;i<95;i++){
			printf("char ’%c’ : %lu times\n", i+32,pcc_total[i]);
		}
		exit(0);
	}
	isHWorking = true;
	
}


int main(int argc, char *argv[]){
	
	unsigned long int sumChar=0;
	unsigned long int printableChar=0;
	int listenfd  = -1;
	int connfd    = -1;
	char buff[1];
	
	struct sockaddr_in serv_addr;
  	struct sockaddr_in peer_addr;
	socklen_t addrsize = sizeof(struct sockaddr_in );
	listenfd = socket( AF_INET, SOCK_STREAM, 0 );
	memset( &serv_addr, 0, addrsize );
	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  	serv_addr.sin_port = htons(atoi(argv[1]));
	
	struct sigaction user_act;
	user_act.sa_handler = user_handler;
	sigaction(SIGINT, &user_act, NULL);
	int flag = 1;  
    if (-1 == setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {  
        perror("setsockopt fail");  
    }
	if( 0 != bind( listenfd,(struct sockaddr*) &serv_addr,addrsize ) ){
    	printf("\n Error : Bind Failed. \n");
    	exit(1);
  	}

  	if( 0 != listen( listenfd, 10 ) ){
    	printf("\n Error : Listen Failed. \n");
    	return 1;
  	}
  	
  	while(1){
   		connfd = accept( listenfd,(struct sockaddr*) &peer_addr,&addrsize);
  		if( connfd < 0 )
    	{
      		printf("\n Error : Accept Failed. %s \n", strerror(errno));
      		exit(1);
    	}
    	isCWorking = true;
    	int read_ = read(connfd,&sumChar,sizeof(sumChar));
    	if( read_ <= 0 ){
      		exit(1);
      	}
    	unsigned long int _sumChar = ntohl((unsigned long int)sumChar);
    	while(_sumChar>0){
    		int readChar = read(connfd,(void*)buff,sizeof(buff));
    		if( readChar <= 0 ){
      			exit(1);
      		}
      		if((buff[0])>=32 && (buff[0])<=126){
      			pcc_total[(buff[0])-32]++;
      			printableChar++;	
      		}
      		_sumChar--;
    	}
    	unsigned long int _printableChar = htonl(printableChar);
    	if(write(connfd,&_printableChar,sizeof(_printableChar))==-1){
    		exit(1);
    	}
    	printableChar=0;
    	isCWorking = false;
    	close(connfd);
    	if(isHWorking){
    	close(listenfd);
    		
    		user_handler();
    	}
  	}
  	
	return 0;
}
