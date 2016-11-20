#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "../util.h"

int main()
{
	char buf[] = "cat  ls  cd  aaa |4";
	char *del = " ";
	char **array;
	char *save;
	
	int res;
	int c;

	split(&array, buf, del, &c);
	printf("%s\n", array[1]);
}
