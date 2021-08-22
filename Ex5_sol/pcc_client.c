#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#define _DEFAULT_SOURCE
#define _BSD_SOURCE



int main(int argc, char *argv[]){
	if(argc!=4){
		perror("Not enough arguments!\n");
		exit(1);
	}
	
	int  sockfd     = -1;
  	unsigned long int  bytes_read =  0;
  	char buff;
  	void* printable;
  	char recv_buff[1024];
  	struct sockaddr_in adress;
  	
  	memset(recv_buff, 0,sizeof(recv_buff));
  	
  	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  	{
    	perror("\n Error : Could not create socket \n");
    	exit(1);
  	}
  	
  	memset(&adress, 0, sizeof(adress));
  	
  	adress.sin_family = AF_INET;
  	adress.sin_port = htons(atoi(argv[2]));
  	adress.sin_addr.s_addr = inet_addr(argv[1]);
  	
  	if( connect(sockfd,(struct sockaddr*) &adress,sizeof(adress)) < 0){
    	perror("\n Error : Connect Failed. \n");
    	exit(1);
    }
    FILE* fg = fopen(argv[3],"r");
    if(fg==NULL){
    	perror("Error : Open Failde. \n");
    	exit(1);
    }
	int get_char = getc(fg);
	
	while(get_char!=EOF){
		bytes_read++;
		get_char = getc(fg);
		
	}
	rewind(fg);  
	 unsigned long int _bytes_read = htonl(bytes_read);
    int rev = write(sockfd,&_bytes_read,sizeof(_bytes_read));
    if(rev<0){
    	perror("Error : sent size.\n");
    	exit(1);
    }
	while(bytes_read>0){
	
		buff = getc(fg);
		if(write(sockfd,&buff,sizeof(buff))!=-1){
			bytes_read--;
		}
	}
	bytes_read = read(sockfd,&printable,sizeof(printable));
	if( bytes_read <= 0 ){
      		exit(1);
      	}
	unsigned long int _printable = ntohl((unsigned long int)printable);
	
	printf("# of printable characters: %lu\n",_printable);
	fclose(fg);
	close(sockfd);
	return 0;
}

