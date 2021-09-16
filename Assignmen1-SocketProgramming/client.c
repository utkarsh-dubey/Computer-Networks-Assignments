// Client side program
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#define PORT 5000

char returnMessage[6000];

struct information{
	char processName[50];
	int pid;
	unsigned long time;
};

int comparator(const void* p, const void* q)
{
    return ((struct information*)p)->time < ((struct information*)q)->time;
} 

int filter(const struct dirent *dir)
{
     return !fnmatch("[1-9]*", dir->d_name, 0);
}

void allPids(){

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
			 //processdir(namelist[n]);
			 //free(namelist[n]);
			 	//printf("%s\n",namelist[n]->d_name);
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
		strcpy(returnMessage,"");
		for(int i=0;i<1;i++){
			char temp[100];
			strcpy(temp,"");
			sprintf(temp,"%s\t%d\t%lu\n",arr[i].processName,arr[i].pid,arr[i].time);
			strcat(returnMessage,temp);
		}
		
	}
}



int main(int argc, char const *argv[])
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[6000] = {0};

	sock = socket(AF_INET, SOCK_STREAM, 0); 
	if ( sock < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	printf("Connected to server\n");
	printf("Enter the number of top process needed\n");
	char number[2];
	int n;
	scanf("%d",&n);
	sprintf(number,"%d",n);
	
	send(sock , number, strlen(number) , 0 );
	
	printf("Rquest sent\n");
	sleep(5);
	
	valread = read( sock , buffer, 6000);
	
	printf("Information received by the server is\n");
	printf("%s\n",buffer );
	
	FILE *out = fopen("cpuinfoclient.txt","w");
	fputs(buffer,out);
	fclose(out);
	
	printf("Information received by server is saved in a file\n");
	
	allPids();
	
	send(sock , returnMessage, strlen(returnMessage) , 0 );
	
	printf("Information sent to the server\n");
	printf("Closing the connection\n");
	
	return 0;
}
