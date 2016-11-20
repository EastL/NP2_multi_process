#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "../util.h"
#include "../token.h"

int main()
{
	char buf[] = "\ncat";
	char *del = " ";
	char **array;
	char *save;
	
	int res;
	int c;

	c = get_token_type(buf);
	printf("%d\n", c);
}
