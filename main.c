#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftp_client.h"
#include "tools.h"

int main(int argc,const char* argv[])
{
	//	创建FTP客户端对象
	FTPClient* ftp = NULL;
	if(2 == argc)
	{
		ftp = create_FTPClient(argv[1],21);	
	}
	else if(3 == argc)
	{
		uint16_t port = atoi(argv[2]);
		ftp = create_FTPClient(argv[1],port);	
	}
	else
	{
		printf("User: ./ftp xxx.xxx.xxx.xxx [port]\n");
		return EXIT_SUCCESS;
	}
	

	//	输入用户名并发送
	char user[20] = {};
	printf("Name ");
	get_str(user,sizeof(user));
	user_FTPClient(ftp,user);

	//	输入密码并登陆
	char pass[20] = {};
	printf("PASS ");
	get_passwd(pass,sizeof(pass),0);
	pass_FTPClient(ftp,pass);

	//	循环地输入命令、执行命令
	char cmd[256] = {};
	for(;;)
	{
		printf("ftp>");
		get_str(cmd,sizeof(cmd));
	
		if(0 == strncmp(cmd,"cd ",3))
		{
			cd_FTPClient(ftp,cmd+3);	
		}
		else if(0 == strcmp(cmd,"ls"))
		{
			ls_FTPClient(ftp);
		}
		else if(0 == strcmp(cmd,"pwd"))
		{
			pwd_FTPClient(ftp);	
		}
		else if(0 == strncmp(cmd,"mkdir ",6))
		{
			mkdir_FTPClient(ftp,cmd+6);
		}
		else if(0 == strncmp(cmd,"put ",4))
		{
			put_FTPClient(ftp,cmd+4);	
		}
		else if(0 == strncmp(cmd,"get ",4))
		{
			get_FTPClient(ftp,cmd+4);	
		}
		else if(0 == strcmp(cmd,"bye"))
		{
			bye_FTPClient(ftp);
			destory_FTPClient(ftp);
			return EXIT_SUCCESS;
		}
		else if('!' == cmd[0])
		{
			//	ftp命令前加 ! 相当于执行系统命令
			system(cmd+1);	
		}
		else
		{
			printf("指令未定义!\n");
		}
	}
}



