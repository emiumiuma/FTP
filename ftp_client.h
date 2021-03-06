#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include <stdint.h>

#define BUF_SIZE (4096)

typedef struct sockaddr* SP;

typedef struct FTPClient
{
	int cli_sock;	//	命令通道socket描述符
	int cli_pasv;	//	数据通道socket描述符
	char ip[16];	//	服务器传回ip地址
	uint16_t port;	//	端口号
	char path[256];	//	服务器当前工作路径
	int status;		//	返回码
	char* buf;		//	返回执行结果
}FTPClient;

//	创建FTP客户端对象
FTPClient* create_FTPClient(const char* ip,uint16_t port);

//	销毁FTP客户端对象
void destory_FTPClient(FTPClient* ftp);

//	向服务器发送用户名
void user_FTPClient(FTPClient* ftp,const char* user);

//	向服务器发送密码
void pass_FTPClient(FTPClient* ftp,const char* pass);

//	pwd命令
void pwd_FTPClient(FTPClient* ftp);

//	cd命令
void cd_FTPClient(FTPClient* ftp,const char* path);

//	mkdir命令
void mkdir_FTPClient(FTPClient* ftp,const char* dir);

//	ls命令
void ls_FTPClient(FTPClient* ftp);

//	get命令
void get_FTPClient(FTPClient* ftp,const char* filename);

//	put命令
void put_FTPClient(FTPClient* ftp,const char* filename);

//	bye命令
void bye_FTPClient(FTPClient* ftp);

#endif//FTP_CLIENT_H


