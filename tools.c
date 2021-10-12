#include <getch.h>
#include "tools.h"

static void clean_stdin(void)
{
	stdin->_IO_read_ptr = stdin->_IO_read_end;	
}

//	从键盘获取指定长度的字符串
char* get_str(char* str,size_t hope_len)
{
	if(NULL == str || 0 == hope_len)
		return NULL;
	
	clean_stdin();

	size_t index = 0;
	while(index < hope_len-1)
	{
		char key_val = getch();
		if(10 == key_val) break;
		if(127 == key_val)
		{
			if(0 < index)
			{
				printf("\b \b");
				index--;
			}
			continue;
		}
		printf("%c",key_val);
		str[index++] = key_val;
	}
	str[index] = '\0';
	printf("\n");
	clean_stdin();
	return str;
}

//	从键盘获取密码
char* get_passwd(char* pd,size_t hope_len,bool is_show)
{
	if(NULL == pd || 0 == hope_len) return NULL;

	clean_stdin();

	size_t index = 0;
	while(index < hope_len-1)
	{
		char key_val = getch();
		if(127 == key_val)
		{
			if(0 < index)
			{
				index--;
				if(is_show) printf("\b \b");
			}
			continue;
		}
		if(10 == key_val) break;
		pd[index++] = key_val;
		if(is_show) printf("*");
	}
	pd[index] = '\0';
	printf("\n");
	clean_stdin();
	return pd;
}








