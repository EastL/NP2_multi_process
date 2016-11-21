#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "shell.h"
#include "command.h"
#include "parse.h"
#include "util.h"
#include "pipe.h"
#include "user.h"
#include "message.h"

//user info
extern user_node *user_list_front;
extern user_node *user_list_rear;

//pipe process node
pipe_node *pipe_client_front = NULL;
pipe_node *pipe_client_rear = NULL;

int cid;
int cfd;

void recv_msg()
{
	msg_node recv = recv_message(-1, cid);
	write(cfd, recv.msg, strlen(recv.msg));
	write(cfd, "\n% ", 3);
}

int shell(user_node *client_fd)
{
	cid = client_fd->ID;
	cfd = client_fd->user_fd;
	signal(SIGUSR1, recv_msg);

	char *welcome = "****************************************\n** Welcome to the information server. **\n****************************************\n";
	char *shellsign = "% ";
	ssize_t bufsize = 0;
	cmd_node *cmd_list;

	//welcome msg
	write(client_fd->user_fd, welcome, strlen(welcome));
	printf("welcome\n");
			

	//broadcast
	char *bro_msg = malloc(sizeof(char) * 80);
	memset(bro_msg, 0, 80);

	sprintf(bro_msg, "*** User '(no name)' entered from %s/%d. ***", client_fd->ip, client_fd->port);
	broadcast_message(client_fd, bro_msg);
	printf("broadcast msg: %s\n", bro_msg);

	//set env
	for (int c = 0; c < client_fd->env_num; c++)
		setenv(client_fd->env[c], client_fd->envval[c], 1);

	while(1)
	{
		//read and parsing line
		parse(client_fd);
		print_cmd(&(client_fd->user_cmd_front));

		//execute process
		cmd_node *current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));

		int next_pipe_num = 0;
		int sign = 1;

		while (current_cmd != NULL)
		{
			if (strncmp(current_cmd->cmd, "setenv", 6) == 0)
			{
				if (current_cmd->arg[1] == NULL || current_cmd->arg[2] == NULL)
				{
					char *ts = "Please give args.\n";
					write(client_fd->user_fd, ts, strlen(ts));
					decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					continue;
				}

				//write to user info
				int cmp = 0;
				for (int c = 0; c < client_fd->env_num; c++)
				{
					if (strcmp(client_fd->env[c], current_cmd->arg[1]) == 0)
					{
						cmp = 1;
						strcpy(client_fd->envval[c], current_cmd->arg[2]);
					}
				}

				if (!cmp)
				{
					strcpy(client_fd->env[client_fd->env_num], current_cmd->arg[1]);
					strcpy(client_fd->envval[client_fd->env_num], current_cmd->arg[2]);
					client_fd->env_num++;
				}

				//real set
				setenv(current_cmd->arg[1], current_cmd->arg[2], 1);
				free_cmd(current_cmd);

				//last command, decress
				decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

				current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
			}

			else if (strncmp(current_cmd->cmd, "who", 3) == 0)
			{
				who(client_fd);
				decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

				current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
			}

			else if (strncmp(current_cmd->cmd, "name", 4) == 0)
			{
				if (current_cmd->arg[1] == NULL)
				{
					char *ts = "Please give args.\n";
					write(client_fd->user_fd, ts, strlen(ts));
					fflush(stdout);
					decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					continue;
				}

				int check_name = is_name_exist(current_cmd->arg[1]);

				if (check_name)
				{
					char *name_repeat = malloc(sizeof(char) * 80);
					memset(name_repeat, 0, 80);

					sprintf(name_repeat, "*** User '%s' already exists. ***\n", current_cmd->arg[1]);
					write(client_fd->user_fd, name_repeat, strlen(name_repeat));
					free(name_repeat);
					decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					continue;
				}


				name(client_fd, current_cmd->arg[1]);

				char *bro_name = malloc(sizeof(char) * 100);
				memset(bro_name, 0, 100);

				sprintf(bro_name, "*** User from %s/%d is named '%s'. ***\n", client_fd->ip, client_fd->port, current_cmd->arg[1]);
				broadcast_message(client_fd, bro_name);
				free(bro_name);
				bro_name = NULL;
				sign = 0;
				
				decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

				current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
			}

			else if (strncmp(current_cmd->cmd, "tell", 4) == 0)
			{
				if (current_cmd->arg[1] == NULL || current_cmd->arg[2] == NULL)
				{
					char *ts = "Please give args.\n";
					write(client_fd->user_fd, ts, strlen(ts));
					decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					continue;
				}

				int merge_tell = current_cmd->arg_count;
				char *marg = malloc(512);
				memset(marg, 0, 512);
				int merge_count;

				strncpy(marg, current_cmd->arg[2], strlen(current_cmd->arg[2]));
				if (merge_tell > 3)
				{
					for (merge_count = 3; merge_count < merge_tell; merge_count++)
					{
						printf("arggg : %s\n", current_cmd->arg[merge_count]);
						//sprintf(marg, "%s %s", marg, current_cmd->arg[merge_count]);
						strcat(marg, " ");
						strcat(marg, current_cmd->arg[merge_count]);
					}
				}

				printf("arg: %s\n", marg);
				
				int ID = atoi(current_cmd->arg[1]);

				if (tell(client_fd, ID, marg) < 0)
				{
					char *tell_err = malloc(sizeof(char) * 100);
					memset(tell_err, 0, 100);
					sprintf(tell_err, "*** Error: user #(%d) does not exist yet. ***\n", ID);

					write(client_fd->user_fd, tell_err, strlen(tell_err));
					free(tell_err);
				}

				decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

				current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
			}

			else if (strncmp(current_cmd->cmd, "yell", 4) == 0)
			{
				if (current_cmd->arg[1] == NULL)
				{
					char *ts = "Please give args.\n";
					write(client_fd->user_fd, ts, strlen(ts));
					fflush(stdout);
					decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					continue;
				}

				yell(client_fd, current_cmd->arg[1]);

				sign = 0;
				decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

				current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
			}

			else if (strncmp(current_cmd->cmd, "printenv", 8) == 0)
			{
				if (current_cmd->arg[1] == NULL)
				{
					char *ts = "Please give args.\n";
					write(client_fd->user_fd, ts, strlen(ts));
					fflush(stdout);
					decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					continue;
				}

				//char *env_name = malloc(strlen(current_cmd->arg[1]) - 1);
				//strncpy(env_name, current_cmd->arg[1], (strlen(current_cmd->arg[1]) - 1));
				char *env_val = getenv(current_cmd->arg[1]);//sooooock
				char *ret = malloc(strlen(env_val) + strlen(current_cmd->arg[1]) + 4);
				sprintf(ret, "%s=%s\n", current_cmd->arg[1], env_val);
				
				write(client_fd->user_fd, ret, strlen(ret));
				free_cmd(current_cmd);

				//last command, decress
				decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

				current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
			}

			else if (strncmp(current_cmd->cmd, "removeenv", 9) == 0)
			{
				if (current_cmd->arg[1] == NULL)
				{
					char *ts = "Please give args.\n";
					write(client_fd->user_fd, ts, strlen(ts));
					decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					continue;
				}

				setenv(current_cmd->arg[1], "", 1);
				free_cmd(current_cmd);

				//last command, decress
				decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

				current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
			}
			
			else if (strncmp(current_cmd->cmd, "exit", 4) == 0)
			{
				close(client_fd->user_fd);
				unlink_user(client_fd);
				user_exit_broadcast(client_fd);
				return -1;
			}

			else
			{
				char *envpath = getenv("PATH");
				if (check_cmd_exist(current_cmd->cmd, envpath) == -1)
				{
					char *unkown = "Unknown command: [";
					char *untail = "].\n";
					write(client_fd->user_fd, unkown, strlen(unkown));
					write(client_fd->user_fd, current_cmd->cmd, strlen(current_cmd->cmd));
					write(client_fd->user_fd, untail, strlen(untail));

					//last command, decress
					decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

					free_cmd(current_cmd);
					free_cmd_line(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
					continue;
				}	

				else
				{
					int exe_ret;
					exe_ret = execute_node(current_cmd, client_fd, &next_pipe_num);

					if (exe_ret != 0)
					{
						char *search_pip = malloc(sizeof(char) * 100);
						memset(search_pip, 0, 100);

						if (exe_ret == -1)
						{
							sprintf(search_pip, "*** Error: the pipe #%d->#%d does not exist yet. ***\n", current_cmd->pip_process_count_in, client_fd->ID);
							write(client_fd->user_fd, search_pip, strlen(search_pip));
							decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
						}

						else if (exe_ret == -2)
						{
							sprintf(search_pip, "*** Error: the pipe #%d->#%d already exist. ***\n", client_fd->ID, current_cmd->pip_process_count_out);
							write(client_fd->user_fd, search_pip, strlen(search_pip));
							decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
						}

						else if (exe_ret == -3)
						{
							sprintf(search_pip, "*** Error: user #%d does not exist. ***\n", client_fd->ID, current_cmd->pip_process_count_out);
							write(client_fd->user_fd, search_pip, strlen(search_pip));
							decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
						}

						else if (exe_ret == -4)
						{
							sprintf(search_pip, "*** Error: user #%d does not exist. ***\n", client_fd->ID, current_cmd->pip_process_count_in);
							write(client_fd->user_fd, search_pip, strlen(search_pip));
							decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));
						}

						else if (exe_ret == 5)
						{
							char *target = get_name(current_cmd->pip_process_count_in);
							sprintf(search_pip, "*** %s (#%d) just received from %s (#%d) by cmd ***", client_fd->name, client_fd->ID, target, current_cmd->pip_process_count_in);
							broadcast_message(client_fd, search_pip);
							char *target1 = get_name(current_cmd->pip_process_count_out);
							sprintf(search_pip, "*** %s (#%d) just piped cmd to %s (#%d) ***", client_fd->name, client_fd->ID, target1, current_cmd->pip_process_count_out);
							broadcast_message(client_fd, search_pip);
							sign = 0;
						}
						else if (exe_ret == 2)
						{
							char *target = get_name(current_cmd->pip_process_count_out);
							sprintf(search_pip, "*** %s (#%d) just piped cmd to %s (#%d) ***", client_fd->name, client_fd->ID, target, current_cmd->pip_process_count_out);
							broadcast_message(client_fd, search_pip);
							sign = 0;
						}

						else if (exe_ret == 1)
						{
							char *target = get_name(current_cmd->pip_process_count_in);
							sprintf(search_pip, "*** %s (#%d) just received from %s (#%d) by cmd ***", client_fd->name, client_fd->ID, target, current_cmd->pip_process_count_in);
							broadcast_message(client_fd, search_pip);
							sign = 0;
						}
						
						free(search_pip);
					}
					
					free_cmd(current_cmd);
					current_cmd = pull_cmd(&(client_fd->user_cmd_front), &(client_fd->user_cmd_rear));
				}
			}
		}

		if (sign)
			write(client_fd->user_fd, shellsign, strlen(shellsign));
	}

	return 0;
}

int execute_node(cmd_node *node, user_node *client_fd, int *next_n)
{
	int stdinfd = -1;
	int stdoutfd = -1;
	int stderrfd = -1;
	int pipe_n = 0;
	int pipe_err = 0;
	int ret_p = 0;

	if (node->is_init)
	{
		pipe_node *ch_node = NULL;
		ch_node = check(&(client_fd->user_pipe_front), 0);
		if (ch_node != NULL)
		{
			stdinfd = ch_node->infd;
			close(ch_node->outfd);
		}
	}

	else
		stdinfd = *next_n;

	if (node->is_pipe_in)
	{
		if (is_id_exist(node->pip_process_count_in) == -1)
			return -4;
		pipe_node *search = search_pipe(node->pip_process_count_in, client_fd->ID);
		printf("search from:%d\n", node->pip_process_count_in);
		printf("search to:%d\n", client_fd->ID);
		if (search == NULL)
		{
			return -1;
		}

		int temp_pipe[2];
		pipe(temp_pipe);
		write(temp_pipe[1], search->msg, 4096);
		close(temp_pipe[1]);
		stdinfd = temp_pipe[0];
		delete_pipe(search);
		ret_p = 1;
	}

	pipe_node *search = NULL;
	if (node->is_pipe_out)
	{
		if (is_id_exist(node->pip_process_count_out) == -1)
			return -3;
		printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
		search = search_pipe(client_fd->ID, node->pip_process_count_out);
		if (search == NULL)
		{
			search = malloc(sizeof(pipe_node));
			search->from = client_fd->ID;
			search->to = node->pip_process_count_out;
			
			int pipep[2];
			pipe(pipep);

			search->outfd = pipep[1];
			search->infd = pipep[0];
			search->next = NULL;

			printf("put node:%x\n", search);
			printf("front:%x\n", pipe_client_front);
		}

		else
		{
			printf("no NULL:%x\n", search);
			return -2;
		}
		stdoutfd = search->outfd;
		if (ret_p == 1)
			ret_p = 5;

		else
			ret_p = 2;
	}

	if (node->type == ISPIPE)
	{
		int pip[2];
		pipe(pip);
		stdoutfd = pip[1];
		*next_n = pip[0];
	}

	else if (node->type == ISREDIR)
	{
		int filefd = open(node->file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
		stdoutfd = filefd;
	}

	else if (node->type == ISPIPEN || node->type == ISPIPEERR)
	{
		pipe_node *pip_node = check(&(client_fd->user_pipe_front), node->pip_count);
		printf("count:%d\n", node->pip_count);

		if (pip_node == NULL)
		{
			pip_node = malloc(sizeof(pipe_node));
			pip_node->num = node->pip_count;

			//construct pipe
			int pipn[2];
			pipe(pipn);
			pip_node->infd = pipn[0];
			pip_node->outfd = pipn[1];
			pip_node->next = NULL;
			push_pipe(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear), &pip_node);
			printf("no find pipe infd:%d\n", pip_node->infd);
			printf("no find pipe outfd:%d\n", pip_node->outfd);
		}

		else
		{
			printf("find pipe infd:%d\n", pip_node->infd);
			printf("find pipe outfd:%d\n", pip_node->outfd);
		}
		
		stdoutfd = pip_node->outfd;
		pipe_n = 1;

		if (node->type == ISPIPEERR)
		{
			stderrfd = pip_node->outfd;
			pipe_err = 1;
		}
	}

	

	printf("cmd:%s\n", node->cmd);
	printf("type:%d\n", node->type);
	printf("infd:%d\n", stdinfd);
	printf("outfd:%d\n", stdoutfd);
	printf("errfd:%d\n", stderrfd);

	//last command, decress
	if (node->is_new)
		decress_count(&(client_fd->user_pipe_front), &(client_fd->user_pipe_rear));

	int pid = fork();

	if (pid == 0)
	{
		close(0);
		close(1);
		close(2);
		dup(client_fd->user_fd);
		dup(client_fd->user_fd);
		dup(client_fd->user_fd);
		close(client_fd->user_fd);

		if (stdinfd != -1)
		{
			close(0);
			dup(stdinfd);
			close(stdinfd);
		}

		if (stdoutfd != -1)
		{
			close(1);
			dup(stdoutfd);
			if (stderrfd == -1)
				close(stdoutfd);
		}

		if (stderrfd != -1)
		{
			close(2);
			dup(stderrfd);
			close(stderrfd);
		}

		execvp(node->cmd, node->arg);

	}

	else
	{

		if (stdinfd != -1)
		{
			close(stdinfd);
			printf("closed:%d\n", stdinfd);
		}

		if (stdoutfd != -1)
			if (!pipe_n)
			{
				close(stdoutfd);
				printf("closed:%d\n", stdoutfd);
			}


		int t;
		waitpid(pid, &t, 0);
		//pipe to shm
		if (node->is_pipe_out)
		{
			close(search->outfd);
			memset(search->msg, 0, 4096);
			read(search->infd, search->msg, 4096);
			put_pipe(search);
		}

	}
	printf("ret_p:%d\n", ret_p);
	return ret_p;
}

char *trick(char *str)
{
	char *ret = malloc(sizeof(char)*(strlen(str)-1));
	strncpy(ret, str, (strlen(str)-1));
	return ret;
}

int check_cmd_exist(char *cmd, char *path)
{
	size_t pnum;
	char **parray;
	char *full_path = malloc(sizeof(char) * 512);
	struct stat s;

	split(&parray, path, ":", &pnum);

	int16_t count;
	for (count = 0; count < pnum; count++)
	{
		sprintf(full_path, "%s/%s", parray[count], cmd);
		char *real_path = malloc(sizeof(char)*(strlen(full_path)-1));
		strncpy(real_path, full_path, (strlen(full_path)-1));
		
		if (stat(full_path, &s) != -1) //hooly shiiiiit
			return 1;
	}

	return -1;
}
