#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>
#include "shell.h"
#include "user.h"


//user info
user_node *user_list_front = NULL;
user_node *user_list_rear = NULL;

void wait4_child(int signo)
{
	int status;  
	while(waitpid(-1, &status, WNOHANG) > 0);  
}

int passivesock(const char *service, const char *transport, int qlen)
{
	struct servent  *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;

	int s, type;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	if ( pse = getservbyname(service, transport) )  
		sin.sin_port = htons(ntohs((unsigned short)pse->s_port));  

	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)  
		perror("can't get service entry\n");

	if ( (ppe = getprotobyname(transport)) == 0)
		perror("can't get protocol entry\n");

	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	
	else
		type = SOCK_STREAM;

	
	s = socket(PF_INET, type, ppe->p_proto);

	int sock_opt = 1 ;
	if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(void *)&sock_opt,sizeof(sock_opt)) == -1)
		perror("setsockopt error");

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		perror("bind error");

	if (type == SOCK_STREAM && listen(s, qlen) < 0)
		perror("listen error");
	
	return s;
}

int main()
{
	char *welcome = "****************************************\n** Welcome to the information server. **\n****************************************\n";
	//kill zombie
	signal(SIGCHLD, wait4_child);

	//create my socket
	char port_service[10];
	sprintf(port_service, "%d", 13421);

	int my_fd = passivesock(port_service, "tcp", 5);

	//client socket
	int clientfd;
	struct sockaddr_in client_socket;
	int cl_len = sizeof(client_socket);


	//set select
	fd_set rfds;
	fd_set afds;
	int nfds = getdtablesize();

	FD_ZERO(&afds);
	FD_SET(my_fd, &afds);

	//for client ID
	int clientID[31] = {0};

	while(1)
	{
		printf("hhhhh\n");
		memcpy(&rfds, &afds, sizeof(rfds));
		if (select(nfds, &rfds, NULL, NULL, NULL) < 0)
			perror("select error");
		printf("select!!\n");

		if (FD_ISSET(my_fd, &rfds))
		{
			//accept client
			if ((clientfd = accept(my_fd, (struct sockaddr*)&client_socket, &cl_len)) < 0)
			{
				printf("Accept error!");
				continue;
			}

			printf("accept %d\n", clientfd);
			char adr[20];
			inet_ntop(AF_INET, &(client_socket.sin_addr), adr, 20);
			printf("accept address: %s\n", adr);
			printf("accept port: %d\n", ntohs(client_socket.sin_port));
			//add client info
			user_node *user = malloc(sizeof(user_node));
			memset(user, 0, sizeof(user_node));

			//find ID
			int index;
			for (index = 1; index < 31; index++)
			{
				if (clientID[index] == 0)
				{
					clientID[index] = 1;	
					break;
				}
			}

			user->ID = index;
			user->user_fd = clientfd;
			user->name = malloc(10);
			memset(user->name, 0, 10);
			user->name = "(no name)";
			user->env_num = 1;
			bzero(user->env[0], 1024);
			strcpy(user->env[0], "PATH");
			bzero(user->envval[0], 1024);
			strcpy(user->envval[0], "bin:.");
			user->user_pipe_front = NULL;
			user->user_pipe_rear = NULL;
			user->user_cmd_front = NULL;
			user->user_cmd_rear = NULL;
			bzero(user->ip, 21);
			strcpy(user->ip, adr);
			user->port = ntohs(client_socket.sin_port);
			
			user->next = NULL;
			
			push_user(&user_list_front, &user_list_rear, user);
			printf("added : %x\n", user);
			printf("added next: %x\n", user->next);
			printf(" front:0x%x\n", user_list_front);
			printf(" front next:0x%x\n", user_list_front->next);
			printf(" rear:0x%x\n", user_list_rear);

			//welcome msg
			write(clientfd, welcome, strlen(welcome));
			printf("welcome\n");
			

			//broadcast
			char *bro_msg = malloc(sizeof(char) * 80);
			memset(bro_msg, 0, 80);

			sprintf(bro_msg, "*** User '(no name)' entered from %s/%d. ***", user->ip, user->port);
			broadcast_message(user_list_front, bro_msg);

		
			//set afds
			FD_SET(clientfd, &afds);
		}

		else
		{
			int ret_sh = -1;
			//find user for open shell
			int i = 0;
			user_node *active_user = user_list_front;
			while (active_user != NULL)
			{
				printf("%d before front:0x%x\n", i, user_list_front);
				printf("%d before front next:0x%x\n", i, user_list_front->next);
				printf("%d before rear:0x%x\n", i, user_list_rear);
				if (FD_ISSET(active_user->user_fd, &rfds))
				{
					ret_sh = shell(active_user);
					if (ret_sh == -1)
					{
						printf("front:0x%x\n", user_list_front);
						printf("front next:0x%x\n", user_list_front->next);
						printf("rear:0x%x\n", user_list_rear);

						close(active_user->user_fd);
						unlink_user(&user_list_front, &user_list_rear, active_user);

						//clear client ID
						clientID[active_user->ID] = 0;

						//brocast
						char *leave_msg = malloc(sizeof(char) * 50);
						memset(leave_msg, 0, 50);

						sprintf(leave_msg, "*** User '%s' left. ***", active_user->name);
						broadcast_message(user_list_front, leave_msg);

						FD_CLR(active_user->user_fd, &afds);
						user_node *temp_user = active_user;
						active_user = active_user->next;
						remove_user(temp_user);
					}
					
					else
						active_user = active_user->next;

					user_node *temp = user_list_front;
					while (temp != NULL)
					{
						printf("list:%x\n", temp);
						temp = temp->next;
					}
				}

				else
				{
					active_user = active_user->next;
				}
			}
			
		}

	}

	return 0;
}
