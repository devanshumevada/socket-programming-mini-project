#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#define port 4444
#define size 1024

void execute_command(char *pattern, int socket);
void send_file_to_client(int socket);
void accept_file_from_client(int socket, char *pattern) {

	int n;
    int fd = open("client.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    long file_size;
    read(socket, &file_size, sizeof(long));
    char *buffer = malloc(file_size);
    read(socket, buffer, file_size);
    write(fd, buffer, file_size);
	printf("\nInside send\n");

    free(buffer);
    close(fd);
	execute_command(pattern,socket);
}

void send_file_to_client(int socket) {

	//printf("\nInside send_file_to_client\n");
	int fd = open("server_executed.txt", O_RDONLY, S_IRUSR | S_IWUSR);

    long file_size = lseek(fd, 0L, SEEK_END);

    write(socket, &file_size, sizeof(long));
    

           
    char *buffer = malloc(file_size);
    lseek(fd, 0L, SEEK_SET);
	read(fd, buffer, file_size);
    write(socket, buffer, file_size);
          

    free(buffer);
    close(fd);


}



void execute_command(char *pattern, int socket) { 
	printf("\nPattern: %s\n",pattern);
	pid_t pid = fork();     
	if (pid == 0) {

		int fd = open("server_executed.txt", O_WRONLY | O_CREAT, 0666);
		dup2(fd, 1);
		char *args[]={"grep", "--color=auto", "-w", pattern, "client.txt", NULL};
		execvp(args[0],args);

	} else {
		wait(NULL);
		send_file_to_client(socket);
	}

	

	
}



void handle_client(int client) {
	
	char pattern[size];
	bzero(pattern, sizeof(pattern));
	recv(client, pattern, sizeof(pattern), 0);
	
    printf("\nPattern: %s\n", pattern);
	
	accept_file_from_client(client, pattern);
	execute_command(pattern, client);
	//send_file_to_client("server_executed.txt", client);

}



int main(int argc, char *argv[]) {
    int sd, client, portNumber;
	socklen_t len;
	struct sockaddr_in servAdd;
	if ((sd=socket(AF_INET,SOCK_STREAM,0))<0){
	fprintf(stderr, "Cannot create socket\n");
	exit(1);
	}
	servAdd.sin_family = AF_INET;
	servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdd.sin_port = htons((uint16_t)port);
	bind(sd,(struct sockaddr*)&servAdd,sizeof(servAdd));
	listen(sd, 5);
	while(1){
		client=accept(sd,(struct sockaddr*)NULL,NULL);
		printf("A Client Entered\n");
		if(!fork())
			handle_client(client);
		close(client);
	}


	return 0;
    
}
