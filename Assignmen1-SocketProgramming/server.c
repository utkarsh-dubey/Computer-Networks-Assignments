//server side program

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <unistd.h> 
#include<pthread.h>
#include <dirent.h>
#include <fnmatch.h>
#define PORT 5000


char client_message[2000],clientCPUInfo[6000];
char buffer[6000];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct information{
	char processName[50];
	int pid;
	unsigned long time;
};

int comparator(const void* p, const void* q)
{
    return ((struct information*)p)->time <
                  ((struct information*)q)->time;
} 

int filter(const struct dirent *dir)
{
     return !fnmatch("[1-9]*", dir->d_name, 0);
}

void allPids(int needed){
	struct dirent **namelist;
	int n;
	
	//printf("checking\n");
	n = scandir("/proc", &namelist, filter, 0);
	struct information arr[n];
	int sizeArr=n;
	if (n < 0)
	perror("Not enough memory.");
	else {
	while(n--) {
			char filename[1000];
			unsigned long utime,stime;
			sprintf(filename, "/proc/%s/stat", namelist[n]->d_name);
			FILE *f = fopen(filename, "r");
			char name[50];
			struct information temp;
			int pid;
			fscanf(f, "%d %s %*c %*d %*d %*d %*d %*d %*u  %*lu %*lu %*lu %*lu %lu %lu",&pid, temp.processName,&utime,&stime);
			
			//printf("%lu\t%lu\n",utime,stime);
			//temp.processName=name;
			
			temp.pid=pid;
			temp.time=(utime+stime);
			arr[n-1]=temp;
	}
	free(namelist);
	qsort(arr,sizeArr,sizeof(struct information),comparator);
	needed=(needed>sizeArr)?sizeArr:needed;
	strcpy(buffer,"");
	for(int i=0;i<needed;i++){	
		char temp[100];
		strcpy(temp,"");
		sprintf(temp,"%s\t%d\t%lu\n",arr[i].processName,arr[i].pid,arr[i].time);
		strcat(buffer,temp);
	}
	
	}
	
}

void * socketThread(void *arg)
{
	printf("Connected to a client\n");
	int newSocket = *((int *)arg);
	strcpy(client_message,"");
	recv(newSocket , client_message , 2000 , 0);
	
	int needed;
	sscanf(client_message,"%d",&needed);
	allPids(needed);

  pthread_mutex_lock(&lock);
  pthread_mutex_unlock(&lock);
  
  sleep(1);
  
  send(newSocket,buffer,sizeof(buffer),0);
  printf("Data sent to the client\n");
  FILE *out = fopen("cpuinfo.txt","a+");
  fputs(buffer,out);
  fclose(out);
  printf("Data saved in a file\n");
  sleep(5);
 
  recv(newSocket , clientCPUInfo , 6000 , 0);
  printf("CPU information received by client is: \n");
  printf("%s\n",clientCPUInfo);
  printf("Connection with client ended\n");
  close(newSocket);
  pthread_exit(NULL);
}

int main(){
  int serverSocket, newSocket;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  //socket creation 
  serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
  bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  if(listen(serverSocket,50)==0)
    printf("Listening\n");
  else
    printf("Error\n");
    pthread_t tid[60];
    int i = 0;
    while(1)
    {
        //creates a new socket
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

        //new thread created for the client leaving main thread for next request
        if( pthread_create(&tid[i++], NULL, socketThread, &newSocket) != 0 )
           printf("Failed to create thread\n");

        if( i >= 50)
        {
          i = 0;
          while(i < 50)
          {
            pthread_join(tid[i++],NULL);
          }
          i = 0;
        }
    }
  return 0;
}
