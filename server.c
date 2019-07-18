#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
//#include <signal.h>

typedef struct
{
	int id;
	int key;
	int class;//0管理员  1普通员工 
	char name[50];
	char job[50];
	int money;
	int order;//0 登录 1注册 2查询 3修改 4注销 5历史记录
	int pattern;
	int flag;
	char data[128];
}MSG;
typedef struct
{
	int id;
	char time[50];
	char behavior[50];
	char result[50];
}History;
void do_client(int acceptfd,sqlite3 *db);
void do_login(int acceptfd,MSG *msg,sqlite3 *db,History *history);
void do_register(int acceptfd,MSG *msg,sqlite3 *db,History *history);
void do_query(int acceptfd,MSG *msg,sqlite3 *db,History *history);
void do_querys(int acceptfd,MSG *msg,sqlite3 *db,History *history);
void do_alter(int acceptfd,MSG *msg,sqlite3 *db,History *history);
void do_delete(int acceptfd,MSG *msg,sqlite3 *db,History *history);
void do_history(int acceptfd,MSG *msg,sqlite3 *db,History *history);
void adhistory(MSG *msg,sqlite3 *db,History *history);
void gettime(char *gtime);
int main(int argc, const char *argv[])
{
	int pid;
	sqlite3 *db;
	int sockfd,acceptfd;
	if(sqlite3_open("my.db",&db) != SQLITE_OK)
	{
		printf("open my.db fail");
		exit(-1);
	}
	sockfd= socket(AF_INET,SOCK_STREAM,0);
	if(sockfd > 0)
	{
		printf("socket create success!!\n");
	}
	else
	{
		perror("fail create socket");
		exit(-1);
	}
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	if (bind(sockfd,(const struct sockaddr*)&addr,sizeof(addr)) == 0)
	{
		printf("bind success!!\n");
	}
	else
	{
		perror("bind fail!!");
		exit(-1);
	}

	if(listen(sockfd,5) == 0)
	{
		printf("listen success\n");
	}
	else
	{
		perror("listen fail");
		exit(-1);
	}
//	signal(SIGCHLD, SIG_IGN);
	while(1)
	{
		if((acceptfd = accept(sockfd,NULL,NULL)) < 0)
		{
			perror("accept fail");
			exit(-1);
		}
		else
		{
			printf("accept success\n");
		}

		if((pid = fork()) < 0)
		{
			perror("fail to fork");
			exit(-1);
		}
		if(pid == 0)
		{
			close(sockfd);
			printf("-------------\n");
			do_client(acceptfd,db);
		}
		else
		{
			close(acceptfd);
		}
	}
	return 0;
}
void do_client(int acceptfd,sqlite3 *db)
{
	MSG  msg;
	History history;
	while(recv(acceptfd,&msg,sizeof(MSG),0) > 0)
	{
		printf("id:%d  name:%s order:%d\n",msg.id,msg.name,msg.order);
		switch(msg.order)
		{
		case 0:
			do_login(acceptfd,&msg,db,&history);
			break;
		case 1:
			do_register(acceptfd,&msg,db,&history);
			break;
		case 2:
			do_query(acceptfd,&msg,db,&history);
			break;
		case 3:
			do_alter(acceptfd,&msg,db,&history);
			break;
		case 4:
			do_delete(acceptfd,&msg,db,&history);
			break;
		case 5:
			do_history(acceptfd,&msg,db,&history);
			break;
		case 6:
			do_querys(acceptfd,&msg,db,&history);
			break;
		}
	}
	printf("client quit\n");
	exit(0);
}

void do_register(int acceptfd,MSG *msg,sqlite3 *db,History *history)
{
	char sqlstr[128]={0};
	char *errmsg;
	printf("id:%d key:%d class:%d name:%s,job:%s money:%d\n",
			msg->id,msg->key,msg->class,msg->name,
			msg->job,msg->money);
	sprintf(sqlstr,"insert into staff values (%d,%d,%d,'%s','%s',%d)",
			msg->id,msg->key,msg->class,msg->name,msg->job,msg->money);
	if(sqlite3_exec(db,sqlstr,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		sprintf(msg->data,"注册失败");
		sprintf(history->result,"失败");
	}
	else
	{
		sprintf(msg->data,"注册成功");
		sprintf(history->result,"成功");
	}
	send(acceptfd,msg,sizeof(MSG),0);
	gettime(history->time);
	sprintf(history->behavior,"注册");
	adhistory(msg,db,history);
	return ;
}
void adhistory(MSG *msg,sqlite3 *db,History *history)
{
	char sqlstr[128]={0};
	char *errmsg;
	sprintf(sqlstr,"insert into history values(%d,'%s','%s','%s')",msg->id,
			history->time,history->behavior,history->result);
	if(sqlite3_exec(db,sqlstr,NULL,NULL,&errmsg) != SQLITE_OK)
		printf("history add fail\n");
	else
		printf("history add success\n");
}
void do_login(int acceptfd,MSG *msg,sqlite3 *db,History *history)
{
	char sqlstr[128]={0};
	char *errmsg,**result;
	int nrow,ncolumn;
	if(msg->pattern == 0)
		sprintf(sqlstr,"select * from staff where id = %d and key =%d",msg->id,msg->key);
 	else if(msg->pattern ==1) 
		sprintf(sqlstr,"select * from staff where id = %d and key =%d and class = 1",msg->id,msg->key);
	if(sqlite3_get_table(db,sqlstr,&result,
	   			&nrow,&ncolumn,&errmsg) != SQLITE_OK)
    	{
	    	printf("error:%s\n",errmsg);
	 	}
		if(nrow == 0)
		{
			if(msg->pattern == 0)
				sprintf(msg->data,"帐号密码不匹配!!!");
			else if(msg->pattern == 1)
				sprintf(msg->data," 帐号密码不匹配,或非管理员帐号!!!");
			sprintf(history->result,"失败");
		}
		else
		{
			sprintf(msg->data,"ok,登录成功.");
			sprintf(history->result,"成功");
		}
	gettime(history->time);
	sprintf(history->behavior,"登录");
	adhistory(msg,db,history);
	sqlite3_free_table(result);
	send(acceptfd,msg,sizeof(MSG),0);
	return ; 
}
void do_query(int acceptfd,MSG *msg,sqlite3 *db,History *history)
{
	char sqlstr[128]={0};
	char *errmsg,**result;
	int ncolumn,nrow,i,j;
	sprintf(sqlstr,"select * from staff where id=%d",msg->id);
	if(sqlite3_get_table(db,sqlstr,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK)
	{
		printf("error:%s\n",errmsg);
	}
	for(i = 1;i<nrow+1;i++)
	{
		sprintf(msg->data,"%s\t%s\t%s\t%s\t%s\t%s\t",result[i*ncolumn+0],
				result[i*ncolumn+1],result[i*ncolumn+2],result[i*ncolumn+3],
				result[i*ncolumn+4],result[i*ncolumn+5]);
	}
	send(acceptfd,msg,sizeof(MSG),0);
	gettime(history->time);
	sprintf(history->result,"%d条",nrow);
	sprintf(history->behavior,"查询");
	adhistory(msg,db,history);
	sqlite3_free_table(result);
	return ;
}
void do_querys(int acceptfd,MSG *msg,sqlite3 *db,History *history)
{
	char sqlstr[128]={0};
	char *errmsg,**result;
	int ncolumn,nrow,i,j;
	if(msg->pattern == 0)
		sprintf(sqlstr,"select * from staff where id=%d",msg->id);
	else if(msg->pattern == 1)
		sprintf(sqlstr,"select * from staff where name='%s'",msg->name);
	else if(msg->pattern == 2)
		sprintf(sqlstr,"select * from staff where job='%s'",msg->job);
	else if(msg->pattern == 3)
		sprintf(sqlstr,"select * from staff ");
	if(sqlite3_get_table(db,sqlstr,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK)
	{
		printf("error:%s\n",errmsg);
	}
	if(nrow > 0)
	{
		for(i = 1;i<nrow+1;i++)
		{
			sprintf(msg->data,"%s\t%s\t%s\t%s\t%s\t%s\t",result[i*ncolumn+0],
					result[i*ncolumn+1],result[i*ncolumn+2],result[i*ncolumn+3],
					result[i*ncolumn+4],result[i*ncolumn+5]);
			if(i == nrow)
			msg->flag = 1;
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	else
	{
		sprintf(msg->data,"无满足条件对象");
		msg->flag = 1;
		send(acceptfd,msg,sizeof(MSG),0);
	}
	gettime(history->time);
	sprintf(history->result,"%d条",nrow);
	sprintf(history->behavior,"查询");
	adhistory(msg,db,history);
	sqlite3_free_table(result);
	return ;
}

void do_alter(int acceptfd,MSG *msg,sqlite3 *db,History *history)
{
	char sqlstr[128],*errmsg;
	sprintf(sqlstr,"update staff set key=%d,class=%d,name='%s',job='%s',money=%d where id=%d",
			msg->key,msg->class,msg->name,msg->job,msg->money,msg->id);
	if(sqlite3_exec(db,sqlstr,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		sprintf(msg->data,"修改失败");
		sprintf(history->result,"失败");
	}
	else
	{
		sprintf(msg->data,"修改成功");
		sprintf(history->result,"成功");
	}
	send(acceptfd,msg,sizeof(MSG),0);
	gettime(history->time);
	sprintf(history->behavior,"修改");
	adhistory(msg,db,history);
	return ;

}
void do_delete(int acceptfd,MSG *msg,sqlite3 *db,History *history)
{
	char sqlstr[128],*errmsg;
	sprintf(sqlstr,"delete from staff where id=%d",msg->id);
	if(sqlite3_exec(db,sqlstr,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		sprintf(msg->data,"删除失败");
		sprintf(history->result,"失败");
	}
	else
	{
		sprintf(msg->data,"删除成功");
		sprintf(history->result,"成功");
	}
	send(acceptfd,msg,sizeof(MSG),0);
	gettime(history->time);
	sprintf(history->behavior,"删除");
	adhistory(msg,db,history);
	return ;
}

void gettime(char dtime[])
{
	time_t t;
	struct tm *tp;
	time(&t);
	tp = localtime(&t);

	sprintf(dtime,"%d-%d-%d %d:%d:%d",tp->tm_year + 1900,
			tp->tm_mon + 1,tp->tm_mday,tp->tm_hour,tp->tm_min,
			tp->tm_sec);
	return ;
}


void do_history(int acceptfd,MSG *msg,sqlite3 *db,History *history)
{
	char sqlstr[128]={0};
	char *errmsg,**result;
	int ncolumn,nrow,i,j;
	sprintf(sqlstr,"select * from history ");
	if(sqlite3_get_table(db,sqlstr,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK)
	{
		printf("error:%s\n",errmsg);
	}
	if(nrow>30)
		i=nrow -30;
	else
		i=1;
	for( ;i<nrow+1;i++)
	{
		sprintf(msg->data,"%s\t%s\t%s\t%s\t",result[i*ncolumn+0],
				result[i*ncolumn+1],result[i*ncolumn+2],result[i*ncolumn+3]);
		if(i == nrow)
		msg->flag = 1;
		send(acceptfd,msg,sizeof(MSG),0);
	}
	sqlite3_free_table(result);
	return ;
}
