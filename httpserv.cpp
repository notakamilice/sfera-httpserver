//OC - Ubuntu

#include "callback.h"
//#include "include.h"

//************************************main***********************************************

int main (int argc, char **argv) {
  	
  	const uint16_t max_port = 65535;
    int port;
	string dir; //dir where server will search for data 
	string ip;
	string file;
		
	//*************************get args from commandline********************************
	// ./wwwd -d <dir> -h <ip> -p <port>
	if (argc == 1) {
    	printf("USAGE: required -d <dir> -h <ip> -p <port>\n\
		\tdir: directory with resources\n\
		\tip: the website ip\n\
		\tport: port number\n");
    	return 1;
  	}
  	
  	int rez;
	while ((rez = getopt (argc, argv, "d:h:p:")) != -1) {
		switch (rez) {
      		case 'd':
        		dir= string(optarg); //strcpy
        		break;
      		case 'h':
        		ip = string(optarg);
        		break;
      		case 'p':
        		port=atoi(optarg);
        		break;
      		case '?':
        		if (optopt == 'd' || optopt == 'h' || optopt == 'p') {
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
					printf ("\nrequired:\t./wwwd -d <dir> -h <ip> -p <port>\n");
				}
        		else 
					fprintf (stderr, "Option -%c is wrong\n", optopt);
        		
    			return 1;
			default:
        		abort();
      }
	}

	if ( optind < 7 ) {
   		printf ("\nrequired:\t./wwwd -d <dir> -h <ip> -p <port>\n");
	  	return 1;
 	}

 	if ( port <= 0 || port > max_port ){
    	printf ("error in port -> port will be changed to 8080\n");    
    	port = 8080;
  	}
  	
  	char str[20];
	if(inet_pton(AF_INET, ip.c_str(), str) == 0){
		printf ("error in ip -> ip will be changed to 127.0.0.1\n");    
	  	ip = "127.0.0.1";
	}
			
  	if (access(dir.c_str(),F_OK) == -1) {
		printf ("error in directory name, specified directory doesn't exist -> default \"dir\" will be set\n");    
	  	dir = "dir";
	}
	  	
 	file = "index.html";
		
	file_name=file;
	file_directory=dir;
	
	//******************************print args******************************************
	printf("port:			%d\n", port);
	printf("IP:			%s\n", ip.c_str());
	printf("documents dir:		%s\n", file_directory.c_str());
	
	//*****************************create listening socket******************************
	struct sockaddr_in SockAddr, CliAddr;
	int listenfd;
		
  	listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //(,,0)
	if (listenfd == -1)	{
		fprintf(stderr,"SOCKET error number %s\n", strerror(errno));
		return 1;
	}
	
	bzero(&SockAddr, sizeof(SockAddr));
	
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(port); 
	//SockAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	SockAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	
	//option so_reuseaddr on
	int yes = 1;
	if ( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ){
    	fprintf(stderr,"SETSOCKOPT error: %s\n", strerror(errno));
		close_fd(listenfd);
		return 1;
	}
		
	int Result = bind(listenfd, (struct sockaddr *)&SockAddr, sizeof(SockAddr));
	if (Result == -1) 	{
		fprintf(stderr,"BIND error %s\n", strerror(errno));
		close_fd(listenfd);
		return 1;
	} 
	
	set_nonblock(listenfd); 
	
	Result = listen(listenfd, SOMAXCONN); 
	if (Result == -1) {
		fprintf(stderr,"LISTEN error  %s\n", strerror(errno));
		close_fd(listenfd);
		return 1;
	}
	
	printf("master socket number %d\n", listenfd);
	
	
	//******************************libev*********************************************
	
	struct ev_loop *loop = EV_DEFAULT;
	assert(loop);
	
	struct ev_io event_watcher; // actual event watcher
	
	// initialise an io watcher, then start it
  	// this one will watch for stdin to become readable
	ev_io_init (&event_watcher, accept_call_back, listenfd, EV_READ);
	ev_io_start (loop, &event_watcher);
	//wait for events to arrive
	ev_run (loop);
	
	//*********************free memory, close fds and exit***************************
	
	close_fd(listenfd);
	
	// unloop was called, so exit
	return 0;
		
}

	