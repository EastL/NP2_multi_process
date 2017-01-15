#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>
#include "shell.h"
#include "user.h"
#include "pipe.h"


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

void init_user_info()
{
	int shmid = shmget((key_t)USER_KEY, sizeof(user_node) * 31, IPC_CREAT | 0600);

	user_node *user_list = shmat(shmid, NULL, 0);

	for (int c = 0; c < 31; c++)
		user_list[c].ID = -1;
	shmdt(user_list);
}

void init_global_pipe()
{
	int shmid = shmget((key_t)PIPE_KEY, sizeof(user_node) * 31, IPC_CREAT | 0600);
	if (shmid < 0)
	{
		perror("shm err: \n");
		printf("shmid : %d\n", shmid);
	}

	user_node *pipe_list = shmat(shmid, NULL, 0);

	for (int c = 0; c < 31; c++)
		pipe_list[c].ID = -1;
	shmdt(pipe_list);
}

int main()
{
	init_user_info();
	init_global_pipe();

	//kill zombie
	signal(SIGCHLD, wait4_child);

	//create my socket
	char port_service[10];
	sprintf(port_service, "%d", 3425);

	int my_fd = passivesock(port_service, "tcp", 5);

	//client socket
	int clientfd;
	struct sockaddr_in client_socket;
	int cl_len = sizeof(client_socket);

	//for client ID
	int clientID[31] = {0};

	while(1)
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
		memset(user->name, 0, 30);
		strcpy(user->name, "(no name)");
		user->env_num = 1;
		bzero(user->env[0], 1024);
		strcpy(user->env[0], "PATH");
		bzero(user->envval[0], 1024);
		strcpy(user->envval[0], "bin:.");
		user->user_pipe_front = NULL;
		user->user_pipe_rear = NULL;
		user->user_cmd_front = NULL;
		user->user_cmd_rear = NULL;
		user->coda = 0;
		bzero(user->ip, 21);
		//strcpy(user->ip, adr);
		char myip[20] = "CGILAB";
		strcpy(user->ip, myip);
		//user->port = ntohs(client_socket.sin_port);
		user->port = 511;
			
		user->next = NULL;
		

		int pid = fork();

		if (pid == 0)
		{
			int r = 0;
			user->pid = getpid();
			push_user(user);
			r = shell(user);
			if (r == -1)
			{
				printf("ID: %d\n", user->ID);
				clientID[user->ID] = 0;
			}
			close(clientfd);
		}

		else
		{
			close(clientfd);
		}
	}

	return 0;
}
