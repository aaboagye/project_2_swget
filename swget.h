/* Name & E-mail: Celena Tan            tan.celena@gmail.com
 * Name & E-mail: Aseda Aboagye         aseda.aboagye@gmail.com
 * File Name: swget.h
 * Course: COMP 177 Computer Networking
 * Project: swget
 * Created on: October 24, 2012
 * Last Edited: November 5, 2012 */

#ifndef SWGET_H
#define SWGET_H
#include <argp.h>

struct arguments {
    int verbose;
    char *url;
    char *destdir;
};

struct host_info {
    char *host;
    char *ip;
    int port;
    char *path;
    char *user;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);

static void parse_url(char *url, struct host_info *h);

int parse_response(char *response);

int parse_content_length(char *response);

int parse_content_type(char *response);

int parse_redirect(char *response);

#endif
