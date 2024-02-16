#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048


volatile unsigned short int flag = 0; 
int connect_socket = 0; 
char name[14]; 
void send_message(char *s); 
void send_msg_handler(); 
void recv_msg_handler(); 
void catch_signal(int sig); 
int main(int argc, char **argv){
	if(argc != 4){ 
		fprintf(stderr, "Usage: %s <ip address> <username> <port>\n", argv[0]);
		return EXIT_FAILURE; 
	}
	char *ip = argv[1]; 
	char *name = argv[2]; 
	int port = atoi(argv[3]); 
	
	signal(SIGINT, catch_signal); 
	signal(SIGQUIT, catch_signal); 

	if (strlen(name) > 14 || strlen(name) < 2){ 
		printf("Name must be less than 14 and more than 2 characters.\n");
		return EXIT_FAILURE; 
	}

	struct sockaddr_in server_addr; 
								    

	connect_socket = socket(AF_INET, SOCK_STREAM, 0); 
	memset(&server_addr, 0, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr(ip); 
	server_addr.sin_port = htons(port); 

	
    if(connect(connect_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))){ 
        perror("ERROR: connect\n"); 
		
		return EXIT_FAILURE; 
						} 
	if(send(connect_socket, name ,strlen(name), 0) < 0) 
	{
		perror("ERROR: Send failed"); 
		return EXIT_FAILURE;
	}
    char server_reply[55]; 
	if(recv(connect_socket, server_reply , strlen(server_reply) , 0) < 0)
	{
		perror("ERROR: Receive failed"); 
		return EXIT_FAILURE;
	}
	printf("%s",server_reply); 

	pthread_t send_msg_thread; 
  	if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    	return EXIT_FAILURE;
	} 

	pthread_t recv_msg_thread; 
  	if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	} 

	while (1){ 
		if(flag){ 
			printf("Thanks for using this ChatRoom\n"); 
			break; 
    } }
	close(connect_socket); 

	return EXIT_SUCCESS; 
	}

void send_message(char *s){ 
		if((send(connect_socket, s, strlen(s), 0)) < 0){ 
            perror("ERROR: Send to descriptor socket failed");
        }
}

void send_msg_handler() { 
    char message[LENGTH];
    char buff_send[LENGTH + 32];
    int count = 0; 

  while(1) {
    fgets(message, LENGTH, stdin); 
    while(count < strlen(message) && message[count] != ':') count++; 
	if('\n' == message[strlen(message) - 1]) message[strlen(message) - 1] = '\0'; 
    if (strcmp(message, "exit") == 0) { 
			send_message(message); 
            flag = 1; 
    }   else if(message[0] != '@') { 
                memset(message, 0, sizeof(message)); 
				count = 0; 
                continue; 
        }   else if(!strchr(message, ':' )){ 
                memset(message, 0, sizeof(message)); 
                printf("Maybe you forgot ':'\n");  
				count = 0; 
                continue;              
            }   else if(count > 14){ 
                    memset(message, 0, sizeof(message)); 
                    printf("The name of the client receiver exceeds the maximum number of characters the names can have \n");  
                    count = 0; 
                    continue;              
                }   else {  
                        sprintf(buff_send, "%s", message); 
						send_message(buff_send);
						count = 0;
                    }
        memset(message, 0, sizeof(message));
    	memset(buff_send, 0, sizeof(buff_send));} 
	}

void recv_msg_handler() {
	char message_recv[LENGTH];
  while (1) {
		int receive = recv(connect_socket, message_recv, LENGTH, 0);
    if (receive > 0) { 
        fprintf(stdout, "%s", message_recv); 
    } else if (receive == 0) { 
			flag = 1;
    } else { 
        break; 
		}
		memset(message_recv, 0, sizeof(message_recv));
  } }

void catch_signal(int sig) {
    flag = 1;
}