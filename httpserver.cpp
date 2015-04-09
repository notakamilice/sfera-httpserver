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
#define MAXHEADERLINE  200



//*************************GET  processing********************************************
int method_get (int sockfd, char* filename, char *doc_dir, char *index)   {
	FILE *fd;
	char buf[MAXHEADERLINE+1]; //response(headers) that will be written to client sockfd
	char filepath[MAXHEADERLINE+1];//here "doc_dir/filename"
    char data[MAXLINE+1]; //response (data) from filename
    //char *data;
	int n;
	
	char pattern1[] = 
			"HTTP/1.1 %d %s\r\n"
			"Content-Type: text/html\r\n"
			"\r\n";
  	char pattern2[] = 
			"HTTP/1.1 %d %s\r\n"
			"Content-Type: image/jpg\r\n"
			"\r\n";
		 
    //bzero(line, sizeof(line));
	
	if (strcmp(filename, "/") == 0)
		sprintf(filepath, "%s%s", doc_dir, index);
	else 
		sprintf(filepath, "%s%s", doc_dir, filename);
  	
	
	if ( (strstr(filename,".html"))!=NULL) { //if html
		if ( (fd = fopen(filepath, "r")) == NULL) {		
			sprintf(buf, pattern1, 404, "NOT_FOUND");
			if((n=write(sockfd,buf,strlen(buf)))<0) {
				printf("write to socket error GET");
            	return 1;
        	}
			
		}
		else {
			sprintf(buf, pattern1, 200, "OK");
			if((n=write(sockfd,buf,strlen(buf)))<0) {
				printf("write to socket error GET");
            	return 1;
        	}
			while (fgets(data, sizeof(data), fd) != NULL) 
				if((n=write(sockfd,data,strlen(data)))<0) {
					printf("write to socket error GET");
            		return 1;
        		}
        	/*
			fseek(fd, 0, SEEK_END);
  			size_t file_size = ftell(fd);
  			rewind(fd);
  			data = (char*) malloc (sizeof(char)*file_size);
  			result = fread (data,1,file_size,fd);
  			if((n=write(sockfd,data,strlen(data)))<0) {
				printf("write to socket error GET");
            	return 1;
        	}
			*/	
		}
	}
	else { //if image(jpg)
	  /*
	   if((bytecount = send(*csock, buffer, buffer_len, 0))==SOCKET_ERROR){
fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
открывал в бинарнике, передавал буфер в sprintf 
забыл что он по окончании строки работает '\0'
//http://stackoverflow.com/questions/10531884/write-a-jpg-image-file-in-c
*/
	}
	
	//free(data);
	//free(filepath);
	//free(buf);
	fclose(fd);
	  
	//return 1;
}
//*************************HEAD  processing******************************************** 
int method_head (int sockfd, char* filename, char *doc_dir, char *index)   {
  	FILE *fd;
	int n;
	char buf[MAXHEADERLINE+1]; //response(headers) that will be written to client sockfd
	char filepath[MAXHEADERLINE+1]; //here "doc_dir/filename"
	char pattern[] = 
		"HTTP/1.1 %d %s\r\n"
		"\r\n";
  	if (strcmp(filename, "/") == 0)
		sprintf(filepath, "%s%s", doc_dir, index);
	else 
		sprintf(filepath, "%s%s", doc_dir, filename);
	
	if ( (fd = fopen(filepath, "r")) == NULL) {		
		sprintf(buf, pattern, 404, "NOT_FOUND");
		if((n=write(sockfd,buf,strlen(buf)))<0) {
            printf("write to socket error HEAD");
            return 1;
        }
		
	}
	else {
		sprintf(buf, pattern, 200, "OK");
		if((n=write(sockfd,buf,strlen(buf)))<0) {
            printf("write to socket error HEAD");
            return 1;
        }
	}
	
	fclose(fd);
	//free(filepath);
	//free(buf);
	return 0;
}

//*************************POST  processing********************************************
int method_post (int sockfd, char* filename, char *doc_dir, char *index)   {  }
//*************************main funtion for http-messages processing*******************
int http_main (int sockfd, char *doc_dir, char *index) {
	struct	request  {
		char method[MAXHEADERLINE+1];
		char filename[MAXHEADERLINE+1];
		char protocol[MAXHEADERLINE+1];
	};
	
	/*
	Content-Type: 
	Content-Length: 
	*/
	ssize_t n;
   	char buffer[MAXLINE+1]; //client's request
	struct	request Request;
  	
	while ((n = read(sockfd, buffer, MAXLINE)) > 0) {
		buffer[n] = '\0';
		printf("%s", buffer);
	}
	if (n<0) {
	  	printf("can't read from client-socket");
	  	close(sockfd);
	  	return 1;
	}
	
		
	
	if ( sscanf (buffer, "%s %s %s", &Request.method, &Request.filename, &Request.protocol) == 3) {
		if (strcmp(Request.method, "GET") == 0)
			method_get(sockfd, Request.filename, doc_dir, index);
		else 
		if  (strcmp(Request.method, "POST") == 0)
		  	method_post(sockfd, Request.filename, doc_dir, index);
		else 
		if  (strcmp(Request.method, "HEAD") == 0)
		  	method_head(sockfd, Request.filename, doc_dir,index);
	}
	//free(buffer);
	return 0;
}  


//*************************************************************************************

int main (int argc, char **argv) {
  
    //*********************************************************************************

    char *dir; //dir where server will search for data 
    char *index; //default html-doc - index.html
    char *ip; //~host
	int port;
	
	/*
	char temp[MAXLINE+1]; 
	сhar document_root[MAXLINE+1];
	char index[MAXLINE+1]; //page - index.html
	*/
	    
    //**********************************************************************************
	
	struct sockaddr_in SockAddr, CliAddr;
	int CliAddr_size = sizeof(CliAddr);
	int listenfd, connfd;
	
	//******************get data from commandline args***************************
	//getopt
	if(argc == 1){
    	printf("USAGE: required -d <dir> -h <ip> -p <port>\n\
		\tdir: directory with resources\n\
		\tip: the website ip\n\
		\tport: port number\n");
    	return 2;
  	}
  	
  	dir=argv[2];
  	ip=argv[4]; //inet_ntop?
  	port=atoi(argv[6]);
	index="index.html"; //

	//***********************************************************************************
  	
	printf("port:			%d\n", port);
	printf("IP:			%s\n", ip);
	printf("documents dir:		%s\n", dir);
	printf("index:			%s\n", index);
	
	//***********************************************************************************
	
  	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //(,,0)
	if(listenfd == -1)	{
		//std::cout << strerror(errno) << std::endl;
		printf("SOCKET error number %d", errno);
		return 1;
	}
	
	bzero(&SockAddr, sizeof(SockAddr));
	
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(port); 
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	
	int Result = bind(listenfd, (struct sockaddr *)&SockAddr, sizeof(SockAddr));
	if(Result == -1) 	{
		//std::cout << strerror(errno) << std::endl;
		printf("BIND error number %d", errno);
		return 1;
	} 
	Result = listen(listenfd, SOMAXCONN); 

	if(Result == -1){
		//std::cout << strerror(errno) << std::endl;
		printf("LISTEN error number %d", errno);
		return 1;
	}
	printf("Waiting for client...");
	//***********************************************************************************
	while (true) {
		connfd = accept(listenfd, (struct sockaddr *) &CliAddr, (socklen_t *)&CliAddr_size);
		printf("Client %s connected\n",inet_ntoa(CliAddr.sin_addr));
		http_main(connfd, dir,index); // main function, gives necessary data to client
		shutdown (connfd, SHUT_RDWR);
		close(connfd);	
	  
	}
	
	//***********************************************************************************
	shutdown (listenfd, SHUT_RDWR);
	close(listenfd);
	return 0;
	

}

	