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
