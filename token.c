#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util.h"
#include "token.h"

static token_node *front = NULL;
static token_node *rear = NULL;

int get_token_type(char *str)
{
	if (regular_match(str, "\\|[[:digit:]]+") == 1)
		return PIPEN;
	else if (regular_match(str, "\\|") == 1)
		return PIPE;
	else if (regular_match(str, ">[[:digit:]]+") == 1)
		return PIPEPOUT;
	else if (regular_match(str, "<[[:digit:]]+") == 1)
		return PIPEPIN;
	else if (regular_match(str, ">") == 1)
		return REDIR;
	else if (regular_match(str, "![[:digit:]]+") == 1)
		return PIPERR;
	else if (regular_match(str, "\n") == 1)
		return NEWLINE;
	else 
		return CMDF;
}

void push_node(token_node **node)
{
	if (front == NULL && rear == NULL)
		front = rear  = *node;
	else
	{
		rear->next = *node;
		rear = *node;
	}
}

token_node *pull_node()
{
	if (front == NULL)
		return NULL;
	else
	{
		token_node *temp = front;
		front = front->next;
		return temp;
	}
}

void push_enter_node()
{
	token_node *temp = malloc(sizeof(token_node));
	temp->token = malloc(sizeof(int));
	temp->token[0] = '\n';
	temp->token[1] = '\0';
	temp->token_type = NEWLINE;
	temp->next = NULL;
	push_node(&temp);
}

void print_node()
{
	token_node *t = front;
	while(t != NULL)
	{
		printf("aa\n");
		printf("token:%s\n", t->token);
		printf("type:%d\n\n", t->token_type);
		t = t->next;
	}
	printf("hi\n");
}

void tokenizer(int cfd)
{
	char *buf = malloc(sizeof(char) * 10010);
	memset(buf, 0, 10010);
	size_t token_n;
	char **token_array;
	size_t i;
	size_t recvsize = 0;

	printf("precount\n");
	recvsize = read(cfd, buf, 10010);
	printf("size:%lu\n", recvsize);

	split(&token_array, buf, " ", &token_n);
	
	for (i = 0; i < token_n; i++)
	{

		//printf("token name:%s\n", token_array[i]);
		//printf("token size:%lu\n", strlen(token_array[i]));
		token_node *tnode = malloc(sizeof(token_node));
		if (token_array[i][strlen(token_array[i])-1] == '\n')
		{
			int strip = 0;
			if (token_array[i][strlen(token_array[i])-3] == '\r')
			{
				strip = 3;
			}

			else if (token_array[i][strlen(token_array[i])-2] == '\r')
			{
				strip = 2;
			}

			else
				strip = 1;
			
			printf("strip:%d\n", strip);

			if ((strlen(token_array[i]) - strip) == 0)
				push_enter_node();

			else
			{
				tnode->token = malloc(sizeof(char) * (strlen(token_array[i]) + 1));
				memset(tnode->token, 0, (strlen(token_array[i]) + 1));
				token_array[i][strlen(token_array[i])-strip] = '\0';
				strcpy(tnode->token, token_array[i]);
				tnode->token_type = get_token_type(token_array[i]);
				tnode->next = NULL;
				free(token_array[i]);
				push_node(&tnode);
				push_enter_node();
			}
		
		}

		else
		{
			tnode->token = malloc(sizeof(char) * (strlen(token_array[i]) + 1));
			memset(tnode->token, 0, (strlen(token_array[i]) + 1));
			strcpy(tnode->token, token_array[i]);
			tnode->token_type = get_token_type(token_array[i]);
			free(token_array[i]);
			tnode->next = NULL;
			push_node(&tnode);
		}
			
	}

	free(token_array);
	printf("count\n");
	//print_node();
}

token_node *get_node(int cfd)
{
	if (front == NULL)
	{
		rear = NULL;
		tokenizer(cfd);
	}

	return pull_node();

}

void free_token_node(token_node *node)
{
	free(node->token);
	node->token = NULL;
	node->token_type = 0;
	node->next = NULL;
	free(node);
}

void free_token()
{
	front = rear = NULL;
}
