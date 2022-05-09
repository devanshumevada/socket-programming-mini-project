

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
#include <libgen.h>
#define port 4444
#define size 1024

void execute_command(char *pattern, int socket, char *file_1) { 
	printf("\nPattern: %s\n",pattern);
    int fd;
	pid_t pid = fork();     
	if (pid == 0) {

			int fd = open("client_executed.txt", O_WRONLY | O_CREAT, 0666);
			dup2(fd, 1);
			char *args[]={"grep", "--color=auto", "-w", pattern, file_1, NULL};
			execvp(args[0],args);

	} else {
		wait(NULL);
	}

	

	
}

void send_file_to_server(char *file, int socket) {

    int n;
    

    int fd = open(file, O_RDONLY, S_IRUSR | S_IWUSR);

    long file_size = lseek(fd, 0L, SEEK_END);

            
    write(socket, &file_size, sizeof(long));
            
            

            
    char *buffer = malloc(file_size);
    lseek(fd, 0L, SEEK_SET);
    read(fd, buffer, file_size);
    write(socket, buffer, file_size);
         

    free(buffer);
    close(fd);
       

} 


void accept_file_from_server(int socket) {
    int n;

   


    int fd = open("server_executed.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    long file_size;
    read(socket, &file_size, sizeof(long));
    

    char *buffer = malloc(file_size);
    read(socket, buffer, file_size);
    write(fd, buffer, file_size);
	printf("\nInside send\n");

    free(buffer);
    close(fd);

}

void process_files(char *file_1, char *file_2, char *pattern) {
    
    printf("\nServer Execution: \n");
    char command[1024]= "";
    char *command_1 = "cat server_executed.txt | sed ";
    strcat(command, command_1);
    strcat(command,"s/^/");
    strcat(command,basename(file_2));
    strcat(command,":/");
    strcat(command, " | grep --color=auto ");
    strcat(command, pattern);
    printf("\n%s\n",command);
    system(command);


    printf("\nClient Execution: \n");
    char command2[1024]= "";
    char *command_2 = "cat client_executed.txt | sed ";
    strcat(command2, command_2);
    strcat(command2,"s/^/");
    strcat(command2,basename(file_1));
    strcat(command2,":/");
    strcat(command2, " | grep --color=auto ");
    strcat(command2, pattern);
    printf("\n%s\n",command2);
    system(command2);
    //char *args[] = {"cat", "cat", "server_executed.txt","|","sed",""};

}

void handle_server(int server, char *pattern, char* file_1, char *file_2) {
    
       
        send(server, pattern, size, 0);
        send_file_to_server(file_2, server);
        
       
        //bzero(buf, sizeof(buf));
        char buf[size];
        //read(server, buf, sizeof(buf));
        accept_file_from_server(server);
        execute_command(pattern, server, file_1);
        process_files(file_1, file_2, pattern);

}

int main(int argc, char *argv[]) {

    char *pattern = argv[1];
    char *file_1 = argv[2];
    char *file_2 = argv[3];

    if (pattern == NULL || file_1 == NULL || file_2 == NULL) {
        printf("\nEither of pattern or file 1 or file 2 missing\n");
        exit(1);
    }




    int server;
    socklen_t len;
    struct sockaddr_in servAdd;
    if((server = socket(AF_INET, SOCK_STREAM, 0))<0){
        fprintf(stderr, "Cannot create socket\n");
        exit(1);
    }
    servAdd.sin_family = AF_INET;
    servAdd.sin_port = htons((uint16_t)port);
    servAdd.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    if(connect(server,(struct sockaddr *)&servAdd, sizeof(servAdd))<0){
        fprintf(stderr, "connect() has failed, exiting\n");
        exit(3);
    } else {


        handle_server(server,pattern, file_1, file_2);
          
    }
	
    return 0;
}