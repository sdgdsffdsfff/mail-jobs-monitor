#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jobs.h"
//LOG_TAG

#ifdef DEBUG
#define LOGI(str) printf("%s\n",str);
#define LOGE(str) printf("%s\n",str);
#else
#define LOGI(str) ;
#define LOGE(str) ;
#endif
#define BUFFSIZE 1024*1024

char request[BUFFSIZE], text[5*1024*1024];

int httpGet(char* hostname,char *url)
{
	LOGI("httpGet");
	LOGI(hostname);
	//char myurl[BUFFSIZE] = {0};
	//char host[BUFFSIZE] = {0};
	//char GET[BUFFSIZE] = {0};
	struct sockaddr_in sin;
	int sockfd;
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
		LOGE("httpget create socket failed !");
		return -100;
	}

	struct hostent * host_addr = gethostbyname(hostname);
	if(host_addr==NULL) {
		LOGE("httpget Unable to locate host");
		return -103;
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons( (unsigned short)80);
	sin.sin_addr.s_addr = *((unsigned long*)host_addr->h_addr_list[0]);
	if( connect (sockfd,(const struct sockaddr *)&sin, sizeof(struct sockaddr_in) ) == -1 ) {
		LOGE("httpget connect failed !");
		return -101;
	}
	LOGI("httpGet send");
	// 向WEB服务器发送URL信息
	memset(request, 0, BUFFSIZE);
	strcat(request, url); //请求内容与http版本
	//strcat(request, "GET /index.html HTTP/1.1\r\n"); //请求内容与http版本
	strcat(request, "HOST:"); //主机名，，格式："HOST:主机"
	strcat(request, hostname);
	strcat(request, "\r\n");
	strcat(request, "Accept:*/*\r\n"); //接受类型，所有类型
	// strcat(request, "User-Agent:Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)");//指定浏览器类型？
	// strcat(request, "Connection: Keep-Alive\r\n");//设置连接，保持连接
	// strcat(request, "Set Cookie:0\r\n");//设置Cookie
	// strcat(request, "Range: bytes=0 - 500\r\n");//设置请求字符串起止位置，断点续传关键"Range: bytes=999 -"
	strcat(request, "\r\n");//空行表示结束
	LOGI(request);
	if( send (sockfd, request, strlen(request), 0) == -1){
		LOGE("httpget send failed");
		return -99;
	}
	LOGI("httpGet recv");
	memset(text,0,BUFFSIZE);
	int recv_size = 0,all_size = 0;
	while((recv_size = recv (sockfd, text + all_size, 1024*1024*1024, 0)) != 0){
		all_size += recv_size;
		if(all_size > 5*1024*1024)
			break;
	}
	LOGI(text);
	LOGI("httpGet end");
	return 0;
}

float price_list[1024];
char mail[5*1024];
int need_send;

void get_text()
{
	memset(price_list,0,sizeof(float) * 1024);
	memset(mail,0,5*1024);
	need_send = 0;
	int i = 0;
	char *p = text;
	while(1){
		char* c = strstr(p, "<ul class=\"pdlist_unitprice\">");
		if(c == NULL)
			break;
		price_list[i++] = atof(c + 44);
		p = c + 44;
	}
	for(i = 0;i != 1024;++i){
		if(price_list[i] == 0)
			break;
		if(price_list[i] >= 25)
			need_send = 1;
		sprintf(mail,"%s%f\n",mail,price_list[i]);
	}
	return ;
}

void send_mail()
{
	char send_command[5*1024 + 1024];
	memset(send_command,0,6*1024);
	if(need_send == 0){
		sprintf(send_command,"echo \"%s\" | mailx -A qq1 -v -s \"send from job monitor\" 541877075@qq.com",mail);
		//printf("%s\n",send_command);
		system(send_command);
	}
}

static void* mail_job_monitor(time_t job_time,void *arg) 
{
	httpGet("s.5173.com","GET /H2-xptjnl-522v1q-ae4tn2-0-wu0tw4-0-0-0-a-a-a-a-a-0-itemprice_asc-0-0.shtml HTTP/1.1\r\n");
	get_text();
	send_mail();
	return NULL;
}
int main()
{
	struct job job;
	job_service(&job);
	job.call = mail_job_monitor;
	//mail_job_monitor(0,NULL);
	sleep(1000000000);
	return 0;
}