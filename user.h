#ifndef __USER__
#define __USER__
#define USER_KEY 6666
#include "command.h"
#include "pipe.h"

struct _user
{
	int ID;
	int pid;
	char *name;
	int user_fd;
	char env[256][1024];
	char envval[256][1024];
	int env_num;
	int coda;

	char ip[21];
	unsigned short port;

	pipe_node *user_pipe_front;
	pipe_node *user_pipe_rear;

	cmd_node *user_cmd_front;
	cmd_node *user_cmd_rear;

	struct _user *next;
};

typedef struct _user user_node;

user_node *get_user_list();
int get_client_coda(int id);
void set_client_coda(int id, int coda);
void push_user(user_node *node);
void remove_user(user_node *node);
void unlink_user(user_node *node);
void broadcast_message(user_node *node, const char *m);
user_node *search_name(user_node *front, int id);

#endif
