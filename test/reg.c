#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include "../util.h"

int main()
{
	char *buf = "cat <3";
	char *cmp = ">[[:digit:]]+";
	int res;

	res = regular_match(buf, cmp);
	printf("%d\n", res);
}
