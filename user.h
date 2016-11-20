#ifndef __USER__
#define __USER__
#include "command.h"
#include "pipe.h"

struct _user
{
	int ID;
	char *name;
	int user_fd;
	char env[256][1024];
	char envval[256][1024];
	int env_num;

	char ip[21];
	unsigned short port;

	pipe_node *user_pipe_front;
	pipe_node *user_pipe_rear;

	cmd_node *user_cmd_front;
	cmd_node *user_cmd_rear;

	struct _user *next;
};

typedef struct _user user_node;

void push_user(user_node **front, user_node **rear, user_node *node);
void remove_user(user_node *node);
void unlink_user(user_node **front, user_node **rear, user_node *node);
void broadcast_message(user_node *front, const char *m);
user_node *search_name(user_node *front, int id);

#endif
