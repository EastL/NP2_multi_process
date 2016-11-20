#ifndef __SHELL__
#define __SHELL__
#include "command.h"
#include "user.h"

int shell(user_node *client_fd);
int execute_node(cmd_node *node, user_node *client_fd, int *next_n);
int check_cmd_exist(char *cmd, char *path);
char *trick(char *str);
#endif
