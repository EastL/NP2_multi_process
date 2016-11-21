#ifndef __MSG__
#define __MSG__

struct _msg
{
	int from;
	int to;
	char msg[1025];
};

typedef struct _msg msg_node;

int send_message(int from, int to, const char *msg);
msg_node recv_message(int from, int to);
#endif
