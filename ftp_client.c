#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <getch.h>
#include "ftp_client.h"

int check_status(FTPClient* ftp,int status,bool is_die)
{
	size_t ret = recv(ftp->cli_sock,ftp->buf,BUF_SIZE,0);	
	if(0 == ret)
	{
		perror("recv");
		free(ftp->buf);
		free(ftp);
		exit(EXIT_FAILURE);
	}

	ftp->buf[ret] = '\0';
	printf("%s",ftp->buf);

	sscanf(ftp->buf,"%d",&ftp->status);
	if(ftp->status != status && is_die)
	{
		free(ftp->buf);
		free(ftp);
		exit(EXIT_FAILURE);
	}
	return ftp->status;
}

void send_cmd(FTPClient* ftp)
{
	int ret = send(ftp->cli_sock,ftp->buf,strlen(ftp->buf),0);	
	if(0 >= ret)
	{
		perror("send");
		destory_FTPClient(ftp);
		exit(EXIT_FAILURE);
	}
}

//	创建FTP客户端对象
FTPClient* create_FTPClient(const char* ip,uint16_t port)
{
	FTPClient* ftp = malloc(sizeof(FTPClient));
	ftp->cli_sock = socket(AF_INET,SOCK_STREAM,0);
	if(0 > ftp->cli_sock)
	{
		free(ftp);
		perror("socket");
		return NULL;
	}
	
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	socklen_t addrlen = sizeof(addr);

	if(connect(ftp->cli_sock,(SP)&addr,addrlen))
	{
		free(ftp);
		perror("connect");
		return NULL;
	}
	
	ftp->buf = malloc(BUF_SIZE);

	check_status(ftp,220,true);

	return ftp;
}

//	销毁FTP客户端对象
void destory_FTPClient(FTPClient* ftp)
{
	printf("%s %d %s\n",__FILE__,__LINE__,__func__);
	close(ftp->cli_sock);
	free(ftp->buf);
	free(ftp);
}

//	向服务器发送用户名
void user_FTPClient(FTPClient* ftp,const char* user)
{
	sprintf(ftp->buf,"USER %s\n",user);
	send_cmd(ftp);
	check_status(ftp,331,true);
}

//	向服务器发送密码
void pass_FTPClient(FTPClient* ftp,const char* pass)
{
	sprintf(ftp->buf,"PASS %s\n",pass);
	send_cmd(ftp);
	check_status(ftp,230,true);
/*
	sprintf(ftp->buf,"OPTS UTF8 OFF\n");
	send_cmd(ftp);
	check_status(ftp,200,false);
	*/
}

//	pwd命令
void pwd_FTPClient(FTPClient* ftp)
{
	sprintf(ftp->buf,"PWD\n");
	send_cmd(ftp);
	check_status(ftp,257,true);

	//	获取出工作目录，是为了后期界面显示
	sscanf(ftp->buf,"257 \"%s",ftp->path);
	ftp->path[strlen(ftp->path)-1] = '\0';

//	printf("path:%s\n",ftp->path);
}

//	cd命令
void cd_FTPClient(FTPClient* ftp,const char* path)
{
	sprintf(ftp->buf,"CWD %s\n",path);
	send_cmd(ftp);
	check_status(ftp,250,false);
}

//	mkdir命令
void mkdir_FTPClient(FTPClient* ftp,const char* dir)
{
	sprintf(ftp->buf,"MKD %s\n",dir);
	send_cmd(ftp);
	check_status(ftp,257,false);
}

void pasv_FTPClient(FTPClient* ftp)
{
	sprintf(ftp->buf,"PASV\n");
	send_cmd(ftp);
	check_status(ftp,227,true);

	uint8_t ip1,ip2,ip3,ip4,port1,port2;
	sscanf(ftp->buf,"227 Entering Passive Mode (%hhu,%hhu,%hhu,%hhu,%hhu,%hhu)",&ip1,&ip2,&ip3,&ip4,&port1,&port2);

	sprintf(ftp->ip,"%hhu.%hhu.%hhu.%hhu",ip1,ip2,ip3,ip4);
	ftp->port = (port1 << 8) + port2;

	ftp->cli_pasv = socket(AF_INET,SOCK_STREAM,0);
	if(0 > ftp->cli_pasv)
	{
		perror("socket");
		return;
	}

	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ftp->port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if(connect(ftp->cli_pasv,(SP)&addr,sizeof(addr)))
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}
}

//	ls命令
void ls_FTPClient(FTPClient* ftp)
{
	pasv_FTPClient(ftp);
	sprintf(ftp->buf,"LIST -al\n");
	send_cmd(ftp);

	check_status(ftp,150,true);

	size_t ret = 0;
	while((ret = recv(ftp->cli_pasv,ftp->buf,BUF_SIZE-1,0)))
	{
		ftp->buf[ret] = '\0';
		printf("%s",ftp->buf);	
	}
	
	close(ftp->cli_pasv);
	check_status(ftp,226,true);
}


//	get命令
void get_FTPClient(FTPClient* ftp,const char* filename)
{
	pasv_FTPClient(ftp);

	sprintf(ftp->buf,"RETR %s\n",filename);
	send_cmd(ftp);

	check_status(ftp,150,false);
	if(150 == ftp->status)
	{
		int fd = open(filename,O_WRONLY|O_CREAT|O_EXCL,0644);
		if(0 > fd)
		{
			printf("本地已存在%s文件,是否覆盖?(y/n)\n",filename);
			char cmd = getch();
			printf("%c\n",cmd);
			if('y' != cmd && 'Y' != cmd)
			{
				printf("放弃下载!\n");
				close(ftp->cli_pasv);
				return;
			}
			fd = open(filename,O_WRONLY|O_TRUNC);
		}
		
		size_t ret = 0;
		while((ret = recv(ftp->cli_pasv,ftp->buf,BUF_SIZE,0)))
		{
			write(fd,ftp->buf,ret);	
		}

		close(fd);
		close(ftp->cli_pasv);

		check_status(ftp,226,false);
	}
}

//	put命令
void put_FTPClient(FTPClient* ftp,const char* filename)
{
	printf("%s %d %s\n",__FILE__,__LINE__,__func__);
}

//	bye命令
void bye_FTPClient(FTPClient* ftp)
{
	sprintf(ftp->buf,"QUIT\n");
	send_cmd(ftp);
	check_status(ftp,221,true);
}

