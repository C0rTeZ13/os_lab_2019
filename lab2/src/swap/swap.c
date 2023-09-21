#include "swap.h"

void Swap(char *left, char *right)
{
	char memory = *left;
	*left = *right;
	*right = memory;
}
