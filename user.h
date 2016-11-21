#ifndef __USER__
#define __USER__
#define USER_KEY 6666
#include "command.h"
#include "pipe.h"

struct _user
{
	int ID;
	int pid;
	char name[30];
	int user_fd;
	char env[128][1024];
	char envval[128][1024];
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
char *get_name(int id);
int is_name_exist(char *name);
void who(user_node *node);
void name(user_node *node, char *n);
void yell(user_node *node, char *msg);
void user_exit_broadcast(user_node *node);
int tell(user_node *node, int to, char *msg);

#endif
