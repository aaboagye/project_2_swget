/* Name & E-mail: Celena Tan            tan.celena@gmail.com
 * Name & E-mail: Aseda Aboagye         aseda.aboagye@gmail.com
 * File Name: swget.h
 * Course: COMP 177 Computer Networking
 * Project: swget
 * Created on: October 24, 2012
 * Last Edited: October 26, 2012 */

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
    int port;
    char *path;
    int is_ftp;
    char *user;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);

static void parse_url(char *src_url, struct host_info *h);

#endif