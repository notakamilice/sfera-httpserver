#include "include.h"

int parse_request(string buffer, size_t size, struct http_request *request);


int parse_request(string buffer, size_t size, http_request *request) {
	
  	printf("--------------------parsing----------------------\n");
  	printf("get header with len = %zu for parsing\n", size);
  
	
	size_t i = 0;
	
	
	if (size < 14) {
		printf ("bad string to parse");
	  	return 1;  
	}
	
	for(i = 0; i < size; i++) {
		if(buffer[i] != ' ' && buffer[i] != '\t')
		break;
	}
	
	if(i == size) {
		printf ("empty string to parse");
	  	return 1;  
	}
	
	if (buffer[i] == 'G' && buffer[i+1] == 'E' && buffer[i+2] == 'T' ){
		request->method = REQUEST_GET;
		i+=3;
	}
	else if(buffer[i] == 'P' && buffer[i+1] == 'O' && buffer[i+2] == 'S' && buffer[i+3] == 'T') {
		request->method = REQUEST_POST;
		i+=4;
	}
	else if(buffer[i] == 'H' && buffer[i+1] == 'E' && buffer[i+2] == 'A' && buffer[i+3] == 'D'){
		request->method = REQUEST_HEAD;
		i+=4;
	}
  	
  	for(;i < size; i++){
		if (buffer[i] != ' ' && buffer[i] != '\t')
		break;
	}

	if( i == size ) return -1;
  	
  	size_t k = 0;
	string f_name;
	
	while(buffer[i + k]!=' ') {
		f_name[k] =  buffer[i + k];
		k++;
	}
  	
  	i+= k;
	if(k == 1 && f_name[0] == '/') {
		printf("default file will be set: %s\n", file_name.c_str());
		f_name = file_name;
	};
	
	 	
	
	unsigned content_type;
  	if ( (content_type = f_name.find_last_of ('.')) == string::npos) { 
		fprintf (stderr, "dot in file_path not found - can't set content_type\n");
    	return 1;
    }
    
    if (f_name.substr(content_type+1).compare("html") == 0 || f_name.substr(content_type+1).compare("htm") == 0) 
		request->content_type= HTTP_HTML; 
	else if (f_name.substr(content_type+1).compare("jpeg") == 0 || f_name.substr(content_type+1).compare("jpg") == 0) 
		request->content_type = HTTP_JPEG;
  	else { 
		fprintf (stderr, "error in content_type definition\n");
    	return 1;
    }
		
	request->file_path = file_directory + f_name;
	
	if (!access(request->file_path.c_str(),F_OK)) request->file_exist=true;
	else request->file_exist=false;
	
	
	printf("file_path: %s\n", request->file_path.c_str());
	printf("content-type: %s\n", request->content_type);
	//printf("content: %s\n", request->content);
	//printf("content-length: %s\n", request->content_length);
	printf("method: %s\n", request->method);
	printf("file exists?: %b\n", request->file_exist);
	printf("------------------end parsing--------------------\n");
	
	return 0;
}




/*
int parse_request(string buffer, size_t size, http_request *request) {
	
  	printf("-----------------parsing---------------\n");
  	printf("get header with len = %zu for parsing\n", size);
  
	
  	char *space1, *space2, *enter1, *bigenter;
	
  	space1 = strchr (buffer, ' ');
  	if ( !space1 || (space1 - buffer) > 4 ) {
    	fprintf (stderr, "error in first line request1\n");
		
    	return 1;	
  	}

  	if ( !(space2 = strchr (space1 + 1, ' ')) ){ 
		fprintf (stderr, "error in first line request2\n");
		
    	return 1;
  	}
  	
  	if ( !( enter1 = strstr(space2, "\r\n")) ){ 
		fprintf (stderr, "error in first line request3\n");
		
    	return 1;
  	}
  	
  	char method[6];
  	strncpy (method, buffer, space1 - buffer);
	if ( !strcmp (method, "HEAD") ) request->method = REQUEST_HEAD;
  	else if ( !strcmp (method, "GET" ) ) request->method = REQUEST_GET;
  	else if ( !strcmp (method, "POST") ) request->method = REQUEST_POST;
  	else { 
		fprintf (stderr, "unknown method\n");
    	return 1;
  	}
  	
  	printf("method: %s\n", request->method);
  	
  	int index_path_length = (space2 - space1); 
	
	//TODO use default_file instead /
	
  	int file_path_length = index_path_length + strlen(default_dir);
	request->file_path = (char *)calloc (file_path_length, sizeof (char));
  	if ( !request->file_path ) { 
		fprintf (stderr, "%s\n", errno);
		
    	return  1;
    }
    
    char *file;
	strncpy (file, space1 + 1, index_path_length - 1);
    char *file_path=strcat(default_dir, file);
  	strncpy (request->file_path, file_path, file_path_length - 1);
	
	char *content_type;
  	if ( !(content_type = strrchr (request->file_path, '.')) ) { 
		fprintf (stderr, "dot in file_path not found - can't set content_type\n");
    	
		return 1;
    }
	++content_type;
	
	if ( !strcmp (content_type, "html") || !strcmp (content_type, "htm") ) request->content_type= HTTP_HTML; 
    else if ( !strcmp (content_type, "jpeg") || !strcmp (content_type, "jpg")) request->content_type = HTTP_JPEG;
  	else { 
		fprintf (stderr, "error in content_type definition\n");
    	
		return 1;
    }
    
    if ( !(bigenter  = strstr (space2, "\r\n\r\n")) ) { 
    	fprintf (stderr, "\r\n\r\n not found\n");
    	
		return 1;
    }
  	char *content_from=bigenter+4;
  	
	request->content_length=(buffer+size) - content_from + 1;
  	if (request->content_length && !(request->content = (char *) calloc (request->content_length, sizeof (char))) ) { 
		fprintf (stderr, "%d\n", errno);
    	
		return 1;	
	}
  	
  	strncpy (request->content, content_from, request->content_length - 1);
	
	
	
	
	if (!access(request->file_path,F_OK)) request->file_exist=true;
	else request->file_exist=false;
	
	
	printf("file_path: %s\n", request->file_path);
	printf("content: %s\n", request->content);
	printf("content-type: %s\n", request->content_type);
	printf("content-length: %s\n", request->content_length);
	//printf("method: %s\n", request->method);
	printf("file exists: %s\n", request->file_exist);
	printf("-------------end parsing---------------\n");
	
	return 0;
}

*/











