#include "revert_string.h"
#include <string.h>
#include <stdlib.h>

void RevertString(char *str)
{
	char* start = str;
	char* end = str + strlen(str) - 1;
	char temp;
	while (start < end) 
	{
		temp = *start;
		*start = *end;
		*end = temp;
		start++;
		end--;
	}
}

