#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <set>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <fcntl.h>
#include <poll.h>
#include <ev.h>
//#include <event.h>
//#include <event2/event.h>
//#include <event2/buffer.h>

using namespace std;

const size_t MAX_MESSAGE_LENGTH = 2048*2;
const size_t MAX_HEADER_LENGTH = 2048;
string HEADER_END = "\r\n\r\n";
//const int MAX_EVENTS = 10;

//use in functions 
//char file_directory[20];  //TODO string? then change in main
//char file_name[20]; 
string file_directory;
string file_name;

string RESPONSE_200 = "HTTP/1.1 200 OK\r\n\r\n";                                   
string RESPONSE_404 = "HTTP/1.1 404 Not Found\r\n\r\n";

enum http_request_method { REQUEST_HEAD, REQUEST_GET, REQUEST_POST, RESPONSE } ;
enum  file_type { HTTP_HTML, HTTP_JPEG };


struct  http_request {
	http_request_method  method; //+ 
 	string file_path; 		//+	
 	string content;			 //if read content in read_call_back
	size_t	content_length; //if read content in read_call_back
 	file_type content_type; //+
	bool file_exist;        //+
	size_t file_size;
};


struct my_io {
  	struct ev_io *watcher; 
  	string read_buffer;
	string write_buffer;
	size_t read_buffer_size;
	size_t write_buffer_size;
	bool header_exist;
};


//глобальная структура, в которой хранятся все подключенный клиенты

//************************some additional functions*****************************
int set_nonblock(int fd) {
	int flags;
#if defined(O_NONBLOCK)
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}  

void close_fd(int fd) {
	printf ("socket %d closed\n", fd);
  	shutdown (fd, SHUT_RDWR);
	close(fd);
	return;
}

void free_my_io(struct my_io *w) {
  	close_fd(w->watcher->fd); 
	if (w->watcher) {
	  	free(w->watcher);
	}
	printf ("memory was freed for my_io\n");
	return;
}

/*
??????? something like that
struct my_io *clients[MAX_EVENTS];
void close_all(struct ev_loop *loop) {
	int i;
	for (i=0; i < MAX_EVENTS; i++) {
		if (clients[i].flags & FD_ACTIVE) 
			close_io(EV_A_ clients[i].watcher.fd);
	
		if(clients[i].read_buffer) 
			free(clients[i].read_buffer);
		if(clients[i].write_buffer) 
			free(clients[i].write_buffer);	
 	}
}
*/




