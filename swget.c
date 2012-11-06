/* Name & E-mail: Celena Tan            tan.celena@gmail.com
 * Name & E-mail: Aseda Aboagye         aseda.aboagye@gmail.com
 * File Name: main.c
 * Course: COMP 177 Computer Networking
 * Project: swget
 * Created on: October 24, 2012
 * Last Edited: November 1, 2012 */

#include <sys/types.h>
#include <stdio.h>			// Provides for printf, etc...
#include <stdlib.h>			// Provides for exit, ...
#include <argp.h>			// Provides GNU argp() argument parser
#include <getopt.h>
#include <errno.h>
#include <string.h>			// Provides for memset, ...
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "swget.h"
#define MAXDATASIZE 1024
#define BLOCK_SIZE 512
#define MAXDATASIZE_buffer 1024*6
#define DEBUG 1

//Set up the argument parser
const char *argp_program_version = "swget 1.0";
const char *argp_program_bug_address = "<aseda.aboagye@gmail.com> or <tan.celena@gmail.com>";
static char doc[] = "swget -- A Simple Web Download Utility";
static char args_doc[] = "";  // No standard arguments i.e. arguments without "names"


//Options.  Field 1 in ARGP. Order of fields: {NAME, KEY, ARG, FLAGS, DOC, GROUP}.
static struct argp_option options[] = {
		{"url",			'u', 	"URL",      0,  "URL of object to download", 							0 },
		{"destdir",		'd', 	"DESTDIR",  0,  "Directory to save downloaded object to", 				0 },
		{"verbose",		'v', 	0,          0,  "Provide verbose output, including server headers",	 	0 },
		{ 0, 			0, 		0, 			0, 	0, 														0 } // Last entry should be all zeros in all fields
};

//Our argp parser.
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main (int argc, char **argv) {
	int status = 0, tcp_socket;
	char buffer[MAXDATASIZE_buffer];
	char request[MAXDATASIZE];
	FILE *target_file = NULL;
	int bytes_read = BLOCK_SIZE + 1;
	int bytes_left;							//How many bytes left in the buffer to send.
	int bytes_sent;
	int total = 0;
	char response[MAXDATASIZE_buffer];

    struct addrinfo peer;
    struct addrinfo *peerinfo;
	struct arguments arguments = { .verbose = 0, .url = "", .destdir = "" };

	time_t t;
	time(&t);

	//Parse our arguments; every option seen by parse_opt will be reflected in arguments
	struct host_info host_info = { .host = "", .path = "", .port = 80 }; // Default values

	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	parse_url(arguments.url, &host_info);
	char filename[MAXDATASIZE];
	strcpy(filename, arguments.destdir);
	strcat(filename, host_info.path);
	printf("%s\n", filename);
/*	target_file = fopen(arguments.destdir, "w"); */

	/* Declared send_data; now initialize it here!
	 */

	strcpy(request, "GET "); // TODO: Rest of the HTTP request
	if(host_info.path[0] != '/')
		strcat(request, "/");		//In order to not put an extra '/'
	// add either '/' for www.foobar.com/ or the actual path.
	strcat(request, host_info.path);
	strcat(request,"\r\n");
	strcat(request, "Host: ");
	strcat(request, host_info.host);
	strcat(request, "\r\n");
	strcat(request, "Connection: close\r\n"); //To make things easier for now.
	strcat(request, "\r\n"); //HTTP Header must end with a single \r\n on it's own.
	// I believe request is done now.
	#if DEBUG
	printf("%s\n", request);
	#endif

    peer.ai_family = AF_UNSPEC;     //IPv4 or IPv6
    peer.ai_socktype = SOCK_STREAM; //TCP stream sockets
    peer.ai_flags = AI_CANONNAME;   //Fill in my IP for me
    peer.ai_protocol = 0;

    /* TODO:
     * 0. Open and close socket													<----DONE!
     * 1. Name resolution (ie: going from www.google.com to its IP address)		<----DONE! through getaddrinfo
     *      Use the function getaddrinfo() -- Check Beej's guide.
     * 2. URL parsing: This part seems to be a bit challenging and is very    	<----DONE!
     *      important to this project. --> string tokenizer??
     * 3. Create HTTP GET request header										<----Work on it 11/1 DONE!
     * 4. Parse the HTTP return header from the server							<----DONE!
     * 5. Send & Recv															<----1/2 DONE!
     * 6. Handle redirects
     * 7. Need to create a file descriptor to actually download the file to disk.
     * 8. Create verbose and non-verbose functions?
     * 9. Cleaning / Testing / Debugging
     * 10.Done! Due 11/06/12 =)
     */

    //Client opens TCP connection to server on port 80
    if ((status = getaddrinfo(host_info.host, "80", &peer, &peerinfo)) != 0) {
        fprintf(stderr, "Getaddrinfo ERROR: %s\n", gai_strerror(status)); //GAI
        exit(EXIT_FAILURE);
    }

    //Creating socket
    if((tcp_socket = socket(peerinfo -> ai_family, peerinfo -> ai_socktype, peerinfo -> ai_protocol)) < 0){
        perror("Socket()");
        exit(EXIT_FAILURE);
    }

    //Connecting to the server
    if(connect(tcp_socket, peerinfo -> ai_addr, peerinfo -> ai_addrlen)) {
        perror("Connect()");
        exit(EXIT_FAILURE);
    }


    // Client sends a request to the server:
            // GET /filename HTTP/1.1
            // Host: www.server.com
	bytes_left = sizeof(request);
	while (total < bytes_left) { //Should break when we've sent out everything
		bytes_sent = send(tcp_socket, request, bytes_left, 0);
		if(bytes_sent == -1)
			break;
		total += bytes_sent;
		bytes_left -= bytes_sent;
	}

	bytes_read = MAXDATASIZE + 1;	//To make sure we do it at least once.
	while (bytes_read >= MAXDATASIZE) { //Should break if the buffer is not full.
		bytes_read = recv(tcp_socket, buffer, sizeof(buffer), 0);
		/*fwrite(buffer, 1, bytes_read, target_file);*/
	} /* For fwrite, I'm not sure if it resets the file pointer to the beginning
	   * of the file on each write. I guess we'll find out when we try it. */

	int fd; // File descriptor


	strcpy(response, buffer);

	//Check what response is
	parse_response(response);

	if(parse_response(response) == 301 || 302) {
		handle_redirects(arguments.url, &host_info);
	}

	else if (parse_response(response) == 200) {
			//VERBOSE OUTPUT!!!
			if(arguments.verbose == 1) {
					printf("Downloading: %s\n", arguments.url);
			  		printf("Resolving %s... %s\n", host_info.host, host_info.ip);
			  		printf("Connecting to %s|%s|:80... connected\n", host_info.host, host_info.ip);
			  		printf("HTTP request sent, awaiting response...\n");
			  		printf("Received HTTP response header: \n");
			  		printf("HTTP/1.1 200 OK\n");
			  		printf("%s", response);
			 		printf("Connection: close\n");
			 		//printf("Length: %i (%i.%s) [%s]\n", );
			 		printf("Saving to: %s\n", arguments.destdir);
			 		printf("Finished\n");
			 }

			//NON-VERBOSE OUTPUT
			if(arguments.verbose == 0) {
			 		printf("Downloading: %s", arguments.url);
			 		//printf("Length: %i (%i.%s) [%s]", );
			 		printf("Saving to: %s", arguments.destdir);
			 		printf("Finished");
			}
	}

	else if(parse_response(response) == 400 || 404) {
		//Output verbose or non verbose
		//Bad request or no longer available
	}

    close(tcp_socket); //Connection close
    freeaddrinfo(peerinfo);
    fclose(target_file);
    return 0;
}

// Parser. Field 2 in ARGP. Parse a single option.
// Order of parameters: KEY, ARG, STATE.
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
	//Get the input argument from argp_parse, which we know is a pointer to our arguments structure
	struct arguments *arguments = state -> input;

	//Figure out which option we are parsing, and decide how to store it
	switch (key) {
    	case 'v':
    		arguments -> verbose = 1;
    		break;
    	case 'u':
    		arguments -> url = arg;
    		break;
    	case 'd':
    		arguments -> destdir = arg;
    		break;

    	case ARGP_KEY_END: //Reached the last key.
    		//Check if our url and destdir REQUIRED "options" have been set to non-default values
    		if (strcmp(arguments -> url, "") == 0 || strcmp(arguments -> destdir, "") == 0) {
    			argp_usage (state);
    		}
    		break;

    	default:
    		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

//Loop through string until second slash then the null bit
static void parse_url(char *url, struct host_info *h)	{
	char *it1, *it2;
	int len;

// http://
	if(strncmp(url, "http://", 7) == 0)	{
		it1 = url + 7;
	}

	else {
		it1 = url + 0;
	}

// host
	for(it2 = it1; *it2 != 0; it2++)
		if(*it2 == '/')
			break;

	len = it2 - it1;
	h -> host = (char *)malloc(len + 1);
	strncpy(h -> host, it1, len);
	h -> host[len] = 0;
	//printf("%s\n", h -> host);

// path
	it1 = it2;
	for(; *it2 != 0; it2++)	{}
	len = it2 - it1;
	h -> path = (char *)malloc(len + 1);
	strncpy(h -> path, it1, len);
	h -> path[len] = 0;
	//printf("%s\n", h -> path);
}

int parse_response(char *response) {
	char *it1;

	it1 = response + 9;

	if(strncmp(it1, "200", 3) == 0) 			return 200;	//OK
	else if(strncmp(it1, "301", 3) == 0)		return 301; //Moved Permanently
	else if(strncmp(it1, "302", 3) == 0)		return 302; //Found with a redirect
	else if(strncmp(it1, "400", 3) == 0)		return 400; //Bad Request
	else if(strncmp(it1, "404", 3) == 0)		return 404; //Not Found
	else										return -1;
}

int parse_header(char *response) {
	char *it1, *it2;
	char header_len[255]; 	//Store content length and content type in this string
							//Just add content length then append to it and add contend-type

	it1 = response + 12;

	for(it2 = it1; it2 != 0; it2++)
		if(*it2 == 'C')
			if(*it2 + 3 == 't')
				if(*it2 + 1 == 'e')

		break;

	//Notes:
	//Iterate through until you find capital C
	//Then it + 3 to test if char is 't' for Content-Length
	//Then it + 1 to test if char is 'e' for Content-Length

	return -1;
}

void handle_redirects(char *url, struct host_info *h) {
	//Obtain the new URL
	//Parse the URL
	//Call send
	//Call recv
}
