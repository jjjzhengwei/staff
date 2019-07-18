#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

typedef struct
{
	int id;
	int key;
	int class;//0管理员  1普通员工 
	char name[50];
	char job[50];
	int money;
	int order;//0 员工登录 1注册 2查询 3修改 4注销 5管理员登录
	int pattern;
	int flag;
	char data[128];
}MSG;

void do_register(int sockfd,MSG *msg);
int do_login(int sockfd, MSG *msg);
void do_query(int sockfd, MSG *msg);
void do_querys(int sockfd, MSG *msg);
void do_delete(int sockfd,MSG *msg);
void do_alter(int sockfd,MSG *msg);
void do_history(int sockfd,MSG *msg);
int main(int argc, const char *argv[])
{
	int n;
	char c[128];
	MSG msg;
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd > 0)
	{
	//	printf("sockfd create success!!\n");
	}
	else
	{
		perror("sockfd fail !!");
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(sockfd,(const struct sockaddr*)&addr,sizeof(addr)) == 0)
	{
	//	printf("connect successn\n");
	}
	else 
	{
		perror("connect fail");
	}
start:
	while(1){
		printf("**************************************************\t\n");
		printf("*******请选择：0普通登录 1管理员登录 2关闭********\t\n");
		printf("**************************************************\t\n");
		printf("选择：");
		if(scanf("%d",&n) <= 0)
		{
			scanf("%[^\n]",c);
			n=-1;
		}
		if(n<0||n>2)
		{
			printf("请正确输入！！！\n");
			goto start;
		}
		switch(n)
		{
		case 0:
			msg.pattern = 0;
			if(do_login(sockfd,&msg) == 1)
			{
				goto next;
			}
			break;
		case 1:
			msg.pattern = 1;
			if(do_login(sockfd,&msg) == 1)
			{
				goto adm;
			}
			break;
		case 2:
			close(sockfd);
			exit(1);
		}
	}
next:
	while(1)
	{
		printf("***********************************************\t\n");
		printf("** 请输入：0登出  1查询个人信息  2 退出系统  **\t\n");
		printf("***********************************************\t\n");
		printf("选择：");
		if(scanf("%d",&n) <= 0)
		{
			scanf("%[^\n]",c);
			n=-1;
		}
		if(n<0||n>2)
		{
			printf("请正确输入！！！\n");
			goto next;
		}
		switch(n)
		{
		case 0:
			goto start;
			break;
		case 1:
			do_query(sockfd,&msg);
			break;
		case 2:
			close(sockfd);
			exit(1);
		}
	}
adm:
	while(1)
	{
		printf("*******************************************************\t\n");
		printf("* 请输入：0 登出 1查询 2删除 3修改 4添加员工 5历史记录*\t\n");
		printf("* 6退出系统                                           *\t\n");
		printf("*******************************************************\t\n");
		printf("选择：");
		if(scanf("%d",&n) <= 0)
		{
			scanf("%[^\n]",c);
			n=-1;
		}
		if(n<0||n>6)
		{
			printf("请正确输入！！！\n");
			goto adm;
		}
		switch(n)
		{
		case 0:
			goto start;
			break;
		case 1:
			goto adm_query;
			break;
		case 2:
			do_delete(sockfd,&msg);
			break;
		case 3:
			 do_alter(sockfd,&msg);
			break;
		case 4:
			do_register(sockfd,&msg);
			break;
		case 5:
			do_history(sockfd,&msg);
			break;
		case 6:
			close(sockfd);
			exit(1);
		}
	}
adm_query:
	while(1)
	{
		printf("***************************************************************\t\n");
		printf("* 请输入：0返回 1个人信息查询 2按id查询 3按name查询 4按job查询*\t\n");
		printf("* 请输入: 5所有员工信息                                       *\t\n");
		printf("****************************************************************\t\n");
		printf("选择：");
		if(scanf("%d",&n) <= 0)
		{
			scanf("%[^\n]",c);
			n=-1;
		}
		if(n<0||n>5)
		{
			printf("请正确输入！！！\n");
			goto adm_query;
		}
		switch(n)
		{
		case 0:
			goto adm;
			break;
		case 1:
			do_query(sockfd,&msg);
			break;
		case 2:
			msg.pattern = 0;
			do_querys(sockfd,&msg);
			break;
		case 3:
			msg.pattern = 1;
			do_querys(sockfd,&msg);
			break;
		case 4:
			msg.pattern = 2;
			do_querys(sockfd,&msg);
			break;
		case 5:
			msg.pattern = 3;
			do_querys(sockfd,&msg);
			break;
		}
	}
	close(sockfd);
	return 0;
}
void do_register(int sockfd,MSG *msg)
{
	msg->order = 1;
	printf("输入员工id：");
	scanf("%d",&(msg->id));
	printf("密码：");
	scanf("%d",&(msg->key));
	printf("员工姓名：");
	scanf("%s",msg->name);
	printf("员工职位：");
	scanf("%s",msg->job);
	printf("薪资：");
	scanf("%d",&(msg->money));
	printf("员工是否拥有管理员权限(1有/0无):");
	scanf("%d",&(msg->class));
	send(sockfd,msg,sizeof(MSG),0);

	recv(sockfd,msg,sizeof(MSG),0);

	printf("%s\n",msg->data);
	return;
}
int do_login(int sockfd,MSG *msg)
{
	msg->order = 0;
	printf("输入id：");
	scanf("%d",&(msg->id));
	printf("输入密码：");
	scanf("%d",&(msg->key));
	send(sockfd,msg,sizeof(MSG),0);

	recv(sockfd,msg,sizeof(MSG),0);

	printf("%s\n",msg->data);
	if(strncmp(msg->data,"ok",2) == 0)
		return 1;
	else
		return 0;
}

void do_query(int sockfd,MSG *msg)
{
	msg->order = 2;
	send(sockfd,msg,sizeof(MSG),0);

	recv(sockfd,msg,sizeof(MSG),0);
	printf("工号\t密码\t管理员\t姓名\t工作\t工资\n");
	printf("%s\n",msg->data);
	return ;
}

void do_querys(int sockfd,MSG *msg)
{
	int oldint;
	char oldchar[50];
	if(msg->pattern == 0)
	{
		oldint = msg->id;
		printf("请输入id号：");
		scanf("%d",&(msg->id));
	}
	else if(msg->pattern ==1)
	{
		sprintf(oldchar,"%s",msg->name);
		printf("请输入查询的姓名：");
		scanf("%s",msg->name);
	}
	else if(msg->pattern == 2)
	{
		sprintf(oldchar,"%s",msg->job);
		printf("请输入查询的职位：");
		scanf("%s",msg->job);
	}
	msg->order = 6;
	msg->flag = 0;
	send(sockfd,msg,sizeof(MSG),0);

	printf("工号\t密码\t管理员\t姓名\t工作\t工资\n");	
	while(msg->flag == 0)
	{
		recv(sockfd,msg,sizeof(MSG),0);
		printf("%s\n",msg->data);
	}
	if (msg->pattern == 0)
		msg->id = oldint;
	else if(msg->pattern == 1)
		sprintf(msg->name,"%s",oldchar);
	else if(msg->pattern == 2)
		sprintf(msg->job,"%s",oldchar);
	return ;
}

void do_delete(int sockfd,MSG *msg)
{
	int oldint;
	oldint = msg->id;
	printf("将被删除员工的id:");
	scanf("%d",&(msg->id));
	msg->order = 4;
	
	send(sockfd,msg,sizeof(MSG),0);

	recv(sockfd,msg,sizeof(MSG),0);
	printf("%s\n",msg->data);
	msg->id = oldint;
	return;
}

void do_alter(int sockfd,MSG *msg)
{
	int oldint;
	oldint = msg->id;
	printf("输入修改员工id：");
	scanf("%d",&(msg->id));
	printf("新密码：");
	scanf("%d",&(msg->key));
	printf("员工新姓名：");
	scanf("%s",msg->name);
	printf("员工新职位：");
	scanf("%s",msg->job);
	printf("新薪资：");
	scanf("%d",&(msg->money));
	printf("是否拥有管理员权限（1有/0无）：");
	scanf("%d",&(msg->class));
	msg->order = 3;
	send(sockfd,msg,sizeof(MSG),0);

	recv(sockfd,msg,sizeof(MSG),0);

	printf("%s\n",msg->data);
	msg->id = oldint;
	return;
}

void do_history(int sockfd,MSG *msg)
{
	msg->order = 5;
	msg->flag = 0;
	send(sockfd,msg,sizeof(MSG),0);
	printf("id\t      时间     \t        行为\t结果\n");	
	while(msg->flag == 0)
	{
		recv(sockfd,msg,sizeof(MSG),0);
		printf("%s\n",msg->data);
	}
	return ;
}

