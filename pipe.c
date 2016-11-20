#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
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

pipe_node *search_pipe(int from, int to)
{
	extern pipe_node *pipe_client_front;
	extern pipe_node *pipe_client_rear;

	pipe_node *temp = pipe_client_front;
	while (temp != NULL)
	{
		if (temp->from == from && temp->to == to)
			break;
		temp = temp->next;
	}

	return temp;
}

void put_pipe(pipe_node *node)
{
	extern pipe_node *pipe_client_front;
	extern pipe_node *pipe_client_rear;

	if (pipe_client_front == NULL)
	{
		pipe_client_front = pipe_client_rear = node;
	}

	else
	{
		pipe_client_rear->next = node;
		pipe_client_rear = node;
	}
	
}

void delete_pipe(pipe_node *node)
{
	extern pipe_node *pipe_client_front;
	extern pipe_node *pipe_client_rear;
	pipe_node *temp = pipe_client_front;
	pipe_node *pre = NULL;

	while (temp != NULL)
	{
		if (temp == node)
		{
			if (pre == NULL)
			{
				pipe_client_front = temp->next;
				if (pipe_client_front == NULL)
					pipe_client_rear = NULL;
			}

			else
			{
				pre->next = temp->next;
				if (temp == pipe_client_rear)
					pipe_client_rear = pre;
			}

			break;
		}

		else
		{	
			pre = temp;
			temp = temp->next;
		}
	}
}
