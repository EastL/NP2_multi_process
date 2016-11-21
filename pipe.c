#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "command.h"
#include "token.h"
#include "pipe.h"
#include "user.h"

void push_pipe(pipe_node **pipe_front, pipe_node **pipe_rear, pipe_node **node)
{
	if (*pipe_front == NULL)
	{
		*pipe_front = *pipe_rear = *node;
	}

	else
	{
		(*pipe_rear)->next = *node;
		*pipe_rear = *node;
	}
}


pipe_node *check(pipe_node **pipe_front, int count)
{
	pipe_node *temp = *pipe_front;
	pipe_node *ret = NULL;

	while (temp != NULL)
	{
		printf("ffffffind count:%d\n", temp->num);
		if (temp->num == count)
			ret = temp;

		temp = temp->next;
	}

	return ret;
}

void free_pipe(pipe_node *node)
{
	node->num = 0;
	node->from = 0;
	node->to = 0;
	node->infd = 0;
	node->outfd = 0;
	node->next = NULL;
	free(node);
}

void decress_count(pipe_node **pipe_front, pipe_node **pipe_rear)
{
	pipe_node *temp = *pipe_front;
	pipe_node *pre = NULL;

	while (temp != NULL)
	{
		printf("decr num:%d\n", temp->num);
		printf("decr infd:%d\n", temp->infd);
		printf("decr outfd:%d\n", temp->outfd);
		if (--temp->num == -1)
		{
			//do unlink and free
			if (pre == NULL)
			{
				*pipe_front = temp->next;

				free_pipe(temp);

				//for next
				temp = *pipe_front;
			}
			else
			{
				pre->next = temp->next;
				free_pipe(temp);
				//for next
				temp = pre->next;
			}	
		}

		else
		{
			pre = temp;
			temp = temp->next;
		}

	}
}

pipe_node *get_global_pipe()
{
	int shmid = shmget((key_t)PIPE_KEY, sizeof(pipe_node) * 50, IPC_CREAT | 0600);
	pipe_node *global_pipe_list = shmat(shmid, NULL, 0);
	return global_pipe_list;
}


pipe_node *search_pipe(int from, int to)
{
	pipe_node *temp = get_global_pipe();
	pipe_node *ret = malloc(sizeof(pipe_node));
	int exist = 0;

	for (int c = 0; c < 50; c++)
	{
		if (temp[c].from == from && temp[c].to == to && temp[c].ID != -1)
		{
			*ret = temp[c];
			exist = 1;
			break;
		}
	}

	shmdt(temp);
	if (exist)
		return ret;
	else
		return NULL;
}

void put_pipe(pipe_node *node)
{
	pipe_node *temp = get_global_pipe();

	for (int i = 0; i < 50; i++)
	{
		if (temp[i].ID == -1)
		{
			node->ID = i;
			temp[i] = *node;
			printf("from:%d\n", temp[i].from);
			printf("to:%d\n", temp[i].to);
			break;
		}
	}
	
	shmdt(temp);
}

void delete_pipe(pipe_node *node)
{
	pipe_node *temp = get_global_pipe();
	temp[node->ID].ID = -1;
	temp[node->ID].num = 0;
	temp[node->ID].from = 0;
	temp[node->ID].to = 0;
	temp[node->ID].infd = 0;
	temp[node->ID].outfd = 0;
	temp[node->ID].is_err = 0;
	
	shmdt(temp);
}
