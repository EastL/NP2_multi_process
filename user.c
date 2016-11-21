#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "user.h"
#include "message.h"

user_node *get_user_list()
{
	int shmid = shmget((key_t)USER_KEY, sizeof(user_node) * 31, 0666);
	user_node *ret = (user_node*) shmat(shmid, NULL, 0);
	return ret;
}

int get_client_coda(int id)
{
	user_node *user_list = get_user_list();

	int ret = user_list[id].coda;
	
	shmdt(user_list);
	return ret;
}

void set_client_coda(int id, int coda)
{
	user_node *user_list = get_user_list();

	user_list[id].coda = coda;
	
	shmdt(user_list);
}

void push_user(user_node *node)
{
	user_node *user_list = get_user_list();
	for (int i = 1; i < 31; i++)
	{
		if (user_list[i].ID == -1)
		{
			node->ID = i;
			user_list[i] = *node;
			break;
		}
	}
	shmdt(user_list);
}

void remove_user(user_node *node)
{
	node->ID = 0;
	node->user_fd = 0;
	node->user_pipe_front = NULL;
	node->user_pipe_rear = NULL;
	node->user_cmd_front = NULL;
	node->user_cmd_rear = NULL;
	node->next = NULL;
}

void unlink_user(user_node *node)
{
	user_node *user_list = get_user_list();

	user_list[node->ID].ID = -1;

	
	shmdt(user_list);
}

void broadcast_message(user_node *node, const char *m)
{
	user_node *user_list = get_user_list();

	for (int i = 1; i < 31; i++)
	{
		if (user_list[i].ID != -1)
		{
			send_message(node->ID, i, m);
			printf("pid:%d\n", user_list[i].pid);
			kill(user_list[i].pid, SIGUSR1);
		}
	}
	
	shmdt(user_list);
}

int is_name_exist(char *name)
{
	user_node *node = get_user_list();
	int ret = 0;
	
	for (int i = 1; i < 31; i++)
	{
		if (strcmp(node[i].name, name) == 0)
		{
			ret = 1;
			break;
		}
	}
	
	shmdt(node);
	return ret;
}

char *get_name(int id)
{
	user_node *node = get_user_list();
	char *name = malloc(50);
	memset(name, 0, 50);

	strcpy(name, node[id].name);

	shmdt(node);

	return name;
}

void who(user_node *node)
{
	char *title = "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n";
	write(node->user_fd, title, strlen(title));
	user_node *node_list = get_user_list();
	
	for (int i = 0; i < 31; i++)
	{
		if (node_list[i].ID != -1)
		{ 
			char *content = malloc(sizeof(char) * 100);
			memset(content, 0, 100);

			if (i == node->ID)
				sprintf(content, "%d\t%s\t%s/%d\t%s\n", node_list[i].ID, node_list[i].name, node_list[i].ip, node_list[i].port, "<- me");
			else
				sprintf(content, "%d\t%s\t%s/%d\n", node_list[i].ID, node_list[i].name, node_list[i].ip, node_list[i].port);
						
			write(node->user_fd, content, strlen(content));
			free(content);
		}
	}

	shmdt(node_list);
}

void name(user_node *node, char *n)
{
	user_node *node_list = get_user_list();

	memset(node_list[node->ID].name, 0, 30);
	strcpy(node_list[node->ID].name, n);
	strcpy(node->name, n);

	shmdt(node_list);
}

void yell(user_node *node, char *msg)
{
	user_node *node_list = get_user_list();
	char *yell_msg = malloc(sizeof(char) * 1024);
	memset(yell_msg, 0, 1024);

	sprintf(yell_msg, "*** %s yelled ***: %s", node_list[node->ID].name, msg);
	broadcast_message(node, yell_msg);
	free(yell_msg);
	
	shmdt(node_list);
}

void user_exit_broadcast(user_node *node)
{
	user_node *node_list = get_user_list();
	char *leave_msg = malloc(sizeof(char) * 50);
	memset(leave_msg, 0, 50);

	sprintf(leave_msg, "*** User '%s' left. ***", node_list[node->ID].name);
	broadcast_message(node, leave_msg);
	shmdt(node_list);

}

int tell(user_node *node, int to, char *msg)
{
	user_node *node_list = get_user_list();
	
	char *tell_msg = malloc(sizeof(char) * 1024);
	memset(tell_msg, 0, 1024);

	sprintf(tell_msg, "*** (%s) told you ***: %s\n", node_list[node->ID].name, msg);
	if (send_message(node->ID, to, tell_msg) == -1)
		return -1;
	kill(node_list[to].pid, SIGUSR1);

	free(tell_msg);
	shmdt(node_list);
	return 0;
}
