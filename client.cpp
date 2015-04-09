//OC - Ubuntu

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;


#define MAXLINE 1024
#define MAXHEADERSLINE  200



//*****функция получения ip по host для клиента******************

char *get_ip(char *host)
{
  	struct hostent *hent;
  	char ip[15];  //XXX.XXX.XXX.XXX
  	char **pptr; //
	
  	if((hent = gethostbyname(host)) == NULL){
    	printf ("Can't get IP");
	   	exit(1);
  	}
  	printf("hostname: %s\n", hent->h_name);
 	
	
	//if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, sizeof(ip)) == NULL){
    //	perror ("Can't resolve host");
    //	exit(1);
  	//}
  	
	
	if (hent->h_addrtype == AF_INET  && (pptr = hent->h_addr_list) != NULL) {
		printf("IP: %s\n", inet_ntop(hent->h_addrtype, *pptr, ip,  sizeof(ip)));
	} 
	else {
		printf("Error call inet_ntop \n");
	}
  	
  	return ip;
} 


//************отображения для клиента***********************************

ssize_t process_http(int sockfd, char *host, char *page, char *poststr)
{
	char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
	ssize_t n;
	
	snprintf(sendline, MAXHEADERSLINE,
		 "HEAD %s HTTP/1.0\r\n"
		 "Host: %s\r\n\r\n",
		  page, host);
	/*snprintf(sendline, MAXHEADERSLINE,
		 "GET %s HTTP/1.0\r\n"
		 "Host: %s\r\n"
		 "Content-type: application/x-www-form-urlencoded\r\n"
		 "Content-length: %d\r\n\r\n"
		 "%s", page, host, strlen(poststr), poststr);
	*/
	/*snprintf(sendline, MAXHEADERSLINE,
		 "POST %s HTTP/1.0\r\n"
		 "Host: %s\r\n"
		 "Content-type: application/x-www-form-urlencoded\r\n"
		 "Content-length: %d\r\n\r\n"
		 "%s", page, host, strlen(poststr), poststr);
*/
	write(sockfd, sendline, strlen(sendline));
	while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = '\0';
		printf("%s", recvline);
	}
	return n;

}


int main (int argc, char **argv) {

    //**********************************************************************************
    //char *host;
    //char *page;
    
    
    char *ip;
	int port;
	
    //********************test**********************************************************
	//char *host = "posttestserver.com";
	char *page = "/";
	//char *poststr = "Hello\r\n";
	//char *host = "httpbin.org";
	//char *page = "/post";
	char *poststr = "Hello\r\n";
 	//**********************************************************************************
	
	struct sockaddr_in SockAddr;
	int sockfd;
	
	//******************получение аргументов командной строки***************************
	/*
	if(argc == 1){
    	printf("USAGE: required <port> <host> <page>\n\
		\tport: port number\n\
		\thost: the website hostname\n\
		\tpage: the page to retrieve. default: /\n");
    	return 2;
  	}
  	
  	
  	port=atoi(argv[1]);
	host=argv[2];
	//page=argv[3];
	ip=host;
  	*/
	//***********************************************************************************
  	port=3101; //test
  	ip="127.0.0.1"; //ip
  	//ip = get_ip(host);
	//printf("IP is %s\n", ip);
	
	//***********************************************************************************
  	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //(,,0)
	if(sockfd == -1)	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}
	
	bzero(&SockAddr, sizeof(SockAddr));
	
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(port); 
	
	int i_pton=inet_pton(AF_INET, ip, &SockAddr.sin_addr);	
	if (i_pton < 0){
		printf("Can't set sin_addr.s_addr");
		return 1;
	 }
	else if(i_pton == 0) {
	  	printf("%s is not a valid IP address\n", ip);
		return 1;
	}

	
	if ( (connect(sockfd, (struct sockaddr *) &SockAddr, sizeof(SockAddr)))<0){
	  	std::cout<< strerror(errno)<<std::endl;
		return 1; 	 
	}
	
	process_http(sockfd, ip, page, poststr);
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	return 0;
	

}

	
