/* Name & E-mail: Celena Tan            tan.celena@gmail.com
 * Name & E-mail: Aseda Aboagye         aseda.aboagye@gmail.com
 * File Name: main.c
 * Course: COMP 177 Computer Networking
 * Project: swget
 * Created on: October 24, 2012
 * Last Edited: October 26, 2012 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "swget.h"

#define DEBUG 1

const char *argp_program_version = "swget 1.0";
const char *argp_prog_bug_email =
"<aseda.aboagye@gmail.com> or <tan.celena@gmail.com>";
static char doc[] = "swget -- A simple web download utility";
static char args_doc[] = "";        // No default args
static struct argp_option options[] = {
    {"URL",        'u',     "URL",          0,      "URL of object to download. [REQUIRED]",              0 },
    {"destDir",    'd',     "DESTDIR",      0,      "Destination directory to save files to. [REQUIRED]", 0 },
    {"verbose",    'v',     0,              0,      "Provide verbose output, including server headers",   0 },
    { 0,            0,      0,              0,      0,                                                    0 }
    // Last entry should be all zeros in all fields
};
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char **argv) {
    struct arguments arguments = { .verbose = 0, .url = "", .destdir = "" };
    struct host_info host_info = { .host = "", .path = "", .port = 80 };
    argp_parse (&argp, argc, argv, 0, 0, &arguments);
    /* after calling argp_parse, all the info is in the arguments struct. */
    struct addrinfo peer;
    struct addrinfo *peerinfo;
    int status = 0, tcp_socket;
    parse_url(arguments.url, &host_info); /* This SEGFAULTs, but we can use as guide. */

    peer.ai_family = AF_UNSPEC;     //IPv4 or IPv6
    peer.ai_socktype = SOCK_STREAM; //TCP stream sockets
    peer.ai_flags = AI_CANONNAME;   //Fill in my IP for me

    if ((status = getaddrinfo(host_info.host, "80", &peer, &peerinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status)); //GAI
        exit(EXIT_FAILURE);
    }
    //Create socket. Print out failed if socket cannot be created
    if((tcp_socket = socket(peerinfo -> ai_family, peerinfo -> ai_socktype, peerinfo -> ai_protocol)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if(bind(tcp_socket, peerinfo -> ai_addr, peerinfo -> ai_addrlen)){
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if(connect(tcp_socket, peerinfo -> ai_addr, peerinfo -> ai_addrlen)){
        perror("Connect()");
        exit(EXIT_FAILURE);
    }
    printf("GET ");
    scanf("%255s", host_info.path);
    printf("Host: ");
    scanf("%255s", host_info.host);

#if DEBUG
    printf("host: %s\n", arguments.url);
#endif

    // HERE WE GO..
    // Client opens TCP connection to server on port 80
    // Client sends a request to the server:
            // GET /filename HTTP/1.1
            // Host: www.server.com

    //The above line would work, except it would only accept connections from
    //127.0.0.1. Instead, let's use INADDR_ANY, which states that the program
    //peer.sin_addr.s_addr = htonl(INADDR_ANY);

    /* The general way for a client to connect to a server is the following:
     *
     * socket() - make the socket
     * bind() - bind the socket with the server information
     * connect() - actually, connecting to the server
     *
     * From here, we can just call our send() and recv() functions.
     * close() - close socket when we're done
     */

    // Server sends response
    // Server closes TCP connection (Can client?)

    close(tcp_socket); // Connection: close
    return 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state){
    // Get the input argument from argp_parse, which we know is a pointer to our arguments structure.
    struct arguments *arguments = state -> input;
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

static void parse_url(char *src_url, struct host_info *h) {
    char *url = NULL, *p, *sp;

    /*url = xstrdup(src_url); Why do we do this instead of a strcpy?
    we cannot use this because it is not a standard library. */
    strcpy(url, src_url);

    if(strncmp(url, "http://", 7) == 0) { // They match
        /*h -> port = bb_lookup_port("http", "tcp", 80); what's this?
        HTTP works off of port 80, and we've already set this value earlier.
        Furthermore, it requires a non standard library. */
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
