all:	
	gcc server.c user.c shell.c command.c parse.c token.c util.c pipe.c message.c -o rashell
	#gcc -g -fsanitize=address server.c user.c shell.c command.c parse.c token.c util.c pipe.c message.c -o rashell
