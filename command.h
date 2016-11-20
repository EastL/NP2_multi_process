#ifndef __COMMAND__
#define __COMMAND__

#define ISPIPE 1
#define ISPIPEN 2
#define ISPIPEERR 3
#define ISREDIR 4

struct __cmd
{
	char *cmd;
	char **arg;
	int arg_count;
	int pip_count;
	int pip_process_count_in;
	int pip_process_count_out;
	int type;
	char *file;
	int is_init;
	int is_new;
	int is_pipe_out;
	int is_pipe_in;
	struct __cmd *next;
};

typedef struct __cmd cmd_node;

void push_cmd(cmd_node **cfront, cmd_node **crear, cmd_node **node);
cmd_node *pull_cmd(cmd_node **cfront, cmd_node **crear);
void print_cmd(cmd_node **cfront);
void free_cmd(cmd_node *node);
void free_cmd_line(cmd_node **cfront, cmd_node **crear);
#endif
