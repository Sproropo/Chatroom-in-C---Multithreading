#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <pthread.h> 
#include <sys/types.h>
#include <signal.h> 

#define SET_BUFFER 2048

int listening_socket = 0, conn_socket = 0; 
                                           
typedef struct client_struct{ 
    int local_sock; 
    char name[14]; 
    short volatile int online_flag; 
	struct sockaddr_in address; 
    struct client_struct *next; 
}client_info; 
client_info *first_client = NULL; 

pthread_mutex_t mut_ip_name_recv = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mut_msg_recv = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mut_client_sock= PTHREAD_MUTEX_INITIALIZER; 

void *handle_client(void *arg); 
void append_client(client_info user); 
int update_client_socket(client_info user); 
client_info *search_client(char *namae); 
void send_message(char *s, client_info *user); 
void set_online_flag(char *namae); 

int main(int argc, char **argv){ 
    puts("Welcome. To exit press ctrl+c or ctrl+\\");
	if(argc != 2){ 
        fprintf(stderr, "Usage: %s <port>\n", argv[0]); 
		return EXIT_FAILURE; 
	}

	int port = atoi(argv[1]); 
    struct sockaddr_in server; 
    struct sockaddr_in client; 
    pthread_t tid; 

    signal(SIGPIPE, SIG_IGN); 
   
    listening_socket = socket(AF_INET, SOCK_STREAM, 0); 
    memset(&server, 0, sizeof(server)); 
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(port); 
	int option = 0;
	if(setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR,(char*)&option,sizeof(option)) < 0){ /* SOL_SOCKET -> Stiamo configurando le opzioni per un File Descriptor di tipo socket 
SO_REUSEADDR -> Posso eseguire un bind su una certa porta anche se esistono delle connessioni established che usano quella porta */
		perror("ERROR: setsockopt failed"); 
    return EXIT_FAILURE;
	}

/* Con bind() vincolo la socket ad un indirizzo ip e porta. 
Il primo argomento specifica il file descriptor da vincolare (la socket). Il secondo argomento punta alla struttura contenente l'indirizzo e la porta a cui vincolare la socket. Il terzo specifica la dimensione della struttura del secondo argomento*/
    if(bind(listening_socket, (struct sockaddr*)&server, sizeof(server)) < 0) { 
       perror("ERROR: Socket binding failed"); 
        return EXIT_FAILURE; 
    }
    

    if (listen(listening_socket, 10) < 0) { 
        perror("ERROR: Socket listening failed"); 
        return EXIT_FAILURE; 
        }

        
    puts("Waiting for incoming connections...");
    while(1) {
        socklen_t client_struct_dim = sizeof(struct sockaddr_in); 
        if ((conn_socket = accept(listening_socket, (struct sockaddr *)&client, &client_struct_dim)) < 0) 
            { 
                perror("ERROR: accept failed"); 
            return EXIT_FAILURE; 
            }

        
        client_info *user = calloc(1, sizeof(client_info)); 
        pthread_mutex_lock(&mut_client_sock); 
            user->address = client; 
            user->local_sock = conn_socket; 
        pthread_mutex_unlock(&mut_client_sock); 
        pthread_create(&tid, NULL, &handle_client, (void*)user); 
     } 
 	return EXIT_SUCCESS;
 }

void *handle_client(void *arg){
    client_info *local_user; 
    local_user = (client_info *) arg;
    char message_to_send[SET_BUFFER];
    char buff_handle[SET_BUFFER-3];
	char *buff_handle_extra; 
    char name[14]; 
    client_info *client_receiver; 
    char *token_name; 
    pthread_mutex_lock(&mut_ip_name_recv); 
    if(recv(local_user->local_sock, name, 14, 0) <= 0){ 
        printf("ERROR: Can't retrieve the name\n");
	    close(local_user->local_sock); 
        free(local_user); 
        return NULL; 
    } else {    strcpy(local_user->name, name); 
                short int is_online; 
                if(!search_client(local_user->name)) { 
                    append_client(*local_user); 
                    sprintf(buff_handle, "Your first time? Welcome. You're now connected\n"); 
                    } else {
                        is_online = update_client_socket(*local_user); 
                        if(is_online) { 
                            sprintf(buff_handle, "The client is already connected. Bye\n"); 
                        } else { 
                            sprintf(buff_handle, "Connected\n"); 
                        }
                        }
            send_message(buff_handle, local_user); 
            if(is_online) { 
                memset(&buff_handle, 0, sizeof(buff_handle)); 
                close(local_user->local_sock); 
                free(local_user); 
                pthread_mutex_unlock(&mut_ip_name_recv); 
                return NULL; 
            }
            printf("%s:%s connected\n",inet_ntoa(local_user->address.sin_addr), local_user->name); 
            memset(&buff_handle, 0, sizeof(buff_handle)); 
	}
    pthread_mutex_unlock(&mut_ip_name_recv); 
	while(1){
		int receive = recv(local_user->local_sock, buff_handle, SET_BUFFER, 0);
        pthread_mutex_lock(&mut_msg_recv);
		    if (receive >= 0){ 
                if (buff_handle[0] == '@') { 
                    token_name = strtok_r(buff_handle + 1, ":",&buff_handle_extra); 
                    if(!strcmp(local_user->name, token_name)) { 
                        sprintf(message_to_send, "You can't text yourself\n");
                        send_message(message_to_send, local_user);
                    } else if(!search_client(token_name)) {
                        sprintf(message_to_send, "The client doesn't exists\n");
                        send_message(message_to_send, local_user);
                    } else { 
                        sprintf(message_to_send, "> %s:%s\n", local_user->name, buff_handle_extra); 
                        client_receiver = search_client(token_name); 
                        if(write(client_receiver->local_sock, message_to_send, strlen(message_to_send)) < 0){ 
                            if(errno == 9) {
                                sprintf(message_to_send, "The client is not available\n");
                                send_message(message_to_send, local_user); 
                            } else { 
                                sprintf(message_to_send, "Sorry, we couldn't send you message. Please write it again\n");
                                send_message(message_to_send, local_user); 
                            };
                        };
                    }} else {
                        
                        set_online_flag(local_user->name); 
                        sprintf(buff_handle, "%s has left\n", local_user->name); 
                        printf("%s", buff_handle);
                        pthread_mutex_unlock(&mut_msg_recv);  
                        break; } 
		 } else {
                perror("ERROR: recv failed"); 
		}
        memset(&buff_handle, 0, sizeof(buff_handle));
        memset(&message_to_send, 0, sizeof(message_to_send));
        pthread_mutex_unlock(&mut_msg_recv); 
	} 
	close(local_user->local_sock); 
    free(local_user); 
    return NULL; 
}

void send_message(char *s, client_info *user){ 
		if(write(user->local_sock, s, strlen(s)) < 0){
            printf("ERROR: write to %s socket failed",user->name);
        }
}

void append_client(client_info user){ 
    client_info *temp, *last_client;
    temp = calloc(1, sizeof(client_info)); 
    temp->address = user.address; 
    temp->local_sock = user.local_sock;
    strcpy(temp->name, user.name);
    temp->online_flag = 1;
    temp->next = NULL;
    last_client = first_client;
    if (!last_client) { 
        first_client = temp; 
        }
    else { while(last_client->next) { 
        last_client = last_client->next; 
        }
        last_client->next = temp; 
    } }

client_info *search_client(char *namae) { 
    client_info *tmp;
    tmp = first_client;
    while (tmp != NULL && strcmp(tmp->name,namae)) tmp = tmp->next;
    return tmp;
}

int update_client_socket(client_info user) { 
    client_info *temp;
    temp = first_client;
    while (strcmp(temp->name,user.name)) { 
        temp = temp->next;
        }  
    if(temp->online_flag == 1){ 
        return 1;
    } else { 
        memset(&temp->address, 0, sizeof(temp->address));
        temp->address = user.address; 
        temp->local_sock = user.local_sock; 
        temp->online_flag = 1;
        return 0;
    }

}

void set_online_flag(char *namae) { 
    client_info *tmp;
    tmp = first_client;
    while (strcmp(tmp->name,namae)) {tmp = tmp->next;}
    tmp->online_flag = 0;
}