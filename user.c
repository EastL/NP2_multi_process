#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include "user.h"

user_node *get_user_list()
{
	int shmid = shmget((key_t)USER_KEY, sizeof(user_node) * 31, 0666);
	user_node *ret = (user_node*) shmat(shmid, NULL, 0);
	return ret;
}

void push_user(user_node *node)
{
	user_node *user_list = get_user_list();

	user_list[node->ID] = *node;
	shmdt(user_list);
}

void remove_user(user_node *node)
{
	node->ID = 0;
	node->name = NULL;
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

void broadcast_message(const char *m)
{
	user_node *user_list = get_user_list();

	

	shmdt(user_list);
}

user_node *search_name(user_node *front, int id)
{
	user_node *bro_node = front;
	
	while (bro_node != NULL)
	{
		if (bro_node->ID == id)
			break;	
		bro_node = bro_node->next;
	}
	
	return bro_node;
}
