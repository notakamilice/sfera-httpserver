//#include "include.h"
#include "parse.h"

void accept_call_back (struct ev_loop *loop, struct ev_io *watcher, int revents);
void read_call_back (struct ev_loop *loop, struct ev_io *watcher, int revents); 
void write_call_back (struct ev_loop *loop, struct ev_io *watcher, int revents); 

//TODO:free memory 

//************************accepting connection - read from server's socket****************************
 //watcher ссылка на структуру события, вызвавшего cb функцию 
void accept_call_back (struct ev_loop *loop, struct ev_io *watcher, int revents) {
  	
  	if(EV_ERROR & revents){
	  	fprintf(stderr,"got invalid event\n");
		return;
	}
		
	struct sockaddr_in CliAddr;
	socklen_t CliAddr_len = sizeof(CliAddr);
	
  	int watch_fd = watcher->fd;
  	int  connfd = accept (watch_fd,(struct sockaddr *)&CliAddr, &CliAddr_len);
  	if ( connfd < 0 ) {
		fprintf (stderr, "client accept error %s\n", strerror(errno));
		return ;
		
  	}
  	
   	set_nonblock (connfd);
	
	
	struct my_io *client = (struct my_io*) malloc (sizeof(struct my_io)); //TODO MALLOC!!!
	
	if ( !client ) { 
	  	fprintf (stderr, "allocate error for client %s\n", strerror(errno));
    	close_fd (connfd);
		return ;
  	}
  	
	client->read_buffer_size = 0;
	client->write_buffer_size = 0;
	client->header_exist = false;	
	
	
	printf ("connection established\n");
	printf ("client socket number %d\n", connfd);
	
	
	ev_io_init((struct ev_io*)client, read_call_back, connfd, EV_READ);
	ev_io_start(loop, (struct ev_io*)client);
	
	//free_my_io(client);
	//printf ("memory is freed\n");
	
	return;
}


void read_call_back (struct ev_loop *loop, struct ev_io *watcher, int revents) { //here watcher - connfd TODO delete comments
	
	//ev_io_stop(loop, watcher);// TODO ???
	
  	if(EV_ERROR & revents) {
	  	fprintf(stderr,"got invalid event\n");
		return;
	}
		
	struct my_io *r_watcher = (struct my_io *)watcher; 
	r_watcher->watcher=watcher;	
	
	printf("read_buffer_size = %zu\n", r_watcher->read_buffer_size);
	/*
	if (r_watcher->header_exist) {
		printf("header exist - no need to read more\n");
		return;	  
	}*/
	
  	printf ("read from client %d\n", watcher->fd);
  	
	//read request
	static char buffer[MAX_MESSAGE_LENGTH];
	ssize_t recieved = recv(watcher->fd, buffer, MAX_MESSAGE_LENGTH, MSG_NOSIGNAL); 
	printf("received = %zu,  message length = %zu\n", recieved, strlen(buffer));

	if(recieved <= 0)	{ 
		ev_io_stop(loop, watcher);
	
		printf("stop receiving data\n");
	};
	
	buffer[recieved] = '\0';
	printf("request message:\n%s\n", buffer);
	
	/*
	char *has_header_end;
	has_header_end = strstr(buffer, HEADER_END.c_str()); //pointer to header_end or NULL if it is not recieved yet

	size_t current_header_size = r_watcher->read_buffer_size;
	
	if (has_header_end == NULL)
		current_header_size += recieved;
		else
		current_header_size += has_header_end  + HEADER_END.length() - buffer;

	if(current_header_size > MAX_HEADER_LENGTH){
		printf("header is too big\n");
		printf("header size - %zu\n", current_header_size);
		r_watcher -> read_buffer_size = 0;
		return;
	}
	
	//fill struct r_watcher
	//memcpy((char *)r_watcher -> read_buffer + r_watcher->read_buffer_size, buffer, current_header_size - r_watcher -> read_buffer_size);
	(r_watcher->read_buffer).append(buffer, current_header_size - r_watcher -> read_buffer_size);
	r_watcher -> read_buffer_size = current_header_size;
	
	//check
	//if (has_header_end == NULL)
		//has_header_end = (r_watcher->read_buffer).find(HEADER_END);
	
	
	if (has_header_end != NULL) {
	  
		ev_io_stop (loop, watcher);
		printf("request header recieved \n");
		
		//TODO server should read data after header 
				
		printf("watcher read_buffer:\n%s\n", r_watcher->read_buffer.c_str());
		r_watcher->header_exist = true;
		r_watcher->write_buffer_size = 0;
		
		ev_io_init((struct ev_io *)r_watcher, write_call_back, watcher->fd, EV_WRITE);
		ev_io_start(loop, (struct ev_io*)r_watcher); 
		printf("write_call_back finished\n");

	}
	*/
	
	//simple read
	//fill struct r_watcher
	//memcpy((char *)r_watcher -> read_buffer + r_watcher->read_buffer_size, buffer, current_header_size - r_watcher -> read_buffer_size);
	(r_watcher->read_buffer).append(buffer);
	r_watcher -> read_buffer_size = strlen(buffer);
	
	
	printf("request header recieved \n");
		
	printf("watcher read_buffer:\n%s\n", r_watcher->read_buffer.c_str());
	r_watcher->header_exist = true;
	r_watcher->write_buffer_size = 0;
	
	ev_io_init((struct ev_io *)r_watcher, write_call_back, watcher->fd, EV_WRITE);
	ev_io_start(loop, (struct ev_io*)r_watcher); 
	printf("write_call_back finished\n");
	
	return;
}


void write_call_back (struct ev_loop *loop, struct ev_io *watcher, int revents){
	
  	ev_io_stop(loop,watcher); //TODO ???	
  
  	struct my_io *w_watcher = (struct my_io *)watcher;
	
	struct http_request *request = (struct http_request *)malloc(sizeof(struct http_request));
	int res = parse_request(w_watcher->read_buffer, w_watcher->read_buffer_size, request);
	if (res == 1) {
	  	//TODO
	  	ev_io_stop(loop,watcher);
	  	return;
	}
	
	
	string response = "";
	
	if (!request->file_exist) 
	  	response = RESPONSE_404;
	
	else  if (request->file_exist && request->method == REQUEST_HEAD)
	  	response = RESPONSE_200;
	
	else {
		
		if (request->content_type == HTTP_HTML )	{
			response = RESPONSE_200 + string("\r\nContent-Type: text/html\r\n");
		}
		else if(request->content_type == HTTP_JPEG ){
			response = RESPONSE_200 + string("\r\nContent-Type: image/jpeg\r\n");
			
		}
		
			
	}
	
	send(watcher->fd,response.c_str(),response.length(),MSG_NOSIGNAL);
	
	FILE * f;
	size_t lSize;
	if (request->method == REQUEST_GET || request->method == REQUEST_POST) {
	  	if (request->content_type == HTTP_HTML) {
			
  			f = fopen (request->file_path.c_str(),"r");	
			lSize = ftell (f);
  			rewind (f);
			
		}
		if (request->content_type == HTTP_JPEG) {
			
  			f = fopen (request->file_path.c_str(),"rb");	
			lSize = ftell (f);
  			rewind (f);
		}
		request->file_size = lSize;
		
		string response2 = string("Content-Length:") + to_string(lSize) + string("\r\n\r\n");
			
		send(watcher->fd,response2.c_str(),response2.length(),MSG_NOSIGNAL);
		
		void *msg_body = malloc (request->file_size);
		int r =	fread(msg_body,1, request -> file_size, f );
		
		w_watcher->write_buffer = (const char *)msg_body;
		w_watcher->write_buffer_size = request->file_size;
		
		send(watcher -> fd,(char*) msg_body, request -> file_size, MSG_NOSIGNAL); 
		printf("have sent %zu",lSize);
		
		free(msg_body);	
	}
	fclose(f);
	
	
	free(request);
	
	free_my_io(w_watcher);
	

	return;
  
}














