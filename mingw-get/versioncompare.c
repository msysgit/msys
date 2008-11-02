
#include <string.h>

static int IsDigit(char ch)
{
	return (ch >= '0' && ch <= '9');
}

int VersionCompare(const char* v1, const char* v2)
{
	static int at1, at2, num1, num2;
	at1 = 0, at2 = 0;
	while (v1[at1] && v2[at2])
	{
		if (v1[at1] != v2[at2])
		{
			if (IsDigit(v1[at1]) && IsDigit(v2[at2]))
			{
				num1 = 0;
				while (IsDigit(v1[at1]))
				{
					num1 = num1 * 10 + v1[at1] - '0';
					++at1;
				}
				num2 = 0;
				while (IsDigit(v2[at2]))
				{
					num2 = num2 * 10 + v2[at2] - '0';
					++at2;
				}
				if (num1 != num2)
					return num1 - num2;
			}
			else
				return v1[at1] - v2[at2];
		}
		else
		{
			++at1;
			++at2;
		}
	}
	return v1[at1] - v2[at2];
}
