#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <fnmatch.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAX_THREADS 100
pthread_t threads[MAX_THREADS];
int counter = 0;
int threads_counter = 0;
size_t exit_error = 0;
size_t num_of_threads;
char* search_term;
pthread_mutex_t  lock;
pthread_cond_t not_empty;


typedef struct Node
{
	char* dir;
	struct Node* next;
}Node;

typedef struct Queue
{
	Node* head;
	Node* tail;
}Queue;

Queue queue;

//create_node//

Node* create_node(char* buff) 
{ 
    Node* node = (Node*)malloc(sizeof(Node));
    node->dir = (char*)malloc(strlen(buff)*sizeof(char));
    strcpy(node->dir,buff);
    node->next = NULL; 
    return node; 
} 


//empty//

_Bool empty(){
	return !queue.head;
}


//enqueue//

void enqueue(Node* node){   
	pthread_mutex_lock( &lock );
	if (empty()) { 
		queue.head = node;
		queue.tail = queue.head; 
	}
	else {
		queue.tail->next=node;
		queue.tail = node;
	}
	pthread_cond_signal(&not_empty);
	pthread_mutex_unlock( &lock );
}


//treat_file//

void treat_file(char * path){
	if(!fnmatch(search_term,strrchr(path,'/')+1,0)){
		__sync_fetch_and_add(&counter, 1);
		printf("%s\n",path);
	}
}


//dequeue//

char* dequeue(){  
	pthread_mutex_lock( &lock );
	while (empty()){
		pthread_cond_wait(&not_empty ,&lock);
		pthread_testcancel();
	
	}
	pthread_testcancel();
	threads_counter++;	
	Node* temp = queue.head;
	queue.head = queue.head->next; 
	pthread_mutex_unlock( &lock );
	char* a = temp->dir;
	if(temp){
		free(temp);
	}
	return a;
} 


//free_queue//

void free_queue(){
	while(!empty()){
		Node* temp=queue.head->next;
		if(queue.head->dir){
			free(queue.head->dir);
		}
		if(!empty()){
			free(queue.head);
		}
		queue.head=temp;
	}
	return;
}


//handler//

void handler(void* arg){
	pthread_mutex_t *test = (pthread_mutex_t*)arg;
	pthread_mutex_unlock(test);
}


//browse//

void* browse(){
	pthread_cleanup_push(handler,&lock);
	while(1){
		char* directory = dequeue();
		DIR *dir = opendir(directory);
		struct dirent *entry;
		struct stat dir_stat;
		if (dir==NULL){
			perror(directory);
			__sync_fetch_and_add(&exit_error, 1);
			if(exit_error == num_of_threads){
				printf("Done searching, found %d files\n",counter);
            	pthread_mutex_destroy(&lock);
            	if(search_term){
					free(search_term);
				}
           	 	free_queue();
            	exit(1);
			}
			pthread_exit(NULL);
		}else{
			while ((entry = readdir(dir))){
				char buff[strlen(directory)+strlen(entry->d_name)+2];
				sprintf(buff,"%s/%s",directory,entry->d_name);
				stat(buff, &dir_stat);
				if(strcmp(entry->d_name,"..") != 0){
					if(((dir_stat.st_mode & __S_IFMT) == __S_IFDIR)  && (strcmp(entry->d_name, ".") != 0)){
						enqueue(create_node(buff));
					}else{
						treat_file(buff);
					}	
				}
			}
			if(directory){
				free(directory);
			}
			closedir(dir);
			pthread_testcancel();
			__sync_fetch_and_sub(&threads_counter, 1);
			if (empty()&& !threads_counter){
				raise(SIGUSR1);
			}
		}
	}
	pthread_cleanup_pop(1);
}


//cancel_handler//

void cancel_handler(int signal){
	pthread_t currentt_thread = pthread_self();
	for(size_t i=0; i<num_of_threads;i++){
		if(currentt_thread != threads[i]){
			pthread_cancel(threads[i]);
		}
	}
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&not_empty);
	if(search_term){
		free(search_term);
	}
	free_queue();
	return;
}


//int_handler//

void int_handler(int signal){
	printf("Search stopped, found %d files\n‏",counter);
	cancel_handler(signal);
	pthread_exit(NULL);
}


//self_handler//

void self_handler(int signal){
	printf("Done searching, found %d files\n", counter);
	cancel_handler(signal);
	pthread_exit(NULL);
}

//main//

int main(int argc, char** argv){
	struct sigaction int_ac;
	int_ac.sa_handler = int_handler;
	sigaction(SIGINT, &int_ac,NULL);
	struct sigaction self_ac;
	self_ac.sa_handler = self_handler;
	sigaction(SIGUSR1, &self_ac, NULL);
	if(argc!=4){
		perror("Not enough arguments!\n");
		exit(1);
	}
	search_term=(char*)malloc(sizeof(argv[2])+2);
	sprintf(search_term,"%c%s%c",'*',argv[2],'*');
	enqueue(create_node(argv[1]));
	num_of_threads = atoi(argv[3]);
	int rc = pthread_mutex_init( &lock, NULL );
	if( rc ){
		printf("ERROR in pthread_mutex_init(): ""%s\n", strerror(rc));
		exit(-1);
	}
	for (size_t i = 0; i < num_of_threads; i++){
		int rc = pthread_create(&threads[i], NULL, browse, (void *)i);
		if (rc){
			printf("ERROR in pthread_create(): %s\n", strerror(rc));
			exit(-1);
		}
	}
	for(int i = 0; i < num_of_threads; i++){
		pthread_join(threads[i], NULL);
	}
	return 0;
}







