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

int parse_header(char *response);

static void handle_redirects(char *url, struct host_info *h);

#endif
