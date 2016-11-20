#ifndef __PIPE__
#define __PIPE__

struct __pipe
{
	int num;
	int from;
	int to;
	int infd;
	int outfd;
	int is_err;
	struct __pipe *next;
};

typedef struct __pipe pipe_node;

void push_pipe(pipe_node **pipe_front, pipe_node **pipe_rear, pipe_node **node);
pipe_node *check(pipe_node **pipe_front, int count);
void free_pipe(pipe_node *node);
void decress_count(pipe_node **pipe_front, pipe_node **pipe_rear);
pipe_node *search_pipe(int from, int to);
void put_pipe(pipe_node *node);
void delete_pipe(pipe_node *node);
#endif
