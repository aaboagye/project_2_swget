// Name & E-mail: Celena Tan 			tan.celena@gmail.com
// Name & E-mail: Aseda Aboagye			aseda.aboagye@gmail.com
// File Name: main.c
// Course: COMP 177 Computer Networking
// Project: swget
// Created on: October 24, 2012
// Last Edited: October 25, 2012

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <winsock.h> //<-- why doesn't thing work ='(

const char *argp_program_version = "swget 1.0";
const char *argp_program_bug_address = "<aseda.aboagye@gmail.com>";
static char doc[] = "swget -- A simple web download utility";
static char args_doc[] = "";		// No default args

static struct argp_option options[] = {
	{"URL",        'u', 	"URL",      	0,  	"URL of object to download", 							0 },
	{"destDir",    'd', 	"DESTDIR",  	0,  	"Destination directory to save files to", 				0 },
	{"verbose",    'v', 	0,	          	0,  	"Provide verbose output, including server headers", 	0 },
	{ 0, 			0, 		0, 				0, 		0, 														0 }
	// Last entry should be all zeros in all fields
};

struct arguments {
	int verbose;
	char *url;
	char *destdir;
};

struct host_info {
	char *host;
	int port;
	char *path;
	int is_ftp;
	char *user;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state){
	// Get the input argument from argp_parse, which we know is a pointer to our arguments structure.
	struct arguments *arguments = state->input;

	switch (key){ // Figure out which option we are parsing, and decide how to store it
		case 'v':
			arguments -> verbose = 1;
			break;
		case 'u':
			arguments -> url = arg;
			break;
		case 'd':
			arguments -> destdir = arg;
			break;
		case ARGP_KEY_END:
			// Check if our url & destdir REQUIRED "options" have been set to non-default values
			if (strcmp(arguments -> url, "") == 0 || strcmp(arguments -> destdir, "") == 0) {
				argp_usage (state);
			}

			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

static void parse_url(char *src_url, struct host_info *h) {
	char *url, *p, *sp;

	url = xstrdup(src_url);

	if(strncmp(url, "http://", 7) == 0) { // They match
		h -> port = bb_lookup_port("http", "tcp", 80);
		h -> host = url + 7;
	}

	else {
		perror("ERROR: Not an HTTP URL");
	}

	// wget 'http://busybox.net?login=john@doe':
	//   request: 'GET /?login=john@doe HTTP/1.0'
	//   saves: 'index.html?login=john@doe' (we save '?login=john@doe')
	// wget 'http://busybox.net#test/test':
	//   request: 'GET / HTTP/1.0'
	//   saves: 'index.html' (we save 'test')

	sp = strchr(h->host, '/');
	p = strchr(h->host, '?'); if (!sp || (p && sp > p)) sp = p;
	p = strchr(h->host, '#'); if (!sp || (p && sp > p)) sp = p;

	if (!sp) {
		static char nullstr[] = ""; // Must be writable because of bb_get_last_path_component()
		h->path = nullstr;
	}

	else if (*sp == '/') {
		*sp = '\0';
		h->path = sp + 1;
	}

	else { // '#' or '?'
		memmove(h->host-1, h->host, sp - h->host);
		h->host--;
		sp[-1] = '\0';
		h->path = sp;
	}

	sp = strrchr(h->host, '@');
	h->user = NULL;

	if (sp != NULL) {
		h->user = h->host;
		*sp = '\0';
		h->host = sp + 1;
	}

	sp = h->host;
}

int main(int argc, char const *argv[]) {
	struct arguments arguments;
	struct host_info host_info;
	arguments.verbose = 0;
	arguments.url = "";
	arguments.destdir = "";
	host_info.host = "";
	host_info.path = "";
	host_info.port = 80; // Port 80 is for HTTP
	int sockfd = 0; // Used for socket
	WSADATA ws;
	WSAStartup(0x0101,&ws);
	int tcp_socket;
	struct sockaddr_in peer;
	int peerlen;

	peer.sin_family = AF_INET;
		peer.sin_port = htons(80);

    // HERE WE GO..
	// Client opens TCP connection to server on port 80
    // Client sends a request to the server:
    		// GET /filename HTTP/1.1
    		// Host: www.server.com

	//The above line would work, except it would only accept connections from
	//127.0.0.1. Instead, let's use INADDR_ANY, which states that the program
	//peer.sin_addr.s_addr = htonl(INADDR_ANY);

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	bind(tcp_socket,(struct sockaddr *)&peer,sizeof(peer));

	printf("GET ");
	scanf("%255s", host_info.path);
	printf("Host: ");
	scanf("%255s", host_info.host);

	// Server sends response
	// Server closes TCP connection (Can client?)

	listen(tcp_socket, 3);
	peerlen = sizeof(peer);
	accept(tcp_socket,(struct sockaddr *)&peer, &peerlen);

	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	parse_url(&arguments.url, &host_info);

	close(tcp_socket); // Connection: close
	return 0;
}