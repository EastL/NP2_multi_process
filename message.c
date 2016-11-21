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
#include "message.h"

int send_message(int from, int to, const char *msg)
{
	int coda = get_client_coda(to);
	if (coda == 9)
		return -1;

	int m_key = 3421 + to;
	int shmid = shmget(m_key, sizeof(msg_node) * 10, IPC_CREAT | 0666);

	msg_node *msg_list = shmat(shmid, NULL, 0);

	msg_list[coda].from = from;
	msg_list[coda].to = to;
	strncpy(msg_list[coda].msg, msg, 1024);

	coda++;
	set_client_coda(to, coda);
	shmdt(msg_list);
	return 0;
}

msg_node recv_message(int from, int to)
{
	int coda = get_client_coda(to);
	coda--;
	msg_node ret;
	ret.from = -1;
	if (coda < 0)
		return ret;

	set_client_coda(to, coda);

	int m_key = 3421 + to;
	int shmid = shmget(m_key, sizeof(msg_node) * 10, IPC_CREAT | 0666);

	msg_node *msg_list = shmat(shmid, NULL, 0);

	ret = msg_list[coda];
	return ret;
}
