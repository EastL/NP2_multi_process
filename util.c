#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

void split(char ***arr, char *str, const char *del, size_t *count)
{
	size_t _count = 0;
	char buf[10010];
	char **_arr = malloc(sizeof(char*) * 5000);
	memset(_arr, 0, sizeof(char*) * 5000);

	strncpy(buf, str, 10010);

	char *save;
	char *s = strtok_r(buf, del, &save);

	while (s != NULL)
	{
		//printf("util:%s\n", s);
		size_t __count = strlen(s);
		*(_arr + _count) = malloc(sizeof(char) * (__count + 1));
		memset(*(_arr + _count), 0, (__count + 1));
		strcpy(*(_arr + _count), s);
		s = strtok_r(NULL, del, &save);
		_count++;
	}

	*count = _count;
	*arr = _arr;
}

int regular_match(char *str, char *reg_str)
{
	regex_t reg;
	int status;
	int cflags = REG_EXTENDED;
	regmatch_t pmatch[1];
	const size_t nmatch = 1;
	int err;

	if ((err = regcomp(&reg, reg_str, cflags)) != 0)
	{
		//char *err_buf;
		//regerror(err, &reg, err_buf);
		regfree(&reg);
		return -1;
	}

	if ((status = regexec(&reg, str, nmatch, pmatch, 0)) < 0)
	{
		regfree(&reg);
		return -1;
	}

	if (status == REG_NOMATCH)
		return 0;
	else
		return 1;	
}
