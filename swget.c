/* Name & E-mail: Celena Tan            tan.celena@gmail.com
 * Name & E-mail: Aseda Aboagye         aseda.aboagye@gmail.com
 * File Name: main.c
 * Course: COMP 177 Computer Networking
 * Project: swget
 * Created on: October 24, 2012
 * Last Edited: October 26, 2012 */

#include <sys/types.h>
#include <stdio.h>    		// Provides for printf, etc...
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
#include "swget.h"

// Set up the argument parser
const char *argp_program_version = "swget 1.0";
const char *argp_program_bug_address = "<aseda.aboagye@gmail.com> or <tan.celena@gmail.com>";
static char doc[] = "swget -- A Simple Web Download Utility";
static char args_doc[] = "";  // No standard arguments i.e. arguments without "names"

// Options.  Field 1 in ARGP.
// Order of fields: {NAME, KEY, ARG, FLAGS, DOC, GROUP}.
static struct argp_option options[] = {
  {"url",		'u', 	"URL",      0,  "URL of object to download", 							0 },
  {"destdir",	'd', 	"DESTDIR",  0,  "Directory to save downloaded object to", 				0 },
  {"verbose",	'v', 	0,          0,  "Provide verbose output, including server headers",	 	0 },
  { 0, 			0, 		0, 			0, 	0, 														0 } // Last entry should be all zeros in all fields
};

//Our argp parser.
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main (int argc, char **argv) {
	int status = 0, tcp_socket;

    struct addrinfo peer;
    struct addrinfo *peerinfo;
	struct arguments arguments;

	//Parse our arguments; every option seen by parse_opt will be reflected in arguments
	struct host_info host_info = { .host = "", .path = "", .port = 80 }; // Default values

	arguments.verbose = 0;
	arguments.url = ""; // Empty string - only contains null character
	arguments.destdir = "";

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	parse_url(arguments.url, &host_info);

    peer.ai_family = AF_UNSPEC;     //IPv4 or IPv6
    peer.ai_socktype = SOCK_STREAM; //TCP stream sockets
    peer.ai_flags = AI_CANONNAME;   //Fill in my IP for me
    peer.ai_protocol = 0;

    /* TODO:
     * 0. Open and close socket													<----DONE!
     * 1. Name resolution (ie: going from www.google.com to its IP address)		<----Work on it 10/31
     *      I think we can use the function getpeername() or something.
     *      Check Beej's guide.
     * 2. URL parsing: This part seems to be a bit challenging and is very    	<----DONE!
     *      important to this project. --> string tokenizer??
     * 3. Create HTTP GET request header
     * 4. Parse the HTTP return header from the server
     * 5. Handle redirects
     * 6. Clean Code / Testing / Debug
     * 7. Done! =)
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

	printf ("User Arguments:\nURL = %s\nDestdir = %s\nVerbose = %s\n",
	        arguments.url, // This is a pointer to the start of the URL char array
	        arguments.destdir, // This is a pointer to the start of the destir char array
	        arguments.verbose ? "yes" : "no"); // This is an integer we are testing

    // Client sends a request to the server:
            // GET /filename HTTP/1.1
            // Host: www.server.com

    /* The general way for a client to connect to a server is the following:
     * 1. socket() - make the socket
     * 2. bind() - bind the socket with the server information
     * 3. connect() - actually, connecting to the server
     *
     * From here, we can just call our send() and recv() functions.
     * close() - close socket when we're done
     */

    // Server sends response
    // Server closes TCP connection (Can client?)

    close(tcp_socket); //Connection close
    freeaddrinfo(peerinfo);
    return 0;
}

// Parser. Field 2 in ARGP.
// Order of parameters: KEY, ARG, STATE.
// Parse a single option.
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
	//Get the input argument from argp_parse, which we know is a pointer to our arguments structure
	struct arguments *arguments = state -> input;

	//Figure out which option we are parsing, and decide how to store it
	switch (key) {
    	case 'v':
    		arguments->verbose = 1;
    		break;
    	case 'u':
    		arguments->url = arg;
    		break;
    	case 'd':
    		arguments->destdir = arg;
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
void parse_url(char *url, struct host_info *h)	{
	char *it1, *it2;
	int len;

// http://
	if(strncmp(url, "http://", 7) == 0)	{
		it1 = url+7;
	}

	else {
		it1 = url+0;
	}

// www.google.com
	for(it2 = it1; *it2 != 0; it2++)
		if(*it2 == '/')
			break;

	len = it2 - it1;
	h -> host = (char *)malloc(len + 1);
	strncpy(h -> host, it1, len);
	h -> host[len] = 0;
	printf("%s\n", h -> host);

// /about/
	it1 = it2;
	for(; *it2 != 0; it2++)	{}
	len = it2 - it1;
	h -> path = (char *)malloc(len + 1);
	strncpy(h -> path, it1, len);
	h -> path[len] = 0;
	printf("%s\n", h -> path);
}

